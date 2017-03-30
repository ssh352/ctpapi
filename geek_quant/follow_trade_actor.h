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

  virtual void OnPositions(std::vector<OrderPosition> positions) override;

  virtual void OnSettlementInfoConfirm() override;

 protected:
  virtual caf::behavior make_behavior() override;

 private:
  CThostFtdcInputOrderActionField MakeCtpCancelOrderAction(
      const OrderIdent& order_ident) const;
  CThostFtdcInputOrderField MakeCtpOrderInsert(
      const EnterOrderData& enter_order) const;
  InstrumentFollow& GetInstrumentFollow(const std::string& instrument);

  void TrySyncPositionIfReady();

  std::string RemapOrderNo(const std::string& order_no,
                           int session_id,
                           RequestBy request_by);

  std::map<std::string, OrderIdent> unfill_orders_;
  CtpOrderDispatcher ctp_order_dispatcher_;
  CtpTrader ctp_;
  std::map<std::string, InstrumentFollow> instrument_follow_set_;
  std::map<std::pair<std::string, int>, int> remap_order_ref_;

  int trader_order_rtn_seq_;
  int follower_order_rtn_seq_;
  int last_check_trader_order_rtn_seq_;
  int last_check_follower_order_rtn_seq_;
  bool wait_sync_orders_;
  std::vector<OrderPosition> trader_positions_;
  std::vector<OrderPosition> follower_positions_;

  std::vector<OrderRtnData> pending_trader_rtn_orders_;
  std::vector<OrderRtnData> pending_follower_rtn_orders_;

  int max_order_no_;
  bool wait_sync_position_;
  bool wait_yesterday_trader_position_;
  bool wait_yesterday_follower_position_;
};

#endif  // STRATEGY_UNITTEST_FOLLOW_TRADE_ACTOR_H
