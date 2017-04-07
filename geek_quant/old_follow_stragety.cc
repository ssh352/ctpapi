#include "old_follow_stragety.h"

OldFollowStragety::OldFollowStragety(bool wait_sync) {
  wait_sync_ = wait_sync;
}

bool OldFollowStragety::WaitSyncOrders() {
  return wait_sync_;
}

void OldFollowStragety::SyncComplete() {
  wait_sync_ = false;
}

void OldFollowStragety::AddPositionToTrader(const std::string& order_no,
                                         OrderDirection order_direction,
                                         int volume) {
  trader_orders_.AddPosition(order_no, order_direction, volume);
}

void OldFollowStragety::AddPositionToFollower(const std::string& order_no,
                                           OrderDirection order_direction,
                                           int volume) {
  follower_orders_.AddPosition(order_no, order_direction, volume);
}

void OldFollowStragety::HandleOrderRtnForTrader(
    const RtnOrderData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {
  if (WaitSyncOrders()) {
    pending_order_actions_[order.order_no].HandleOrderRtnForTrader(
        order, cancel_order_no_list);
    trader_orders_.HandleOrderRtn(order);
    return;
  }

  if (pending_order_actions_.find(order.order_no) !=
      pending_order_actions_.end()) {
    pending_order_actions_[order.order_no].HandleOrderRtnForTrader(
        order, cancel_order_no_list);
  }

  switch (order.order_status) {
    case OldOrderStatus::kOpening: {
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
    case OldOrderStatus::kCloseing: {
      CloseingActionInfo action = trader_orders_.ParseCloseingOrderRtn(order);
      std::vector<std::string> order_list;
      for (auto item : action.items) {
        if (pending_order_actions_.find(item.first) !=
            pending_order_actions_.end()) {
          pending_order_actions_[item.first].HandleCloseingFromTrader(
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

void OldFollowStragety::HandleOrderRtnForFollow(
    const RtnOrderData& order,
    EnterOrderData* enter_order,
    std::vector<std::string>* cancel_order_no_list) {
  if (WaitSyncOrders()) {
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

EnterOrderData OldFollowStragety::MakeOpenReverseAction(
    const RtnOrderData& order,
    std::vector<std::pair<std::string, int> > order_volumes) {
  EnterOrderData enter_order;
  int volume =
      std::accumulate(order_volumes.begin(), order_volumes.end(), 0,
                      [](int val, auto item) { return val + item.second; });
  if (volume <= 0) {
    return enter_order;
  }
  enter_order.order_no = order.order_no;
  enter_order.action = EnterOrderAction::kOpen;
  enter_order.instrument = order.instrument;
  enter_order.order_direction = order.order_direction;
  enter_order.order_price = order.order_price;
  enter_order.volume = std::min<int>(order.volume, volume);
  pending_order_actions_[order.order_no] =
      PendingOrderAction{order, enter_order.volume};
  return enter_order;
}

EnterOrderData OldFollowStragety::MakeOpeningAction(const RtnOrderData& order) {
  EnterOrderData enter_order;
  enter_order.order_no = order.order_no;
  enter_order.action = EnterOrderAction::kOpen;
  enter_order.instrument = order.instrument;
  enter_order.order_direction = order.order_direction;
  enter_order.order_price = order.order_price;
  enter_order.volume = order.volume;
  pending_order_actions_[order.order_no] =
      PendingOrderAction{order, order.volume};
  return enter_order;
}

EnterOrderData OldFollowStragety::MakeCloseingAction(
    const RtnOrderData& order,
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
  enter_order.action = EnterOrderAction::kClose;
  enter_order.instrument = order.instrument;
  enter_order.order_direction = order.order_direction;
  enter_order.order_price = order.order_price;
  enter_order.volume = close_volume;
  enter_order.today = order.today;
  pending_order_actions_[order.order_no] =
      PendingOrderAction{order, enter_order.volume};
  return enter_order;
}

int OldFollowStragety::GetPendingCloseVolume(
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
