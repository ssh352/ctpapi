#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <fstream>
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/any.hpp>
#include "caf/all.hpp"
#include "common/api_struct.h"
#include "hpt_core/backtesting/execution_handler.h"
#include "hpt_core/backtesting/price_handler.h"
#include "hpt_core/backtesting/backtesting_mail_box.h"
#include "hpt_core/cta_transaction_series_data_base.h"
#include "hpt_core/tick_series_data_base.h"
#include "hpt_core/portfolio_handler.h"
#include "strategies/strategy.h"
#include "strategies/wr_strategy.h"
#include "caf_mail_box.h"
#include "live_trade_data_feed_handler.h"
#include "strategies/key_input_strategy.h"
using CTASignalAtom = caf::atom_constant<caf::atom("cta")>;

#include "strategies/delayed_open_strategy.h"
#include "live_trade_broker_handler.h"

class AbstractExecutionHandler;

using IdleAtom = caf::atom_constant<caf::atom("idle")>;

using TickContainer = std::vector<std::pair<std::shared_ptr<Tick>, int64_t>>;
using CTASignalContainer =
    std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>;

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(TickContainer)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTASignalContainer)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CancelOrder)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(InputOrderSignal)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(InputOrder)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<TickData>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<OrderField>)

class RtnOrderToCSV {
 public:
  RtnOrderToCSV(BacktestingMailBox* mail_box, const std::string& prefix_)
      : mail_box_(mail_box), orders_csv_(prefix_ + "_orders.csv") {
    mail_box_->Subscribe(&RtnOrderToCSV::HandleOrder, this);
  }

  void HandleOrder(const std::shared_ptr<OrderField>& order) {
    orders_csv_ << order->update_timestamp << "," << order->order_id << ","
                << (order->position_effect == PositionEffect::kOpen ? "O" : "C")
                << "," << (order->direction == OrderDirection::kBuy ? "B" : "S")
                << "," << static_cast<int>(order->status) << "," << order->price
                << "," << order->qty << "\n";
  }

 private:
  BacktestingMailBox* mail_box_;
  mutable std::ofstream orders_csv_;
};

class config : public caf::actor_system_config {
 public:
  int delayed_close_minutes = 10;
  int cancel_after_minutes = 10;
  int position_effect = 0;

  config() {
    opt_group{custom_options_, "global"}
        .add(delayed_close_minutes, "delayed,d", "set delayed close minutes")
        .add(cancel_after_minutes, "cancel,c", "set cancel after minutes")
        .add(position_effect, "open_close", "backtesting open(0) close(1)")
        .add(scheduler_max_threads, "max-threads",
             "sets a fixed number of worker threads for the scheduler");
  }
};

int caf_main(caf::actor_system& system, const config& cfg) {
  using hrc = std::chrono::high_resolution_clock;
  auto beg = hrc::now();
  CAFMailBox mail_box(system);
  bool running = true;
  double init_cash = 50 * 10000;
  int delayed_input_order_by_minute = 10;
  int cancel_order_after_minute = 10;
  int backtesting_position_effect = 0;

  std::string instrument = "FG801";

  // std::string market = "dc";
  // std::string instrument = "a1709";
  std::string csv_file_prefix =
      str(boost::format("%s_%d_%d_%s") % instrument %
          delayed_input_order_by_minute % cancel_order_after_minute %
          (backtesting_position_effect == 0 ? "O" : "C"));

  // KeyInputStrategy<CAFMailBox> strategy(&mail_box, instrument);
  DelayedOpenStrategy<CAFMailBox> strategy(&mail_box, 10 * 60);

  LiveTradeBrokerHandler<CAFMailBox> live_trade_borker_handler(&mail_box);

  LiveTradeDataFeedHandler<CAFMailBox> live_trade_data_feed_handler(&mail_box);

  PortfolioHandler<CAFMailBox> portfolio_handler_(
      init_cash, &mail_box, instrument, csv_file_prefix, 0.1, 10,
      CostBasis{CommissionType::kFixed, 165, 165, 165});

  mail_box.FinishInitital();

  live_trade_data_feed_handler.SubscribeInstrument(instrument);

  live_trade_data_feed_handler.Connect("tcp://180.166.11.33:41213", "4200",
                                       "15500011", "371070");

  live_trade_borker_handler.Connect("tcp://180.168.146.187:10000", "9999",
                                    "053867", "8661188");

  int qty;
  while (std::cin >> qty) {
    mail_box.Send(qty);
  }

  system.await_all_actors_done();

  std::cout << "espces:"
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   hrc::now() - beg)
                   .count()
            << "\n";
  return 0;
}

CAF_MAIN()
