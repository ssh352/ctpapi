#include "cta_trade_actor.h"

CtaTradeActor::CtaTradeActor(caf::actor_config& cfg)
    : caf::event_based_actor(cfg),
      ctp_(this, "trader_"),
      order_dispatcher_(true) {
  last_check_rtn_order_size_ = -1;
}

void CtaTradeActor::Start(const std::string& front_server,
                          const std::string& broker_id,
                          const std::string& user_id,
                          const std::string& password) {
  ctp_.LoginServer(front_server, broker_id, user_id, password);
}

void CtaTradeActor::OnRtnOrderData(CThostFtdcOrderField* field) {
  if (auto order = order_dispatcher_.HandleRtnOrder(*field)) {
    send(this, OrderRtnForTrader::value, *order);
  }
}

void CtaTradeActor::OnLogon() {
  logon_response_.deliver(true);
  // delayed_send(this, std::chrono::seconds(1),
  // QryInvestorPositionsAtom::value);
}

void CtaTradeActor::OnPositions(std::vector<OrderPosition> positions) {
  // caf::anon_send(actor_, YesterdayPositionForTraderAtom::value,
  //                std::move(positions));
  positions_response_.deliver(positions);
}

void CtaTradeActor::OnSettlementInfoConfirm() {}

caf::behavior CtaTradeActor::make_behavior(){
    // Start("tcp://180.168.146.187:10000", "9999", "053867", "8661188");
    return {
        [=](StartAtom) -> caf::result<bool> {
          logon_response_ = make_response_promise<bool>();
          Start("tcp://59.42.241.91:41205", "9080", "38030022", "140616");
          return logon_response_;
        },
        [=](OrderRtnForTrader, OrderRtnData order) {
          restart_rtn_orders_.push_back(order);
        },
        [=](RestartRtnOrdersAtom) -> caf::result<std::vector<OrderRtnData> > {
          restart_rtn_orders_response_promise_ =
              make_response_promise<std::vector<OrderRtnData> >();
          last_check_rtn_order_size_ = restart_rtn_orders_.size();
          delayed_send(this, std::chrono::seconds(1), ActorTimerAtom::value);
          return restart_rtn_orders_response_promise_;
        },
        [=](QryInvestorPositionsAtom)
            -> caf::result<std::vector<OrderPosition> > {
          positions_response_ =
              make_response_promise<std::vector<OrderPosition> >();
          ctp_.QryInvestorPosition();
          return positions_response_;
        },
        [=](ActorTimerAtom) {
          if (last_check_rtn_order_size_ == restart_rtn_orders_.size()) {
            restart_rtn_orders_response_promise_.deliver(restart_rtn_orders_);
          } else {
            last_check_rtn_order_size_ = restart_rtn_orders_.size();
            delayed_send(this, std::chrono::seconds(1), ActorTimerAtom::value);
          }
        },
    };
}
