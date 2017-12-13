#include "run_strategy.h"
#include <boost/algorithm/string.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

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
                          ProductInfoMananger* product_info_mananger,
                          bool cancel_limit_order_when_switch_trade_date) {
  return {[=](const std::string& instrument,
              const std::string& out_dir,
              const std::string& strategy_config_file,
              const StrategyParam& strategy_param,
              int cancel_order_after_minute, TickContainer tick_container,
              CTASignalContainer cta_signal_container) {
    boost::property_tree::ptree pt;
    try {
      boost::property_tree::read_json(strategy_config_file, pt);
    } catch (boost::property_tree::ptree_error& err) {
      std::cout << "Read Confirg File Error:" << err.what() << "\n";
      return;
    }

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

    RtnOrderToCSV order_to_csv(&mail_box, out_dir, csv_file_prefix);
    BacktestingCTASignalBrokerEx backtesting_cta_signal_broker_(
        &mail_box, cta_signal_container, instrument);

    std::string instrument_code =
        instrument.substr(0, instrument.find_first_of("0123456789"));
    boost::algorithm::to_lower(instrument_code);

    DelayOpenStrategyAgent<OptimalOpenPriceStrategy> strategy(
        &mail_box, &pt, product_info_mananger, &log);

    SimulatedExecutionHandler execution_handler(
        &mail_box, cancel_limit_order_when_switch_trade_date);

    PortfolioHandler portfolio_handler_(
        init_cash, &mail_box, std::move(instrument), out_dir, csv_file_prefix,
        strategy_param.margin_rate, strategy_param.constract_multiple,
        strategy_param.cost_basis, true);

    PriceHandler price_handler(
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
    caf::aout(self) << instrument << " espces:"
                    << std::chrono::duration_cast<std::chrono::milliseconds>(
                           hrc::now() - beg)
                           .count()
                    << "\n";
  }};
}
