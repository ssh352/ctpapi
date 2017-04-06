#include "pending_order_action.h"

PendingOrderAction::PendingOrderAction(const OrderRtnData& order,
                                       int follower_volum)
    : order_no_(order.order_no),
      order_direction_(order.order_direction),
      trader_{0, order.volume, false, false},
      follower_{0, 0, false, false} {
  pending_close_volume_ = 0;
  if (order.order_status == OrderStatus::kCloseing) {
    pending_close_volume_ = follower_volum;
  }
}

PendingOrderAction::PendingOrderAction()
    : trader_{0, 0, false, false}, follower_{0, 0, false, false} {
  order_direction_ = OrderDirection::kUnkown;
  pending_close_volume_ = 0;
}

void PendingOrderAction::HandleOrderRtnForTrader(
    const OrderRtnData& order,
    std::vector<std::string>* cancel_order_no_list) {
  switch (order.order_status) {
    case OrderStatus::kOpening:
    case OrderStatus::kCloseing: {
      order_no_ = order.order_no;
      order_direction_ = order.order_direction;
      trader_.traded_volume = 0;
      trader_.total_volume = order.volume;
    } break;
    case OrderStatus::kOpened:
    case OrderStatus::kClosed:
      trader_.traded_volume += order.volume;
      break;
    case OrderStatus::kOpenCanceled:
    case OrderStatus::kCloseCanceled:
      trader_.total_volume = trader_.traded_volume;
      trader_.cancel = true;
      if (follower_.total_volume != follower_.traded_volume &&
          !follower_.cancel) {
        cancel_order_no_list->push_back(order_no_);
        follower_.cancel = true;
      }
      break;
    default:
      break;
  }
}

bool PendingOrderAction::HandleOrderRtnForFollower(
    const OrderRtnData& order,
    std::vector<std::string>* cancel_order_no_list) {
  switch (order.order_status) {
    case OrderStatus::kOpening: {
      follower_.traded_volume = 0;
      follower_.total_volume = order.volume;
      if (trader_.cancel || trader_.closeing) {
        cancel_order_no_list->push_back(order_no_);
      }
    } break;
    case OrderStatus::kCloseing:
      follower_.traded_volume = 0;
      follower_.total_volume = order.volume;
      pending_close_volume_ = 0;
      break;
    case OrderStatus::kOpened:
    case OrderStatus::kClosed:
      follower_.traded_volume += order.volume;
      break;
    case OrderStatus::kOpenCanceled: {
      case OrderStatus::kCloseCanceled:
        follower_.total_volume = follower_.traded_volume;
        break;
      default:
        break;
    }
  }
  return trader_.total_volume == trader_.traded_volume &&
         follower_.total_volume == follower_.traded_volume;
}

void PendingOrderAction::HandleCloseingFromTrader(
    std::vector<std::string>* cancel_order_no_list) {
  trader_.closeing = true;
  if (follower_.total_volume != follower_.traded_volume && !follower_.cancel) {
    cancel_order_no_list->push_back(order_no_);
    follower_.cancel = true;
  }
}

void PendingOrderAction::HandleOpenReverse(
    std::vector<std::string>* cancel_order_no_list) {
  if (follower_.total_volume != follower_.traded_volume && !follower_.cancel) {
    cancel_order_no_list->push_back(order_no_);
    follower_.cancel = true;
  }
}
