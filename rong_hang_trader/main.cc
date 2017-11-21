#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "rong_hang_trader/rohon_trade_api.h"

class config : public caf::actor_system_config {
 public:
  config() {}
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
          trader_api_->InputOrder(order, order_id);
        },
        [=](const CTPCancelOrder& cancel) { trader_api_->CancelOrder(cancel); },
        [=](const std::string& str) {
          send(caf::actor_cast<caf::actor>(current_sender()), str);
        },
    };
  }

  virtual void HandleLogon() override {}

  virtual void HandleCTPRtnOrder(
      const std::shared_ptr<CTPOrderField>& order) override {}

  virtual void HandleCTPTradeOrder(const std::string& instrument,
                                   const std::string& order_id,
                                   double trading_price,
                                   int trading_qty,
                                   TimeStamp timestamp) override {}

  virtual void HandleRspYesterdayPosition(
      std::vector<OrderPosition> yesterday_positions) override {}

 private:
  std::unique_ptr<RohonTradeApi> trader_api_;
};

int caf_main(caf::actor_system& system) {
  auto ping = system.spawn<RohonTradeApiActor>();
  system.middleman().publish(ping, 4242);
  return 0;
}

CAF_MAIN(caf::io::middleman)
