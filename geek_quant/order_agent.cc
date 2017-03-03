#include "order_agent.h"

OrderAgentActor::behavior_type OrderAgent::make_behavior() {
  return {[=](TAPositionAtom, std::vector<PositionData> positions) {

          },
          [=](TAUnfillOrdersAtom, std::vector<OrderRtnData> orders) {
            pending_orders_ = orders;
          },
          [=](TARtnOrderAtom, OrderRtnData order) {
            switch (order.order_status) {
              case kOSOpening:
                break;
              case kOSCloseing:
                break;
              case kOSOpened:
                HandleOpened(order);
                break;
              case kOSClosed:
                break;
              case kOSCancel:
                HandleCancel(order);
                break;
              default:
                break;
            }
          },
          [=](EnterOrderAtom, EnterOrderData enter_order) {
            if (enter_order.action == EnterOrderAction::kEOAOpen) {
              send(subscriber_, EnterOrderAtom::value, enter_order);
            } else if (enter_order.action == EnterOrderAction::kEOAClose) {
              auto it_pos = std::find_if(
                  positions_.begin(), positions_.end(), [&](auto position) {
                    return enter_order.instrument == position.instrument &&
                           enter_order.order_direction !=
                               position.order_direction;
                  });
              if (it_pos != positions_.end()) {
                enter_order.volume = it_pos->volume;
                send(subscriber_, EnterOrderAtom::value, enter_order);
              }
            } else {
            }
          },
          [=](CancelOrderAtom, std::string order_no) {
            auto it = std::find_if(
                pending_orders_.begin(), pending_orders_.end(),
                [&](auto order) { return order_no == order.order_no; });
            if (it != pending_orders_.end()) {
              send(subscriber_, CancelOrderAtom::value, order_no);
              pending_orders_.erase(it);
            }
          },
          [=](AddStrategySubscriberAtom, OrderSubscriberActor actor) {
            subscriber_ = actor;
          }};
}

void OrderAgent::HandleOpened(const OrderRtnData& order) {
  PositionData position;
  position.instrument = order.instrument;
  position.order_direction = order.order_direction;
  position.volume = order.volume;
  positions_.push_back(position);
}

void OrderAgent::HandleCancel(const OrderRtnData& order) {
}
