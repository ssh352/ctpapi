#ifndef STRATEGY_UNITTEST_ORDER_AGENT_H
#define STRATEGY_UNITTEST_ORDER_AGENT_H
#include "caf_defines.h"

class OrderAgent : public OrderAgentActor::base {
 public:
  OrderAgent(caf::actor_config& cfg) : OrderAgentActor::base(cfg) {
    wait_for_until_receive_positions_ = false;
    wait_for_until_receive_unfill_orders_ = false;
  }
  OrderAgentActor::behavior_type make_behavior();

 private:
  void OnOrderOpened(const OrderRtnData& order);
  void OnOrderCanceled(const OrderRtnData& order);
  void HandleEnterOrder(const EnterOrderData& enter_order);
  void OnOrderClosed(const OrderRtnData& order);
  void TryProcessPendingEnterOrder();
  OrderSubscriberActor subscriber_;
  std::vector<OrderRtnData> pending_orders_;
  std::vector<EnterOrderData> pending_enter_orders_;
  std::vector<PositionData> positions_;
  bool wait_for_until_receive_unfill_orders_;
  bool wait_for_until_receive_positions_;
  bool ReadyToEnterOrder() const;
};

#endif  // STRATEGY_UNITTEST_ORDER_AGENT_H
