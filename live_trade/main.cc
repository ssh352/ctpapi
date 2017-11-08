#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <fstream>
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/any.hpp>
#include "caf/all.hpp"
#include "common/api_struct.h"
// #include "hpt_core/backtesting/execution_handler.h"
// #include "hpt_core/backtesting/price_handler.h"
// #include "hpt_core/backtesting/backtesting_mail_box.h"
#include "hpt_core/cta_transaction_series_data_base.h"
#include "hpt_core/tick_series_data_base.h"
//#include "hpt_core/portfolio_handler.h"
//#include "strategies/strategy.h"
//#include "strategies/wr_strategy.h"
#include "caf_mail_box.h"
#include "live_trade_data_feed_handler.h"
//#include "strategies/key_input_strategy.h"
#include "follow_strategy/delayed_open_strategy_ex.h"
#include "support_sub_account_broker.h"
#include "ctp_broker/ctp_order_delegate.h"
#include "ctp_broker/ctp_instrument_broker.h"
#include "caf_atom_defines.h"
#include "follow_strategy/cta_order_signal_subscriber.h"
#include "live_trade_broker_handler.h"
#include "live_trade_mail_box.h"
#include "ctp_rtn_order_subscriber.h"

//#include "live_trade_broker_handler.h"
class AbstractExecutionHandler;

class CAFDelayOpenStrategyAgent : public caf::event_based_actor {
 public:
  CAFDelayOpenStrategyAgent(caf::actor_config& cfg,
                            DelayedOpenStrategyEx::StrategyParam param,
                            LiveTradeMailBox* inner_mail_box,
                            LiveTradeMailBox* common_mail_box)
      : caf::event_based_actor(cfg),
        agent_(this, std::move(param)),
        inner_mail_box_(inner_mail_box),
        common_mail_box_(common_mail_box) {}

  virtual caf::behavior make_behavior() override { return message_handler_; }

  template <typename... Ts>
  void Send(Ts&&... args) {
    inner_mail_box_->Send(std::forward<Ts>(args)...);
  }

  template <typename CLASS, typename... Ts>
  void Subscribe(void (CLASS::*pfn)(Ts...), CLASS* ptr) {
    common_mail_box_->Subscribe(typeid(std::tuple<std::decay_t<Ts>...>), this);
    message_handler_ = message_handler_.or_else(
        [ptr, pfn](Ts... args) { (ptr->*pfn)(args...); });
  }

  template <typename CLASS>
  void Subscribe(void (CLASS::*pfn)(const std::shared_ptr<OrderField>&),
                 CLASS* ptr) {
    inner_mail_box_->Subscribe(typeid(std::tuple<std::shared_ptr<OrderField>>),
                               this);
    message_handler_ = message_handler_.or_else(
        [ptr, pfn](const std::shared_ptr<OrderField>& order) {
          (ptr->*pfn)(order);
        });
  }

 private:
  LiveTradeMailBox* inner_mail_box_;
  LiveTradeMailBox* common_mail_box_;
  caf::message_handler message_handler_;
  DelayOpenStrategyAgent<CAFDelayOpenStrategyAgent> agent_;
};

