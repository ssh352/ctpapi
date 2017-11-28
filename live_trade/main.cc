#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/log/sources/logger.hpp>
#include <fstream>
#include <boost/format.hpp>
#include <boost/assign.hpp>
#include <boost/any.hpp>
#include <boost/algorithm/string.hpp>
#include "caf/all.hpp"
#include "common/api_struct.h"
#include "caf_common/caf_atom_defines.h"
// #include "hpt_core/backtesting/execution_handler.h"
// #include "hpt_core/backtesting/price_handler.h"
// #include "hpt_core/backtesting/backtesting_mail_box.h"
#include "hpt_core/cta_transaction_series_data_base.h"
#include "hpt_core/tick_series_data_base.h"
//#include "hpt_core/portfolio_handler.h"
//#include "strategies/strategy.h"
//#include "strategies/wr_strategy.h"
// #include "caf_mail_box.h"
#include "live_trade_data_feed_handler.h"
//#include "strategies/key_input_strategy.h"
#include "follow_strategy/delayed_open_strategy_ex.h"
#include "support_sub_account_broker.h"
#include "ctp_broker/ctp_order_delegate.h"
#include "ctp_broker/ctp_instrument_broker.h"
#include "caf_common/caf_atom_defines.h"
#include "follow_strategy/cta_order_signal_subscriber.h"
#include "live_trade_broker_handler.h"
#include "live_trade_mail_box.h"
#include "ctp_rtn_order_subscriber.h"
#include "follow_strategy/delay_open_strategy_agent.h"
#include "local_ctp_trade_api_provider.h"
#include "remote_ctp_trade_api_provider.h"
#include "caf/io/all.hpp"
#include "follow_strategy/optimal_open_price_strategy.h"
#include "serialization_rtn_order.h"
#include "caf_strategy_agent.h"
#include "sub_account_broker.h"
#include "init_instrument_list.h"

//#include "live_trade_broker_handler.h"
class AbstractExecutionHandler;

caf::behavior RemoteTradeApiHandler(caf::event_based_actor* self,
                                    RemoteCtpApiTradeApiProvider* provider) {
  return {
      [=](std::vector<OrderPosition> yesterday_positions) {
        provider->HandleRspYesterdayPosition(std::move(yesterday_positions));
      },
      [=](CTPOrderField order) {
        provider->HandleCTPRtnOrder(
            std::make_shared<CTPOrderField>(std::move(order)));
      },
      [=](const std::string& instrument, const std::string& order_id,
          double trading_price, int trading_qty, TimeStamp timestamp) {
        provider->HandleCTPTradeOrder(instrument, order_id, trading_price,
                                      trading_qty, timestamp);
      },
      [=](int front_id, int session_id) {
        provider->HandleCTPLogon(front_id, session_id);
      }};
}

class config : public caf::actor_system_config {
 public:
  int delayed_close_minutes = 10;
  int cancel_after_minutes = 10;
  int position_effect = 0;

