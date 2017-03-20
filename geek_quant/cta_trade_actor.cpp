#include "cta_trade_actor.h"

CtaTradeActor::CtaTradeActor(caf::actor actor)
    : ctp_(this, "trader_"), actor_(actor) {}

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
