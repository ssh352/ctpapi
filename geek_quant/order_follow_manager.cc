#include "order_follow_manager.h"

void OrderFollowMananger::AddPosition(const std::string& order_no,
                                      OrderDirection order_direction,
                                      int volume) {
  if (orders_.find(order_no) == orders_.end()) {
    orders_[order_no].MakePosition(volume, order_direction);
  }
}

void OrderFollowMananger::HandleOrderRtn(const OrderRtnData& order) {
  switch (order.order_status) {
    case kOSOpening: {
      orders_[order.order_no].MakeOpening(order.volume, order.order_direction);
    } break;
    case kOSCloseing: {
      (void)HandleCloseing(order);
    } break;
    case kOSOpened: {
      if (orders_.find(order.order_no) != orders_.end()) {
        orders_[order.order_no].HandleOpened(order.volume);
      } else {
        // ASSERT(FALSE)
      }
    } break;
    case kOSClosed: {
      int outstanding_volume = order.volume;
      for (auto& item : closeing_orders_[order.order_no]) {
        int close_volume = std::min<int>(outstanding_volume, item.second);
        orders_[item.first].HandleClosed(close_volume);
        item.second -= close_volume;
        outstanding_volume -= close_volume;
        if (outstanding_volume <= 0) {
          break;
        }
      }
    } break;
    case kOSOpenCanceled: {
      orders_[order.order_no].HandleCanceledByOpen();
    } break;
    case kOSCloseCanceled: {
      for (auto item : closeing_orders_[order.order_no]) {
        orders_[item.first].HandleCanceledByClose();
      }
      closeing_orders_.erase(order.order_no);
    } break;
    default:
      break;
  }
}

OpenReverseOrderActionInfo OrderFollowMananger::ParseOpenReverseOrderRtn(
    const OrderRtnData& order) const {
  OpenReverseOrderActionInfo action;
  action.order_no = order.order_no;
  std::vector<std::pair<std::string, int> > direction_order_volumes;
  std::for_each(orders_.begin(), orders_.end(), [&](auto item) {
    if (item.second.order_direction() != order.order_direction) {
      direction_order_volumes.push_back(
          {item.first, item.second.total_volume()});
    }
  });

  int reversed_volumes = std::accumulate(
      orders_.begin(), orders_.end(), 0, [=](int val, auto item) {
        if (item.second.order_direction() == order.order_direction) {
          return val + item.second.total_volume();
        }
        return val;
      });

  // Process exists reverse volumes
  for (auto& item : direction_order_volumes) {
    if (reversed_volumes <= 0) {
      break;
    }
    int volume = std::min<int>(reversed_volumes, item.second);
    item.second -= volume;
    reversed_volumes -= volume;
  }

  int outstanding_voume = order.volume;
  for (auto item : direction_order_volumes) {
    if (item.second <= 0) {
      continue;
    }
    int volume = std::min<int>(outstanding_voume, item.second);
    action.items.push_back(
        OpenReverseOrderItem{item.first, item.second, volume});
    outstanding_voume -= volume;
    if (outstanding_voume <= 0) {
      break;
    }
  }

  return action;
}

CloseingActionInfo OrderFollowMananger::ParseCloseingOrderRtn(
    const OrderRtnData& order) const {
  CloseingActionInfo action_info;
  int outstanding_close_volume = order.volume;
  int close_volume = 0;
  for (auto item : orders_) {
    OrderFollow& order_follow = item.second;
    if (order_follow.order_direction() == order.order_direction) {
      continue;
    }

    int old_position_volume = order_follow.position_volume();
    int close_volume =
        std::min<int>(outstanding_close_volume, order_follow.position_volume());

    action_info.order_no = order.order_no;
    action_info.items[item.first] =
        CloseingActionItem{close_volume, order_follow.position_volume()};
    outstanding_close_volume -= close_volume;
    if (outstanding_close_volume <= 0) {
      break;
    }
  }
  return action_info;
}

void OrderFollowMananger::HandleCloseing(const OrderRtnData& order) {
  int outstanding_close_volume = order.volume;
  int close_volume = 0;
  for (auto& item : orders_) {
    OrderFollow& order_follow = item.second;
    if (order_follow.order_direction() == order.order_direction) {
      continue;
    }

    int old_position_volume = order_follow.position_volume();
    int close_volume =
        std::min<int>(outstanding_close_volume, order_follow.position_volume());

    order_follow.HandleCloseing(close_volume);
    closeing_orders_[order.order_no].push_back(
        std::make_pair(item.first, close_volume));
    outstanding_close_volume -= close_volume;
    if (outstanding_close_volume <= 0) {
      break;
    }
  }
}

bool OrderFollowMananger::IsOpenReverseOrder(const OrderRtnData& order) const {
  int buy_position_volume = std::accumulate(
      orders_.begin(), orders_.end(), 0, [](int val, auto item) {
        if (item.second.order_direction() == kODBuy) {
          return val + item.second.position_volume();
        }
        return val;
      });

  int sell_position_volume = std::accumulate(
      orders_.begin(), orders_.end(), 0, [](int val, auto item) {
        if (item.second.order_direction() == kODSell) {
          return val + item.second.position_volume();
        }
        return val;
      });

  OrderDirection order_direction = kODUnkown;
  if (buy_position_volume > sell_position_volume) {
    order_direction = kODBuy;
  } else if (buy_position_volume > sell_position_volume) {
    order_direction = kODSell;
  } else {
    // Do noting
  }
  return order_direction != kODUnkown ? order_direction != order.order_direction
                                      : false;
}

std::vector<std::pair<std::string, int> >
OrderFollowMananger::GetPositionVolumeWithOrderNoList(
    const std::vector<std::string>& order_list) const {
  std::vector<std::pair<std::string, int> > order_position_list;
  for (auto order_no : order_list) {
    if (orders_.find(order_no) != orders_.end()) {
      order_position_list.push_back(
          std::make_pair(order_no, orders_[order_no].position_volume()));
    }
  }
  return order_position_list;
}
