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
#include "yaml-cpp/yaml.h"
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

#include "ctp_rtn_order_subscriber.h"
#include "follow_strategy/delay_open_strategy_agent.h"
#include "local_ctp_trade_api_provider.h"
#include "remote_ctp_trade_api_provider.h"
#include "caf/io/all.hpp"
#include "follow_strategy/optimal_open_price_strategy.h"
#include "serialization_rtn_order.h"
#include "caf_strategy_agent.h"
#include "sub_account_broker.h"
#include "live_trade_logging.h"

//#include "live_trade_broker_handler.h"
class AbstractExecutionHandler;

caf::behavior RemoteTradeApiHandler(
    caf::event_based_actor* self,
    std::shared_ptr<RemoteCtpApiTradeApiProvider> provider) {
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

struct SubAccount {
  std::string name;
  std::string strategy_config;
  std::string broker;
};

namespace pt = boost::property_tree;
int caf_main(caf::actor_system& system, const config& cfg) {

  //auto t = std::make_tuple(100);
  InitLogging();
  ProductInfoMananger product_info_mananger;
  product_info_mananger.LoadFile("product_info.json");
  try {
    YAML::Node config = YAML::LoadFile("accounts.yaml");
    std::vector<SubAccount> sub_acconts;
    std::set<std::string> sub_accountnames;
    for (YAML::const_iterator it = config.begin(); it != config.end(); ++it) {
      auto node = it->as<YAML::Node>();
      SubAccount account;
      account.name = node["Account"].as<std::string>();
      account.strategy_config = node["Strategy"].as<std::string>();
      account.broker = node["Broker"].as<std::string>();
      sub_accountnames.insert(account.name);
      sub_acconts.push_back(account);
    }

    LiveTradeSystem live_trade_system;
    std::set<std::string> enable_brokers;
    auto cta = system.spawn<CAFCTAOrderSignalBroker>(
        &live_trade_system, live_trade_system.GetGlobalEnvionment());

    system.spawn<SerializationCtaRtnOrder>(
        &live_trade_system, live_trade_system.GetGlobalEnvionment());

    std::unordered_set<std::string> close_today_cost_of_product_codes;
    // = {
    //"fg", "i", "j", "jm", "ma", "ni", "pb", "rb", "sf", "sm", "zc", "zn"};
    YAML::Node live_config = YAML::LoadFile("live_trade.yaml");
    YAML::Node sub_config =
        live_config["CostTodayProductCodes"].as<YAML::Node>();
    for (YAML::const_iterator it = sub_config.begin(); it != sub_config.end();
         ++it) {
      close_today_cost_of_product_codes.insert(it->as<std::string>());
    }

    for (const auto& account : sub_acconts) {
      int env_id = live_trade_system.CreateNamedEnvionment(account.name);
      pt::ptree strategy_config_pt;
      try {
        pt::read_json(account.strategy_config, strategy_config_pt);
      } catch (pt::ptree_error& err) {
        std::cout << "Read Confirg File Error:" << err.what() << "\n";
        return 1;
      }
      enable_brokers.insert(account.broker);

      system.spawn<SerializationStrategyRtnOrder>(&live_trade_system, env_id,
                                                  account.name);
      // system.spawn<SerializationCtaRtnOrder>();
      system.spawn<CAFDelayOpenStrategyAgent>(
          &strategy_config_pt, &product_info_mananger, account.name,
          &live_trade_system, env_id);

      system.spawn<CAFSubAccountBroker>(
          &live_trade_system, env_id, &product_info_mananger,
          close_today_cost_of_product_codes, account.broker, account.name);
    }

    YAML::Node broker_config = YAML::LoadFile("brokers.yaml");
    std::vector<caf::actor> brokers;
    for (YAML::const_iterator it = broker_config.begin();
         it != broker_config.end(); ++it) {
      auto item = it->as<YAML::Node>();
      std::string broker = item["Name"].as<std::string>();
      if (enable_brokers.find(broker) == enable_brokers.end()) {
        continue;
      }
      std::string type = item["Type"].as<std::string>();
      if (type == "Local") {
        //////////////////////////////////////////////////////////////////////////
        auto local_ctcp_trade_api_provider =
            std::make_shared<LocalCtpTradeApiProvider>();
        brokers.push_back(system.spawn<SupportSubAccountBroker>(
            &live_trade_system, live_trade_system.CreateNamedEnvionment(broker),
            sub_accountnames, local_ctcp_trade_api_provider));
        // local_ctcp_trade_api_provider.Connect("tcp://ctp1-front3.citicsf.com:41205",
        //                                      "66666", "120301760", "140616");
        local_ctcp_trade_api_provider->Connect(
            item["Server"].as<std::string>(),
            item["BrokerId"].as<std::string>(),
            item["UserId"].as<std::string>(),
            item["Password"].as<std::string>());
      } else if (type == "Remote") {
        auto remote_ctp_trade_api_provider =
            std::make_shared<RemoteCtpApiTradeApiProvider>();

        auto remote_trade_api_handler =
            system.spawn(RemoteTradeApiHandler, remote_ctp_trade_api_provider);

        remote_ctp_trade_api_provider->SetRemoteHandler(
            remote_trade_api_handler);

        auto remote_trade_api = system.middleman().remote_actor(
            item["RemoteServer"].as<std::string>(),
            item["RemotePort"].as<int>());
        if (!remote_trade_api) {
          std::cerr << "unable to connect to rohon:"
                    << system.render(remote_trade_api.error()) << std::endl;
          return 0;
        } else {
          caf::send_as(remote_trade_api_handler, *remote_trade_api,
                       RemoteCTPConnectAtom::value);
          remote_ctp_trade_api_provider->SetRemoteTradeApi(*remote_trade_api);
        }

        brokers.push_back(system.spawn<SupportSubAccountBroker>(
            &live_trade_system, live_trade_system.CreateNamedEnvionment(broker),
            sub_accountnames, remote_ctp_trade_api_provider));
      } else {
        BOOST_ASSERT(false);
      }
    }

    //////////////////////////////////////////////////////////////////////////
    auto data_feed = system.spawn<LiveTradeDataFeedHandler>(
        &live_trade_system, live_trade_system.GetGlobalEnvionment());
    // caf::anon_send(support_sub_account_broker, CtpConnectAtom::value,
    //             "tcp://180.168.146.187:10001", "9999", "099344",
    //             "a12345678");

    caf::anon_send(cta, CtpConnectAtom::value,
                   live_config["CtaBroker"]["Server"].as<std::string>(),
                   live_config["CtaBroker"]["BrokerId"].as<std::string>(),
                   live_config["CtaBroker"]["UserId"].as<std::string>(),
                   live_config["CtaBroker"]["Password"].as<std::string>());

    // caf::anon_send(support_sub_account_broker, CtpConnectAtom::value,
    //               "tcp://ctp1-front3.citicsf.com:41205", "66666",
    //               "120301760", "140616");

    /*caf::anon_send(cta, CtpConnectAtom::value, "tcp://101.231.3.125:41205",
                   "8888", "181006", "140616");*/

    caf::anon_send(data_feed, CtpConnectAtom::value,
                   live_config["DataFeed"]["Server"].as<std::string>(),
                   live_config["DataFeed"]["BrokerId"].as<std::string>(),
                   live_config["DataFeed"]["UserId"].as<std::string>(),
                   live_config["DataFeed"]["Password"].as<std::string>());

    // TODO: Release TradeAPI & MDAPI when exit

    std::string input;
    while (std::cin >> input) {
      if (input == "exit") {
        // TODO:
        // std::for_each(inner_mail_boxs.begin(), inner_mail_boxs.end(),
        //              [](const auto& mail_box) {
        //                mail_box->Send(SerializationFlushAtom::value);
        //              });
        break;
      }
    }

  } catch (YAML::Exception& err) {
    std::cout << err.what();
  }

  system.await_all_actors_done();
  return 0;
}

CAF_MAIN(caf::io::middleman)
