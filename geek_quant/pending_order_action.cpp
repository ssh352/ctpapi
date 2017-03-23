#include "pending_order_action.h"

PendingOrderAction::PendingOrderAction(const OrderRtnData& order,
                                       int follower_volum)
    : order_no_(order.order_no),
      order_direction_(order.order_direction),
      trader_{0, order.volume, false},
      follower_{0, follower_volum, false} {
  if (order.order_status == kOSCloseing) {
    pending_close_volume_ = follower_volum;
  }
}

void PendingOrderAction::HandleOrderRtnForTrader(const OrderRtnData& order) {
  switch (order.order_status) {
    case kOSOpening:
      break;
    case kOSCloseing:
      break;
    case kOSOpened:
    case kOSClosed:
      trader_.traded_volume += order.volume;
      break;
    case kOSOpenCanceled:
    case kOSCloseCanceled:
      trader_.total_volume = trader_.traded_volume;
      trader_.cancel = true;
      break;
    default:
      break;
  }
}

bool PendingOrderAction::HandleOrderRtnForFollower(
    const OrderRtnData& order,
    std::vector<std::string>* cancel_order_no_list) {
  switch (order.order_status) {
    case kOSOpening: {
      if (trader_.cancel) {
        cancel_order_no_list->push_back(order_no_);
      }
    } break;
    case kOSCloseing:
      pending_close_volume_ = 0;
      break;
    case kOSOpened:
    case kOSClosed:
      follower_.traded_volume += order.volume;
      break;
    case kOSOpenCanceled: {
      case kOSCloseCanceled:
        follower_.total_volume = follower_.traded_volume;
        break;
      default:
        break;
    }
  }
  return trader_.total_volume == trader_.traded_volume &&
         follower_.total_volume == follower_.traded_volume;
}

void PendingOrderAction::HandleCloseing(
    std::vector<std::string>* cancel_order_no_list) {
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
