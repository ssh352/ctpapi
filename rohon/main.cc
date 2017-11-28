#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "rohon/rohon_trade_api.h"
#include "caf_common/caf_atom_defines.h"

class config : public caf::actor_system_config {
 public:
  config() {
    add_message_type<std::vector<OrderPosition>>("order_position_list");
    add_message_type<CTPEnterOrder>("ctp_enter_order");
    add_message_type<CTPCancelOrder>("ctp_cancel_order");
    add_message_type<CTPOrderField>("ctp_order");
    opt_group{custom_options_, "global"}
        .add(port, "port,p", "listen port")
        .add(server, "server", "rohon server tcp")
        .add(broker_id, "broker_id", "rohon broker id")
        .add(user_id, "user_id", "rohon user_id")
        .add(password, "password", "rohon password");
  }

  int port;
  std::string server;
  std::string broker_id;
  std::string user_id;
  std::string password;
};

class RohonTradeApiActor : public caf::event_based_actor,
                           public RohonTradeApi::Delegate {
 public:
  RohonTradeApiActor(caf::actor_config& cfg,
                     std::string server,
                     std::string broker_id,
                     std::string user_id,
                     std::string password)
      : event_based_actor(cfg),
        server_(std::move(server)),
        broker_id_(std::move(broker_id)),
        user_id_(std::move(user_id)),
        password_(std::move(password)) {
    trader_api_ = std::make_unique<RohonTradeApi>(this, "");
  }

  caf::behavior make_behavior() override {
    // trader_api_->Connect("tcp://210.22.96.58:7001", "RohonDemo", "zjqhy01",
    //                     "888888");
    trader_api_->Connect(server_, broker_id_, user_id_, password_);
    return {
        [=](const CTPEnterOrder& order, const std::string& order_id) {
          std::cout << "Input Order:" << order.instrument << " "
                    << (order.direction == OrderDirection::kBuy ? "B" : "S")
                    << " "
                    << (order.position_effect == CTPPositionEffect::kOpen ? "O"
                                                                          : "C")
                    << "\n";
          trader_api_->InputOrder(order, order_id);
        },
        [=](const CTPCancelOrder& cancel) {
          std::cout << "Cancel Order\n";
          trader_api_->CancelOrder(cancel);
        },
        [=](ReqYesterdayPositionAtom) { return std::vector<OrderPosition>(); },
        [=](const std::shared_ptr<CTPOrderField>& order) {
          send(handler_, *order);
        },
        [=](const std::string& instrument, const std::string& order_id,
            double trading_price, int trading_qty, TimeStamp timestamp) {
          send(handler_, instrument, order_id, trading_price, trading_qty,
               timestamp);
        },
        [=](RemoteCTPConnectAtom) {
          std::cout << "Connected\n";
          handler_ = caf::actor_cast<caf::actor>(current_sender());
          send(handler_, front_id_, session_id_);
        },
    };
  }

  virtual void HandleLogon(int front_id, int session_id) override {
    front_id_ = front_id;
    session_id_ = session_id;
  }

  virtual void HandleCTPRtnOrder(
      const std::shared_ptr<CTPOrderField>& order) override {
    if (handler_ != nullptr) {
      send(handler_, *order);
    }
  }

  virtual void HandleCTPTradeOrder(const std::string& instrument,
                                   const std::string& order_id,
                                   double trading_price,
                                   int trading_qty,
                                   TimeStamp timestamp) override {
    if (handler_ != nullptr) {
      send(this, instrument, order_id, trading_price, trading_qty, timestamp);
    }
  }

  virtual void HandleRspYesterdayPosition(
      std::vector<OrderPosition> yesterday_positions) override {}

 private:
  std::unique_ptr<RohonTradeApi> trader_api_;
  caf::actor handler_;
  int front_id_ = -1;
  int session_id_ = -1;
  std::string server_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
};

int caf_main(caf::actor_system& system, const config& cfg) {
  auto trade_api = system.spawn<RohonTradeApiActor>(cfg.server, cfg.broker_id,
                                                    cfg.user_id, cfg.password);
  system.middleman().publish(trade_api, cfg.port);
  std::string input;

  while (std::cin >> input) {
    if (input == "exit") {
      anon_send_exit(trade_api, caf::exit_reason::kill);
      system.middleman().unpublish(trade_api, 0);
      break;
    }
  }
  return 0;
}

CAF_MAIN(caf::io::middleman)
