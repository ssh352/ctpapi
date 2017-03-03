#include "order_agent.h"

OrderAgentActor::behavior_type OrderAgent::make_behavior() {
  return {[=](TAPositionAtom, std::vector<PositionData> positions) {

          },
          [](TAUnfillOrdersAtom, std::vector<OrderRtnData> orders) {

          },
          [](TARtnOrderAtom, OrderRtnData order) {},
          [](EnterOrderAtom, EnterOrderData enter_order) {},
          [](CancelOrderAtom, std::string order_no) {}};
}
