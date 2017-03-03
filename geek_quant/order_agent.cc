#include "order_agent.h"

OrderAgentActor::behavior_type OrderAgent::make_behavior() {
  return {[=](TAPositionAtom, std::vector<PositionData> positions) {
            wait_for_until_receive_positions_ = true;
            positions_ = positions;
            TryProcessPendingEnterOrder();
          },
          [=](TAUnfillOrdersAtom, std::vector<OrderRtnData> orders) {
            pending_orders_ = orders;
            wait_for_until_receive_unfill_orders_ = true;
            TryProcessPendingEnterOrder();
          },
          [=](TARtnOrderAtom, OrderRtnData order) {
            switch (order.order_status) {
              case kOSOpening:
                break;
              case kOSCloseing:
                break;
              case kOSOpened:
                OnOrderOpened(order);
                break;
              case kOSClosed:
                OnOrderClosed(order);
                break;
              case kOSCancel:
                OnOrderCanceled(order);
                break;
              default:
                break;
            }
          },
          [=](EnterOrderAtom, EnterOrderData enter_order) {
            if (!ReadyToEnterOrder()) {
              pending_enter_orders_.push_back(enter_order);
              return;
            }
            HandleEnterOrder(enter_order);
          },
          [=](CancelOrderAtom, std::string order_no) {
            auto it = std::find_if(
                pending_orders_.begin(), pending_orders_.end(),
                [&](auto order) { return order_no == order.order_no; });
            if (it != pending_orders_.end()) {
              send(subscriber_, CancelOrderAtom::value, order_no);
              pending_orders_.erase(it);
            } else {
              // TODO 
            }
          },
          [=](AddStrategySubscriberAtom, OrderSubscriberActor actor) {
            subscriber_ = actor;
          }};
}

void OrderAgent::OnOrderOpened(const OrderRtnData& order) {
  auto it = std::find_if(pending_orders_.begin(), pending_orders_.end(),
                         [&](auto pending_order) {
                           return pending_order.order_no == order.order_no;
                         });

  // ASSERT(it != pending_orders_.end())
  if (it == pending_orders_.end()) {
    return;
  }
  pending_orders_.erase(it);
  PositionData position;
  position.instrument = order.instrument;
  position.order_direction = order.order_direction;
  position.volume = order.volume;
  positions_.push_back(position);
}

void OrderAgent::OnOrderCanceled(const OrderRtnData& order) {
  auto it = std::find_if(pending_orders_.begin(), pending_orders_.end(),
                         [&](auto pending_order) {
                           return pending_order.order_no == order.order_no;
                         });

  // ASSERT(it != pending_orders_.end())
  if (it != pending_orders_.end()) {
    pending_orders_.erase(it);
  }
}

void OrderAgent::TryProcessPendingEnterOrder() {
  if (!ReadyToEnterOrder()) {
    return;
  }
  std::for_each(
      pending_enter_orders_.begin(), pending_enter_orders_.end(),
      [=](auto enter_order) {
        auto it = std::find_if(
            pending_orders_.begin(), pending_orders_.end(),
            [&](auto order) { return order.order_no == enter_order.order_no; });
        if (it == pending_orders_.end()) {
          HandleEnterOrder(enter_order);
        }
      });
  pending_enter_orders_.clear();
}

bool OrderAgent::ReadyToEnterOrder() const {
  return wait_for_until_receive_unfill_orders_ &&
         wait_for_until_receive_positions_;
}

void OrderAgent::OnOrderClosed(const OrderRtnData& order) {
  auto it = std::find_if(pending_orders_.begin(), pending_orders_.end(),
                         [&](auto pending_order) {
                           return pending_order.order_no == order.order_no;
                         });
  if (it != pending_orders_.end()) {
    auto it_pos =
        std::find_if(positions_.begin(), positions_.end(), [&](auto pos) {
          return pos.instrument == order.instrument &&
                 pos.order_direction != order.order_direction;
        });
    if (it_pos != positions_.end()) {
      it_pos->volume -= order.volume;
      // ASSERT(it_pos->volume>=0)
      if (it_pos->volume <= 0) {
        positions_.erase(it_pos);
      }
    } else {
      // TODO:ASSERT
    }
    pending_orders_.erase(it);
  } else {
    // TODO: ASSERT
  }
}

void OrderAgent::HandleEnterOrder(const EnterOrderData& enter_order) {
  switch (enter_order.action) {
    case kEOAOpen: {
      // ASSERT(enter_order.order_no not in pending_orders_);
      send(subscriber_, EnterOrderAtom::value, enter_order);
      OrderRtnData order;
      order.instrument = enter_order.instrument;
      order.order_no = enter_order.order_no;
      order.order_status = OrderStatus::kOSOpening;
      order.order_direction = enter_order.order_direction;
      order.order_price = enter_order.order_price;
      order.volume = enter_order.volume;
      pending_orders_.push_back(order);
    } break;
    case kEOAClose: {
      // ASSERT(not in pending orders)
      auto it_pos = std::find_if(
          positions_.begin(), positions_.end(), [&](auto position) {
            return enter_order.instrument == position.instrument &&
                   enter_order.order_direction != position.order_direction;
          });
      if (it_pos != positions_.end()) {
        EnterOrderData real_enter_order(enter_order);
        real_enter_order.volume = it_pos->volume;
        send(subscriber_, EnterOrderAtom::value, real_enter_order);
        OrderRtnData order;
        order.instrument = enter_order.instrument;
        order.order_no = enter_order.order_no;
        order.order_status = OrderStatus::kOSCloseing;
        order.order_direction = enter_order.order_direction;
        order.order_price = enter_order.order_price;
        order.volume = enter_order.volume;
        pending_orders_.push_back(order);
      }
    } break;
    case kEOAOpenReverseOrder: {
      auto it_pos = std::find_if(
          positions_.begin(), positions_.end(), [&](auto position) {
            return enter_order.instrument == position.instrument &&
                   enter_order.order_direction != position.order_direction;
          });
      if (it_pos != positions_.end()) {
        if (enter_order.old_volume == it_pos->volume) {
          send(subscriber_, EnterOrderAtom::value, enter_order);
        } else {
          EnterOrderData real_enter_order(enter_order);
          real_enter_order.volume = it_pos->volume;
          send(subscriber_, EnterOrderAtom::value, real_enter_order);
        }
      }
    } break;
    case kEOAOpenConfirm:
    case kEOACloseConfirm:
    case kEOAOpenReverseOrderConfirm:
    case kEOACancelForTest:
    default:
      break;
  }
}
