#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/any.hpp>
#include <boost/algorithm/string.hpp>
#include "caf/all.hpp"
#include "common/api_struct.h"
#include "live_trade/caf_atom_defines.h"
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
#include "follow_strategy/delay_open_strategy_agent.h"

std::unordered_map<std::string, std::string> g_instrument_exchange_set = {
    {"a", "dc"},  {"al", "sc"}, {"bu", "sc"}, {"c", "dc"},  {"cf", "zc"},
    {"cs", "dc"}, {"cu", "sc"}, {"fg", "zc"}, {"i", "dc"},  {"j", "dc"},
    {"jd", "dc"}, {"jm", "dc"}, {"l", "dc"},  {"m", "dc"},  {"ma", "zc"},
    {"ni", "sc"}, {"p", "dc"},  {"pp", "dc"}, {"rb", "sc"}, {"rm", "zc"},
    {"ru", "sc"}, {"sm", "zc"}, {"sr", "zc"}, {"ta", "zc"}, {"v", "dc"},
    {"y", "dc"},  {"zc", "zc"}, {"zn", "sc"}, {"zn", "sc"}};

std::unordered_set<std::string> g_close_today_cost_instrument_codes = {
    "fg", "hc", "i", "j", "jm", "ma", "ni", "pb", "rb", "sf", "sm", "zc", "zn"};
//#include "live_trade_broker_handler.h"
class AbstractExecutionHandler;

class CAFDelayOpenStrategyAgent : public caf::event_based_actor {
 public:
  CAFDelayOpenStrategyAgent(
      caf::actor_config& cfg,
      std::unordered_map<std::string, DelayedOpenStrategyEx::StrategyParam>
          params,
      LiveTradeMailBox* inner_mail_box,
      LiveTradeMailBox* common_mail_box)
      : caf::event_based_actor(cfg),
        agent_(this, std::move(params)),
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
    inner_mail_box_->Subscribe(typeid(std::tuple<CancelOrder>), this);
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
        [=](const std::string& instrument, const std::string& order_id,
            double trading_price, int trading_qty, TimeStamp timestamp) {
          auto it = instrument_brokers_.find(instrument);
          BOOST_ASSERT(it != instrument_brokers_.end());
          if (it != instrument_brokers_.end()) {
            it->second->HandleTraded(order_id, trading_price, trading_qty,
                                     timestamp);
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
            std::string instrument_code = order.instrument.substr(
                0, order.instrument.find_first_of("0123456789"));
            boost::algorithm::to_lower(instrument_code);
            bool close_today_cost = false;
            bool close_today_aware = false;
            if (g_instrument_exchange_set.find(instrument_code) !=
                    g_instrument_exchange_set.end() &&
                g_instrument_exchange_set.at(instrument_code) == "sc") {
              close_today_aware = true;
            } else {
            }

            if (g_close_today_cost_instrument_codes.find(instrument_code) !=
                g_close_today_cost_instrument_codes.end()) {
              close_today_cost = true;
            }

            if (close_today_cost && close_today_aware) {
              broker->SetPositionEffectStrategy<
                  CloseTodayCostCTPPositionEffectStrategy,
                  CloseTodayAwareCTPPositionEffectFlagStrategy>();
            } else if (!close_today_cost && close_today_aware) {
              broker->SetPositionEffectStrategy<
                  GenericCTPPositionEffectStrategy,
                  CloseTodayAwareCTPPositionEffectFlagStrategy>();
            } else if (close_today_cost && !close_today_aware) {
              broker->SetPositionEffectStrategy<
                  CloseTodayCostCTPPositionEffectStrategy,
                  GenericCTPPositionEffectFlagStrategy>();
            } else {
              broker->SetPositionEffectStrategy<
                  GenericCTPPositionEffectStrategy,
                  GenericCTPPositionEffectFlagStrategy>();
            }

            broker->HandleInputOrder(order);
            instrument_brokers_.insert({order.instrument, std::move(broker)});
          }
        },
        [=](const CancelOrder& cancel) {
          auto it = instrument_brokers_.find(cancel.instrument);
          BOOST_ASSERT(it != instrument_brokers_.end());
          if (it != instrument_brokers_.end()) {
            it->second->HandleCancel(cancel);
          }
        },
    };
  }

  virtual void HandleEnterOrder(const CTPEnterOrder& enter_order) override {
    common_mail_box_->Send(account_id_, enter_order);
  }

  virtual void HandleCancelOrder(const CTPCancelOrder& cancel_order) override {
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

namespace pt = boost::property_tree;
int caf_main(caf::actor_system& system, const config& cfg) {
  pt::ptree strategy_config_pt;
  try {
    pt::read_json("strategy_config.json", strategy_config_pt);
  } catch (pt::ptree_error& err) {
    std::cout << "Read Confirg File Error:" << err.what() << "\n";
    return 1;
  }
  using hrc = std::chrono::high_resolution_clock;
  auto beg = hrc::now();
  bool running = true;

  auto sub_acconts = {"foo", "bar"};
  // auto sub_acconts = {"foo"};
  LiveTradeMailBox common_mail_box;

  std::vector<std::unique_ptr<LiveTradeMailBox>> inner_mail_boxs;
  std::vector<std::pair<std::string, caf::actor>> sub_actors;
  auto cta = system.spawn<CAFCTAOrderSignalBroker>(&common_mail_box);
  for (const auto& account : sub_acconts) {
    auto inner_mail_box = std::make_unique<LiveTradeMailBox>();
    // DelayedOpenStrategyEx::StrategyParam param;
    // param.delayed_open_after_seconds = 5;
    // param.price_offset = 0;
    std::unordered_map<std::string, DelayedOpenStrategyEx::StrategyParam>
        params;
    for (const auto& pt : strategy_config_pt) {
      try {
        DelayedOpenStrategyEx::StrategyParam param;
        param.delayed_open_after_seconds =
            pt.second.get<int>("DelayOpenOrderAfterSeconds");
        param.price_offset = pt.second.get<double>("PriceOffset");
        params.insert({pt.first, std::move(param)});

      } catch (pt::ptree_error& err) {
        std::cout << "Read Confirg File Error:" << pt.first << ":" << err.what()
                  << "\n";
        return {};
      }
    }
    system.spawn<CAFDelayOpenStrategyAgent>(
        std::move(params), inner_mail_box.get(), &common_mail_box);
    sub_actors.push_back(std::make_pair(
        account, system.spawn<CAFSubAccountBroker>(inner_mail_box.get(),
                                                   &common_mail_box, account)));
    inner_mail_boxs.push_back(std::move(inner_mail_box));
  }

  auto support_sub_account_broker =
      system.spawn<SupportSubAccountBroker>(&common_mail_box, sub_actors);

  auto data_feed = system.spawn<LiveTradeDataFeedHandler>(&common_mail_box);

   //caf::anon_send(support_sub_account_broker, CtpConnectAtom::value,
   //              "tcp://180.168.146.187:10001", "9999", "099344",
   //              "a12345678");
  //caf::anon_send(cta, CtpConnectAtom::value, "tcp://180.168.146.187:10001",
  //               "9999", "053867", "8661188");

  caf::anon_send(support_sub_account_broker, CtpConnectAtom::value,
                 "tcp://ctp1-front3.citicsf.com:41205", "66666", "120301760",
                 "140616");

  caf::anon_send(cta, CtpConnectAtom::value, "tcp://101.231.3.125:41205",
                 "8888", "181006", "140616");


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
