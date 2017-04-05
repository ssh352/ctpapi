#ifndef FOLLOW_TRADE_SERVER_CTA_TRADE_ACTOR_H
#define FOLLOW_TRADE_SERVER_CTA_TRADE_ACTOR_H
#include "caf/all.hpp"
#include "geek_quant/ctp_trader.h"

class CtaTradeActor : public caf::event_based_actor,
                      public CtpTrader::Delegate {
 public:
  CtaTradeActor(caf::actor_config& cfg);

  void Start(const std::string& front_server,
             const std::string& broker_id,
             const std::string& user_id,
             const std::string& password);

  virtual void OnRtnOrderData(CThostFtdcOrderField* order) override;

  virtual void OnLogon() override;

  virtual void OnPositions(std::vector<OrderPosition> positions) override;

  virtual void OnSettlementInfoConfirm() override;

 protected:
  virtual caf::behavior make_behavior() override;

 private:
  CtpTrader ctp_;
  CtpOrderDispatcher order_dispatcher_;
  std::vector<OrderRtnData> restart_rtn_orders_;
  caf::detail::make_response_promise_helper<bool>::type logon_response_;
  caf::detail::make_response_promise_helper<std::vector<OrderPosition>>::type
      positions_response_;

  caf::detail::make_response_promise_helper<std::vector<OrderRtnData>>::type
      restart_rtn_orders_response_promise_;
  size_t last_check_rtn_order_size_;
};

#endif  // FOLLOW_TRADE_SERVER_CTA_TRADE_ACTOR_H
