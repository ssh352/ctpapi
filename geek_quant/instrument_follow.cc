#include "instrument_follow.h"

InstrumentFollow::InstrumentFollow() {
  has_sync_ = false;
  trader_order_rtn_seq_ = 0;
  follower_order_rtn_seq_ = 0;
  last_check_trader_order_rtn_seq_ = 0;
  last_check_follower_order_rtn_seq_ = 0;
}

bool InstrumentFollow::HasSyncOrders() {
  return has_sync_;
}

bool InstrumentFollow::TryCompleteSyncOrders() {
  if (trader_order_rtn_seq_ != last_check_trader_order_rtn_seq_ ||
      follower_order_rtn_seq_ != last_check_follower_order_rtn_seq_) {
    last_check_trader_order_rtn_seq_ = trader_order_rtn_seq_;
    last_check_follower_order_rtn_seq_ = follower_order_rtn_seq_;
    return false;
  }

  has_sync_ = true;
  return true;
}

void InstrumentFollow::HandleOrderRtnForTrader(
    const OrderRtnData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {
  ++trader_order_rtn_seq_;
  if (!HasSyncOrders()) {
    pending_order_actions_[order.order_no].HandleOrderRtnForTrader(order);
    trader_orders_.HandleOrderRtn(order);
    return;
  }

  if (pending_order_actions_.find(order.order_no) !=
      pending_order_actions_.end()) {
    pending_order_actions_[order.order_no].HandleOrderRtnForTrader(order);
  }

  switch (order.order_status) {
    case kOSOpening: {
      if (trader_orders_.IsOpenReverseOrder(order)) {
        OpenReverseOrderActionInfo action =
            trader_orders_.ParseOpenReverseOrderRtn(order);
        std::vector<std::string> order_list;
        for (auto item : action.items) {
          if (pending_order_actions_.find(item.order_no) !=
              pending_order_actions_.end()) {
            pending_order_actions_[item.order_no].HandleOpenReverse(
                cancel_order_no_list);
          }
          order_list.push_back(item.order_no);
        }
        *enter_order = MakeOpenReverseAction(
            order,
            follower_orders_.GetPositionVolumeWithOrderNoList(order_list));
      } else {
        *enter_order = MakeOpeningAction(order);
      }
    } break;
    case kOSCloseing: {
      CloseingActionInfo action = trader_orders_.ParseCloseingOrderRtn(order);
      std::vector<std::string> order_list;
      for (auto item : action.items) {
        if (pending_order_actions_.find(item.first) !=
            pending_order_actions_.end()) {
          pending_order_actions_[item.first].HandleCloseing(
              cancel_order_no_list);
        }
        order_list.push_back(item.first);
      }
      *enter_order = MakeCloseingAction(
          order, action,
          follower_orders_.GetPositionVolumeWithOrderNoList(order_list));
    } break;
    default:
      break;
  }
  trader_orders_.HandleOrderRtn(order);
}

void InstrumentFollow::HandleOrderRtnForFollow(
    const OrderRtnData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {
  ++follower_order_rtn_seq_;
  if (!HasSyncOrders()) {
    std::vector<std::string> dummy_cancel_order_no_list;
    pending_order_actions_[order.order_no].HandleOrderRtnForFollower(
        order, &dummy_cancel_order_no_list);
    follower_orders_.HandleOrderRtn(order);
    return;
  }

  if (pending_order_actions_.find(order.order_no) !=
      pending_order_actions_.end()) {
    if (pending_order_actions_[order.order_no].HandleOrderRtnForFollower(
            order, cancel_order_no_list)) {
      pending_order_actions_.erase(order.order_no);
    }
  }

  follower_orders_.HandleOrderRtn(order);
}

EnterOrderData InstrumentFollow::MakeOpenReverseAction(
    const OrderRtnData& order,
    std::vector<std::pair<std::string, int> > order_volumes) {
  EnterOrderData enter_order;
  int volume =
      std::accumulate(order_volumes.begin(), order_volumes.end(), 0,
                      [](int val, auto item) { return val + item.second; });
  if (volume <= 0) {
    return enter_order;
  }
  enter_order.order_no = order.order_no;
  enter_order.action = kEOAOpen;
  enter_order.instrument = order.instrument;
  enter_order.order_direction = order.order_direction;
  enter_order.order_price = order.order_price;
  enter_order.volume = std::min<int>(order.volume, volume);
  pending_order_actions_[order.order_no] =
      PendingOrderAction{order, enter_order.volume};
  return enter_order;
}

EnterOrderData InstrumentFollow::MakeOpeningAction(const OrderRtnData& order) {
  EnterOrderData enter_order;
  enter_order.order_no = order.order_no;
  enter_order.action = kEOAOpen;
  enter_order.instrument = order.instrument;
  enter_order.order_direction = order.order_direction;
  enter_order.order_price = order.order_price;
  enter_order.volume = order.volume;
  pending_order_actions_[order.order_no] =
      PendingOrderAction{order, order.volume};
  return enter_order;
}

EnterOrderData InstrumentFollow::MakeCloseingAction(
    const OrderRtnData& order,
    const CloseingActionInfo& action,
    std::vector<std::pair<std::string, int> > order_volumes) {
  EnterOrderData enter_order;
  int volume =
      std::accumulate(order_volumes.begin(), order_volumes.end(), 0,
                      [](int val, auto item) { return val + item.second; });
  if (volume <= 0) {
    return enter_order;
  }

  int pending_close_volume = GetPendingCloseVolume(order.order_direction);

  int trader_close_volume = std::accumulate(
      action.items.begin(), action.items.end(), 0,
      [](int val, auto item) { return val + item.second.close_volume; });

  int trader_position_volume = std::accumulate(
      action.items.begin(), action.items.end(), 0,
      [](int val, auto item) { return val + item.second.position_volume; });

  // trader_position_volume
  int close_volume = volume - pending_close_volume -
                     (trader_position_volume - trader_close_volume);
  if (close_volume <= 0) {
    return enter_order;
  }

  enter_order.order_no = order.order_no;
  enter_order.action = kEOAClose;
  enter_order.instrument = order.instrument;
  enter_order.order_direction = order.order_direction;
  enter_order.order_price = order.order_price;
  enter_order.volume = close_volume;
  pending_order_actions_[order.order_no] =
      PendingOrderAction{order, enter_order.volume};
  return enter_order;
}

int InstrumentFollow::GetPendingCloseVolume(
    OrderDirection order_direction) const {
  return std::accumulate(
      pending_order_actions_.begin(), pending_order_actions_.end(), 0,
      [=](int val, auto item) {
        if (item.second.order_direction() == order_direction) {
          return val + item.second.pending_close_volume();
        }
        return val;
      });
  return 0;
}
