#include "run_strategy.h"
#include <boost/algorithm/string.hpp>
#include <boost/log/sources/logger.hpp>
#include "atom_defines.h"
#include "hpt_core/backtesting/backtesting_mail_box.h"
#include "backtesting_cta_signal_broker_ex.h"
#include "hpt_core/portfolio_handler.h"
#include "hpt_core/backtesting/price_handler.h"
#include "rtn_order_recorder.h"
#include "type_defines.h"
#include "follow_strategy/delayed_open_strategy_ex.h"
#include "follow_strategy/delay_open_strategy_agent.h"
#include "follow_strategy/optimal_open_price_strategy.h"
#include "hpt_core/backtesting/execution_handler.h"
#include "backtesting_defines.h"

caf::behavior RunStrategy(caf::event_based_actor* self,
                          caf::actor coor,
                          bool cancel_limit_order_when_switch_trade_date) {
  return {[=](const std::string& market, const std::string& instrument,
              const std::string& out_dir, const StrategyParam& strategy_param,
              int cancel_order_after_minute, TickContainer tick_container,
              CTASignalContainer cta_signal_container) {
    using hrc = std::chrono::high_resolution_clock;
    auto beg = hrc::now();
    boost::log::sources::logger log;
    log.add_attribute(
        "instrument",
        boost::log::attributes::constant<std::string>(instrument));

    std::list<std::function<void(void)>> callable_queue;
    bool running = true;
    double init_cash = 50 * 10000;

    std::string csv_file_prefix = str(boost::format("%s") % instrument);
    BacktestingMailBox mail_box(&callable_queue);

    RtnOrderToCSV<BacktestingMailBox> order_to_csv(&mail_box, out_dir,
                                                   csv_file_prefix);
    BacktestingCTASignalBrokerEx<BacktestingMailBox>
        backtesting_cta_signal_broker_(&mail_box, cta_signal_container,
                                       instrument);

    std::string instrument_code =
        instrument.substr(0, instrument.find_first_of("0123456789"));
    boost::algorithm::to_lower(instrument_code);

    // std::unordered_map<std::string, DelayedOpenStrategyEx::StrategyParam>
    //    strategy_params;
    // strategy_params.insert(
    //    {instrument_code, DelayedOpenStrategyEx::StrategyParam{
    //                          delay_open_after_seconds, price_offset_rate}});
    // DelayOpenStrategyAgent<BacktestingMailBox, DelayedOpenStrategyEx>
    // strategy(
    //    &mail_box, std::move(strategy_params), &log);

    std::unordered_map<std::string, OptimalOpenPriceStrategy::StrategyParam>
        strategy_params;
    strategy_params.insert(
        {instrument_code,
         OptimalOpenPriceStrategy::StrategyParam{
             strategy_param.delay_open_order_after_seconds,
             strategy_param.wait_optimal_open_price_fill_seconds,
             strategy_param.price_offset}});
    DelayOpenStrategyAgent<BacktestingMailBox, OptimalOpenPriceStrategy>
        strategy(&mail_box, std::move(strategy_params), &log);

    SimulatedExecutionHandler<BacktestingMailBox> execution_handler(
        &mail_box, cancel_limit_order_when_switch_trade_date);

    PortfolioHandler<BacktestingMailBox> portfolio_handler_(
        init_cash, &mail_box, std::move(instrument), out_dir, csv_file_prefix,
        strategy_param.margin_rate, strategy_param.constract_multiple,
        strategy_param.cost_basis, true);

    PriceHandler<BacktestingMailBox> price_handler(
        instrument, &running, &mail_box, std::move(tick_container),
        cancel_order_after_minute * 60, cancel_order_after_minute * 60);

    while (running) {
      if (!callable_queue.empty()) {
        auto callable = callable_queue.front();
        callable();
        callable_queue.pop_front();
      } else {
        price_handler.StreamNext();
      }
    }
    self->send(coor, IdleAtom::value, self);
    caf::aout(self) << market << ":" << instrument << " espces:"
                    << std::chrono::duration_cast<std::chrono::milliseconds>(
                           hrc::now() - beg)
                           .count()
                    << "\n";
  }};
}
