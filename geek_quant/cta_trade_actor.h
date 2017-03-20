#ifndef FOLLOW_TRADE_SERVER_CTA_TRADE_ACTOR_H
#define FOLLOW_TRADE_SERVER_CTA_TRADE_ACTOR_H
#include "caf/all.hpp"
#include "geek_quant/ctp_trader.h"

class CtaTradeActor : public CtpTrader::Delegate {
 public:
  CtaTradeActor(caf::actor actor);

  void Start(const std::string& front_server,
             const std::string& broker_id,
             const std::string& user_id,
             const std::string& password);

  virtual void OnRtnOrderData(CThostFtdcOrderField* order) override;

 protected:
 private:
  CtpTrader ctp_;
  caf::actor actor_;
  CtpOrderDispatcher order_dispatcher_;
};

#endif  // FOLLOW_TRADE_SERVER_CTA_TRADE_ACTOR_H
