#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "rong_hang_trader/rohon_trade_api.h"
#include "caf_common/caf_atom_defines.h"

class config : public caf::actor_system_config {
 public:
  config() {
    add_message_type<std::vector<OrderPosition>>("order_position_list");
    add_message_type<CTPEnterOrder>("ctp_enter_order");
    add_message_type<CTPCancelOrder>("ctp_cancel_order");
    add_message_type<CTPOrderField>("ctp_order");
  }
};

class RohonTradeApiActor : public caf::event_based_actor,
                           public RohonTradeApi::Delegate {
 public:
  RohonTradeApiActor(caf::actor_config& cfg) : event_based_actor(cfg) {
    trader_api_ = std::make_unique<RohonTradeApi>(this, "");
  }

  caf::behavior make_behavior() override {
    trader_api_->Connect("tcp://210.22.96.58:7001", "RohonDemo", "zjqhy01",
                         "888888");
    return {
        [=](const CTPEnterOrder& order, const std::string& order_id) {
          std::cout << "Input Order\n";
          trader_api_->InputOrder(order, order_id);
        },
        [=](const CTPCancelOrder& cancel) { 
          std::cout << "Cancel Order\n";
          trader_api_->CancelOrder(cancel); 
        },
        [=](ReqYesterdayPositionAtom) {
          return std::vector<OrderPosition>(); 
        },
        [=](const std::shared_ptr<CTPOrderField>& order) {
          send(handler_, *order);
        },
        [=](const std::string& instrument, const std::string& order_id,
            double trading_price, int trading_qty, TimeStamp timestamp) {
          send(handler_, instrument, order_id, trading_price, trading_qty, timestamp);
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
    send(handler_, *order);
  }

  virtual void HandleCTPTradeOrder(const std::string& instrument,
                                   const std::string& order_id,
                                   double trading_price,
                                   int trading_qty,
                                   TimeStamp timestamp) override {
    send(this, instrument, order_id, trading_price, trading_qty, timestamp);
  }

  virtual void HandleRspYesterdayPosition(
      std::vector<OrderPosition> yesterday_positions) override {}

 private:
  std::unique_ptr<RohonTradeApi> trader_api_;
  caf::actor handler_;
  int front_id_ = -1;
  int session_id_ = -1;
};

int caf_main(caf::actor_system& system, const config& cfg) {
  auto ping = system.spawn<RohonTradeApiActor>();
  system.middleman().publish(ping, 4242);
  return 0;
}

CAF_MAIN(caf::io::middleman)
