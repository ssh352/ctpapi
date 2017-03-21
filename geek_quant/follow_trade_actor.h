#ifndef STRATEGY_UNITTEST_FOLLOW_TRADE_ACTOR_H
#define STRATEGY_UNITTEST_FOLLOW_TRADE_ACTOR_H
#include "geek_quant/caf_defines.h"
#include "caf/all.hpp"
#include "geek_quant/ctp_trader.h"
#include "geek_quant/ctp_order_dispatcher.h"
#include "geek_quant/instrument_follow.h"

class FollowTradeActor : public caf::event_based_actor,
                         public CtpTrader::Delegate {
 public:
  FollowTradeActor(caf::actor_config& cfg);
  ~FollowTradeActor();
  virtual void OnRtnOrderData(CThostFtdcOrderField* order) override;


  virtual void OnLogon() override;

protected:
  virtual caf::behavior make_behavior() override;

 private:
  CThostFtdcInputOrderActionField MakeCtpCancelOrderAction(
      const OrderIdent& order_ident) const;
  CThostFtdcInputOrderField MakeCtpOrderInsert(
      const EnterOrderData& enter_order) const;
  std::map<std::string, OrderIdent> unfill_orders_;
  CtpOrderDispatcher ctp_order_dispatcher_;
  CtpTrader ctp_;
  std::map<std::string, InstrumentFollow> instrument_follow_set_;
};

#endif  // STRATEGY_UNITTEST_FOLLOW_TRADE_ACTOR_H
