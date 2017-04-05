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
    send(this, CTPRtnOrderAtom::value, *order);
  }
}

void CtaTradeActor::OnLogon() {
  send(this, CTPRspLogin::value);
}

void CtaTradeActor::OnPositions(std::vector<OrderPosition> positions) {
  send(this, CTPRspQryInvestorPositionsAtom::value, positions);
}

void CtaTradeActor::OnSettlementInfoConfirm() {}

caf::behavior CtaTradeActor::make_behavior() {
  // Start("tcp://180.168.146.187:10000", "9999", "053867", "8661188");
  return {
      [=](CTPLogin, const std::string& front_server,
          const std::string& broker_id, const std::string& user_id,
          const std::string& password) -> caf::result<bool> {
        auto logon_response_ = make_response_promise<bool>();
        logon_response_promises_.push_back(logon_response_);
        Start(front_server, broker_id, user_id, password);
        return logon_response_;
      },
      [=](CTPRspLogin) {
        for (auto promise : logon_response_promises_) {
          promise.deliver(true);
        }
        logon_response_promises_.clear();
      },
      [=](CTPQryInvestorPositionsAtom)
          -> caf::result<std::vector<OrderPosition> > {
        auto promise = make_response_promise<std::vector<OrderPosition> >();
        positions_response_promises.push_back(promise);
        ctp_.QryInvestorPosition();
        return promise;
      },
      [=](CTPRspQryInvestorPositionsAtom,
          std::vector<OrderPosition> positions) {
        for (auto promise : positions_response_promises) {
          promise.deliver(positions);
        }
        positions_response_promises.clear();
      },
      [=](CTPRtnOrderAtom, OrderRtnData order) {
        restart_rtn_orders_.push_back(order);
      },
      [=](CTPReqRestartRtnOrdersAtom, const caf::strong_actor_ptr& actor)
          -> caf::result<std::vector<OrderRtnData> > {
        auto promise = make_response_promise<std::vector<OrderRtnData> >();
        restart_rtn_orders_response_promises_.push_back(promise);
        last_check_rtn_order_size_ = restart_rtn_orders_.size();
        delayed_send(this, std::chrono::seconds(1), ActorTimerAtom::value);
        return promise;
      },
      [=](ActorTimerAtom) {
        if (last_check_rtn_order_size_ == restart_rtn_orders_.size()) {
          for (auto promise : restart_rtn_orders_response_promises_) {
            promise.deliver(restart_rtn_orders_);
          }
          restart_rtn_orders_response_promises_.clear();
        } else {
          last_check_rtn_order_size_ = restart_rtn_orders_.size();
          delayed_send(this, std::chrono::seconds(1), ActorTimerAtom::value);
        }
      },
  };
}
