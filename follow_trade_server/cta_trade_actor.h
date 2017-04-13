#ifndef FOLLOW_TRADE_SERVER_CTA_TRADE_ACTOR_H
#define FOLLOW_TRADE_SERVER_CTA_TRADE_ACTOR_H
#include "caf/all.hpp"
#include "follow_trade_server/ctp_trader.h"

class CtpTrader : public caf::event_based_actor, public CtpApi::Delegate {
 public:
  CtpTrader(caf::actor_config& cfg,
            const std::string& front_server,
            const std::string& broker_id,
            const std::string& user_id,
            const std::string& password);

  virtual void OnOrderData(CThostFtdcOrderField* order) override;

  virtual void OnLogon(int frton_id, int session_id, bool success) override;

  virtual void OnPositions(std::vector<OrderPosition> positions) override;

  virtual void OnSettlementInfoConfirm() override;


  void on_exit() override {
    rtn_orders_subscribers_.clear();
  }
 protected:
  virtual caf::behavior make_behavior() override;

 private:
  CtpApi ctp_;
  std::string front_server_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  int front_id_;
  int session_id_;

  std::vector<OrderData> rtn_orders_;

  std::vector<caf::strong_actor_ptr> rtn_orders_subscribers_;

  typedef caf::detail::make_response_promise_helper<bool>::type
      BoolResponsePromise;
  std::vector<BoolResponsePromise> logon_response_promises_;

  BoolResponsePromise  settlement_info_confirm_response_promises_;

  typedef caf::detail::make_response_promise_helper<
      std::vector<OrderPosition>>::type PositionsResponsePromise;
  std::vector<PositionsResponsePromise> positions_response_promises;

};

#endif  // FOLLOW_TRADE_SERVER_CTA_TRADE_ACTOR_H