class CAFSubAccountBroker : public caf::event_based_actor,
                            public CTPOrderDelegate {
 public:
  CAFSubAccountBroker(caf::actor_config& cfg,
                      LiveTradeMailBox* inner_mail_box,
                      LiveTradeMailBox* common_mail_box,
                      std::string account_id)
      : caf::event_based_actor(cfg),
        inner_mail_box_(inner_mail_box),
        common_mail_box_(common_mail_box),
        account_id_(std::move(account_id)) {
    // inner_mail_box_->Subscrdibe(
    //    typeid(std::tuple<std::shared_ptr<CTPOrderField>>), this);
    inner_mail_box_->Subscribe(typeid(std::tuple<InputOrder>), this);
    inner_mail_box_->Subscribe(typeid(std::tuple<CancelOrderSignal>), this);
  };

  virtual caf::behavior make_behavior() override {
    return {
        [=](const std::shared_ptr<CTPOrderField>& order) {
          auto it = instrument_brokers_.find(order->instrument);
          BOOST_ASSERT(it != instrument_brokers_.end());
          if (it != instrument_brokers_.end()) {
            it->second->HandleRtnOrder(order);
          }
        },
        [=](const InputOrder& order) {
          auto it = instrument_brokers_.find(order.instrument);
          if (it != instrument_brokers_.end()) {
            it->second->HandleInputOrder(order);
          } else {
            auto broker = std::make_unique<CTPInstrumentBroker>(
                this, order.instrument, false,
                std::bind(&CAFSubAccountBroker::GenerateOrderId, this));
            broker->SetPositionEffectStrategy<
                GenericCTPPositionEffectStrategy,
                GenericCTPPositionEffectFlagStrategy>();
            broker->HandleInputOrder(order);
            instrument_brokers_.insert({order.instrument, std::move(broker)});
          }
        },
        [=](const CancelOrderSignal& cancel) {
          auto it = instrument_brokers_.find(cancel.instrument);
          BOOST_ASSERT(it != instrument_brokers_.end());
          if (it != instrument_brokers_.end()) {
            it->second->HandleCancel(cancel);
          }
        },
    };
  }

  virtual void EnterOrder(const CTPEnterOrder& enter_order) override {
    common_mail_box_->Send(account_id_, enter_order);
  }

  virtual void CancelOrder(const CTPCancelOrder& cancel_order) override {
    common_mail_box_->Send(account_id_, cancel_order);
  }

  virtual void ReturnOrderField(
      const std::shared_ptr<OrderField>& order) override {
    inner_mail_box_->Send(order);
  }

 private:
  std::string GenerateOrderId() {
    return boost::lexical_cast<std::string>(order_seq_++);
  }
  caf::actor ctp_actor_;
  LiveTradeMailBox* inner_mail_box_;
  LiveTradeMailBox* common_mail_box_;
  std::unordered_map<std::string, std::unique_ptr<CTPInstrumentBroker>>
      instrument_brokers_;
  std::string account_id_;
  int order_seq_ = 0;
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
  bool running = true;
  double init_cash = 50 * 10000;
  int delayed_input_order_by_minute = 10;
  int cancel_order_after_minute = 10;
  int backtesting_position_effect = 0;

  std::string account_id = "foo";

  LiveTradeMailBox common_mail_box;
  LiveTradeMailBox inner_mail_box;

  auto cta = system.spawn<CAFCTAOrderSignalBroker>(&common_mail_box);

  DelayedOpenStrategyEx::StrategyParam param;
  param.delayed_open_after_seconds = 5;
  param.price_offset_rate = 0.01;
  system.spawn<CAFDelayOpenStrategyAgent>(std::move(param), &inner_mail_box,
                                          &common_mail_box);
  auto sub_account = system.spawn<CAFSubAccountBroker>(
      &inner_mail_box, &common_mail_box, account_id);

  auto support_sub_account_broker = system.spawn<SupportSubAccountBroker>(
      &common_mail_box, std::vector<std::pair<std::string, caf::actor>>{
                            std::make_pair(account_id, sub_account)});
  // LiveTradeBrokerHandler<LiveTradeMailBox>
  // live_trade_borker_handler(&mail_box);

  auto data_feed = system.spawn<LiveTradeDataFeedHandler>(&common_mail_box);

  // PortfolioHandler<CAFMailBox> portfolio_handler_(
  //    init_cash, &mail_box, instrument, csv_file_prefix, 0.1, 10,
  //    CostBasis{CommissionType::kFixed, 165, 165, 165});

  // mail_box.FinishInitital();

  // live_trade_data_feed_handler.SubscribeInstrument(instrument);

  caf::anon_send(support_sub_account_broker, CtpConnectAtom::value,
                 "tcp://180.168.146.187:10000", "9999", "099344", "a12345678");

  caf::anon_send(cta, CtpConnectAtom::value, "tcp://180.168.146.187:10000",
                 "9999", "053867", "8661188");
  caf::anon_send(data_feed, CtpConnectAtom::value, "tcp://180.166.11.33:41213",
                 "4200", "15500011", "Yunqizhi2_");

  // live_trade_borker_handler.Connect();

  // int qty;
  // while (std::cin >> qty) {
  //  mail_box.Send(qty);
  //}

  system.await_all_actors_done();

  std::cout << "espces:"
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   hrc::now() - beg)
                   .count()
            << "\n";
  return 0;
}

CAF_MAIN()
