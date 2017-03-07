#ifndef LIBGEEKQUANT_SYMBOL_ORDER_AGENT_H
#define LIBGEEKQUANT_SYMBOL_ORDER_AGENT_H

#include "caf_defines.h"

class InstrumentOrderAgent {
 public:
  InstrumentOrderAgent(OrderAgentActor::base* delegate,
                       OrderSubscriberActor subscriber,
                       const std::string& instrument);

  const std::string& instrument() const;

  void AddOrderRtnData(const OrderRtnData& order);

  void AddPositionData(const PositionData& position);

  void AddPendingEnterOrder(const EnterOrderData& enter_order);

  void OnOrderOpened(const OrderRtnData& order);

  void OnOrderClosed(const OrderRtnData& order);

  void OnOrderCancel(const OrderRtnData& order);

  void HandleEnterOrder(const EnterOrderData& enter_order);

  void HandleCancelOrder(const std::string& instrument,
                         const std::string& order_no);

  void ProcessPendingEnterOrder();

  bool IsEmpty() const;

 private:
  std::string instrument_;
  std::vector<OrderRtnData> pending_orders_;
  std::vector<EnterOrderData> pending_enter_orders_;
  std::vector<PositionData> positions_;

  OrderAgentActor::base* delegate_;
  OrderSubscriberActor subscriber_;
};

#endif  // LIBGEEKQUANT_SYMBOL_ORDER_AGENT_H
