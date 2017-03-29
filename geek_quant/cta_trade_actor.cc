#include "cta_trade_actor.h"

CtaTradeActor::CtaTradeActor(caf::actor_config& cfg, caf::actor actor)
    : caf::event_based_actor(cfg),
      ctp_(this, "trader_"),
      order_dispatcher_(true),
      actor_(actor) {}

void CtaTradeActor::Start(const std::string& front_server,
                          const std::string& broker_id,
                          const std::string& user_id,
                          const std::string& password) {
  ctp_.LoginServer(front_server, broker_id, user_id, password);
}

void CtaTradeActor::OnRtnOrderData(CThostFtdcOrderField* field) {
  if (auto order = order_dispatcher_.HandleRtnOrder(*field)) {
    caf::anon_send(actor_, OrderRtnForTrader::value, *order);
  }
}

void CtaTradeActor::OnLogon() {
  delayed_send(this, std::chrono::seconds(1), QryInvestorPositionsAtom::value);
}

void CtaTradeActor::OnPositions(std::vector<OrderPosition> positions) {
  caf::anon_send(actor_, YesterdayPositionForTraderAtom::value,
                 std::move(positions));
}

void CtaTradeActor::OnSettlementInfoConfirm() {}

caf::behavior CtaTradeActor::make_behavior() {
  Start("tcp://59.42.241.91:41205", "9080", "38030022", "140616");
  // Start("tcp://180.168.146.187:10000", "9999", "053867", "8661188");
  return {
      [=](QryInvestorPositionsAtom) { ctp_.QryInvestorPosition(); },
  };
}