  config() {
    add_message_type<std::vector<OrderPosition>>("order_position_list");
    add_message_type<CTPEnterOrder>("ctp_enter_order");
    add_message_type<CTPCancelOrder>("ctp_cancel_order");
    add_message_type<CTPOrderField>("ctp_order");
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
  InitInstrumenList(system, "tcp://ctp1-front3.citicsf.com:41205", "66666",
                    "120301760", "140616");

      pt::ptree strategy_config_pt;
  try {
    pt::read_json("strategy_config.json", strategy_config_pt);
  } catch (pt::ptree_error& err) {
    std::cout << "Read Confirg File Error:" << err.what() << "\n";
    return 1;
  }

  // system.registry().put(SerializeCtaAtom::value,
  //                      caf::actor_cast<caf::strong_actor_ptr>(
  //                          system.spawn<SerializationCtaRtnOrder>()));
  // system.registry().put(SerializeStrategyAtom::value,
  //                      caf::actor_cast<caf::strong_actor_ptr>(
  //                          system.spawn<SerializationStrategyRtnOrder>()));

  using hrc = std::chrono::high_resolution_clock;
  auto beg = hrc::now();
  bool running = true;

  std::vector<std::string> sub_acconts;
  for (int i = 0; i < 2; ++i) {
    sub_acconts.push_back(str(boost::format("a%d") % i));
  }
  // auto sub_acconts = {"foo"};
  LiveTradeMailBox common_mail_box;

  std::vector<std::unique_ptr<LiveTradeMailBox>> inner_mail_boxs;
  std::vector<std::pair<std::string, caf::actor>> sub_actors;
  auto cta = system.spawn<CAFCTAOrderSignalBroker>(&common_mail_box);

  system.spawn<SerializationCtaRtnOrder>(&common_mail_box);

  for (const auto& account : sub_acconts) {
    auto inner_mail_box = std::make_unique<LiveTradeMailBox>();
    // DelayedOpenStrategyEx::StrategyParam param;
    // param.delayed_open_after_seconds = 5;
    // param.price_offset = 0;
    std::unordered_map<std::string, OptimalOpenPriceStrategy::StrategyParam>
        params;
    for (const auto& pt : strategy_config_pt) {
      try {
        if (pt.first == "backtesting")
          continue;
        OptimalOpenPriceStrategy::StrategyParam param;
        param.delayed_open_after_seconds =
            pt.second.get<int>("DelayOpenOrderAfterSeconds");
        param.wait_optimal_open_price_fill_seconds =
            pt.second.get<int>("WaitOptimalOpenPriceFillSeconds");
        param.price_offset = pt.second.get<double>("PriceOffset");
        params.insert({pt.first, std::move(param)});

      } catch (pt::ptree_error& err) {
        std::cout << "Read Confirg File Error:" << pt.first << ":" << err.what()
                  << "\n";
        return {};
      }
    }
    system.spawn<SerializationStrategyRtnOrder>(inner_mail_box.get(), account);
    // system.spawn<SerializationCtaRtnOrder>();
    system.spawn<CAFDelayOpenStrategyAgent>(
        std::move(params), inner_mail_box.get(), &common_mail_box);
    sub_actors.push_back(std::make_pair(
        account, system.spawn<CAFSubAccountBroker>(inner_mail_box.get(),
                                                   &common_mail_box, account)));
    inner_mail_boxs.push_back(std::move(inner_mail_box));
  }

  //////////////////////////////////////////////////////////////////////////
  LocalCtpTradeApiProvider local_ctcp_trade_api_provider;
  auto support_sub_account_broker = system.spawn<SupportSubAccountBroker>(
      &common_mail_box, &local_ctcp_trade_api_provider, sub_actors);
  local_ctcp_trade_api_provider.Connect("tcp://ctp1-front3.citicsf.com:41205",
                                        "66666", "120301760", "140616");

  //////////////////////////////////////////////////////////////////////////
  // RemoteCtpApiTradeApiProvider remote_ctp_trade_api_provider;

  // auto remote_trade_api_handler =
  //    system.spawn(RemoteTradeApiHandler, &remote_ctp_trade_api_provider);

  // remote_ctp_trade_api_provider.SetRemoteHandler(remote_trade_api_handler);

  // auto remote_trade_api = system.middleman().remote_actor("127.0.0.1", 4242);
  // if (!remote_trade_api) {
  //  std::cerr << "unable to connect to rohon:"
  //            << system.render(remote_trade_api.error()) << std::endl;
  //  return 0;
  //} else {
  //  caf::send_as(remote_trade_api_handler, *remote_trade_api,
  //               RemoteCTPConnectAtom::value);
  //  remote_ctp_trade_api_provider.SetRemoteTradeApi(*remote_trade_api);
  //}

  // auto support_sub_account_broker = system.spawn<SupportSubAccountBroker>(
  //    &common_mail_box, &remote_ctp_trade_api_provider, sub_actors);
  //////////////////////////////////////////////////////////////////////////

  auto data_feed = system.spawn<LiveTradeDataFeedHandler>(&common_mail_box);

  // caf::anon_send(support_sub_account_broker, CtpConnectAtom::value,
  //             "tcp://180.168.146.187:10001", "9999", "099344",
  //             "a12345678");

  // caf::anon_send(cta, CtpConnectAtom::value, "tcp://180.168.146.187:10001",
  //               "9999", "053867", "8661188");

  // caf::anon_send(support_sub_account_broker, CtpConnectAtom::value,
  //               "tcp://ctp1-front3.citicsf.com:41205", "66666", "120301760",
  //               "140616");

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

CAF_MAIN(caf::io::middleman)
