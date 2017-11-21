#include "run_benchmark.h"
#include "atom_defines.h"
#include "hpt_core/backtesting/backtesting_mail_box.h"
#include "backtesting_cta_signal_broker_ex.h"
#include "hpt_core/backtesting/simulated_always_treade_execution_handler.h"
#include "hpt_core/portfolio_handler.h"
#include "hpt_core/backtesting/price_handler.h"
#include "rtn_order_recorder.h"
#include "type_defines.h"
#include "follow_strategy/cta_traded_strategy.h"
#include "backtesting_cta_signal_broker.h"
#include "backtesting_defines.h"

caf::behavior RunBenchmark(caf::event_based_actor* self, caf::actor coor) {
  return {[=](const std::string& market, const std::string& instrument,
              const std::string& out_dir, const StrategyParam& strategy_param,
              int cancel_order_after_minute, TickContainer tick_container,
              CTASignalContainer cta_signal_container) {
    caf::aout(self) << market << ":" << instrument << "\n";
    std::list<std::function<void(void)>> callable_queue;
    bool running = true;
    double init_cash = 50 * 10000;

    std::string csv_file_prefix = str(boost::format("%s") % instrument);
    BacktestingMailBox mail_box(&callable_queue);

    RtnOrderToCSV<BacktestingMailBox> order_to_csv(&mail_box, out_dir,
                                                   csv_file_prefix);

    // BacktestingCTASignalBrokerEx<BacktestingMailBox>
    //    backtesting_cta_signal_broker_(&mail_box, cta_signal_container,
    //                                   instrument);

    BacktestingCTASignalBroker<BacktestingMailBox>
        backtesting_cta_signal_broker_(&mail_box, cta_signal_container,
                                       instrument);
    CTATradedStrategy<BacktestingMailBox> strategy(&mail_box);
    SimulatedAlwaysExcutionHandler<BacktestingMailBox> execution_handler(
        &mail_box);

    PortfolioHandler<BacktestingMailBox> portfolio_handler_(
        init_cash, &mail_box, std::move(instrument), out_dir, csv_file_prefix,
        strategy_param.margin_rate, strategy_param.constract_multiple,
        std::move(strategy_param.cost_basis), true);

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
    // self->quit();
  }};
}
