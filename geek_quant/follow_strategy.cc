#include "geek_quant/follow_strategy.h"
#include "follow_strategy.h"

FollowStrategy::FollowStrategy(caf::actor_config& cfg)
    : FollowTAStrategyActor::base(cfg) {}

FollowStrategy::~FollowStrategy() {}

FollowTAStrategyActor::behavior_type FollowStrategy::make_behavior() {
  return {
      [&](AddStrategySubscriberAtom, OrderSubscriberActor subscriber) {
        subscribers_.push_back(subscriber);
      },
      [=](TAPositionAtom, std::vector<PositionData> positions) {
        positions_ = positions;
      },
      [=](TAUnfillOrdersAtom, std::vector<OrderRtnData> orders) {
        unfill_orders_ = orders;
      },
      [=](TARtnOrderAtom, OrderRtnData order) {
        switch (order.order_status) {
          case OrderStatus::kOSInvalid:
            break;
          case OrderStatus::kOSOpening:
            HandleOpenging(order);
            break;
          case OrderStatus::kOSCloseing:
            HandleCloseing(order);
            break;
          case OrderStatus::kOSOpened:
            HandleOpened(order);
            break;
          case OrderStatus::kOSClosed:
            HandleClosed(order);
            break;
          case OrderStatus::kOSCancel:
            HandleCancel(order);
            break;
          default:
            break;
        }
      },
  };
}

void FollowStrategy::HandleOpenging(const OrderRtnData& order) {
  auto it_unfill = std::find_if(
      unfill_orders_.begin(), unfill_orders_.end(), [&](auto unfill_order) {
        return order.order_no == unfill_order.order_no;
      });
  if (it_unfill != unfill_orders_.end()) {
    return;
  }
  EnterOrderData enter_order;
  enter_order.instrument = order.instrument;
  enter_order.order_no = order.order_no;
  enter_order.order_direction = order.order_direction;
  enter_order.order_price = order.order_price;
  enter_order.volume = order.volume;

  auto it_direction_pos =
      std::find_if(positions_.begin(), positions_.end(), [&](auto position) {
        return position.instrument == order.instrument &&
               position.order_direction == order.order_direction;
      });

  auto it_reverse_pos =
      std::find_if(positions_.begin(), positions_.end(), [&](auto position) {
        return position.instrument == order.instrument &&
               position.order_direction != order.order_direction;
      });
  if (it_reverse_pos != positions_.end()) {
    enter_order.old_volume = it_reverse_pos->volume;
    enter_order.action = EnterOrderAction::kEOAOpenReverseOrder;
  } else {
    enter_order.old_volume =
        it_direction_pos != positions_.end() ? it_direction_pos->volume : 0;
    enter_order.action = EnterOrderAction::kEOAOpen;
  }
  unfill_orders_.push_back(order);
  for (auto subscriber : subscribers_) {
    send(subscriber, EnterOrderAtom::value, enter_order);
  }
}

void FollowStrategy::HandleCloseing(const OrderRtnData& order) {
  auto it_unfill = std::find_if(
      unfill_orders_.begin(), unfill_orders_.end(), [&](auto unfill_order) {
        return order.order_no == unfill_order.order_no;
      });
  if (it_unfill != unfill_orders_.end()) {
    return;
  }

  auto it_position =
      std::find_if(positions_.begin(), positions_.end(), [&](auto position) {
        return position.instrument == order.instrument &&
               position.order_direction != order.order_direction;
      });
  if (it_position == positions_.end()) {
    // TODO:ASSERT(FALSE);
    return;
  }

  unfill_orders_.push_back(order);

  EnterOrderData enter_order;
  enter_order.instrument = order.instrument;
  enter_order.order_no = order.order_no;
  enter_order.order_direction = order.order_direction;
  enter_order.order_price = order.order_price;
  enter_order.old_volume = it_position->volume;
  enter_order.volume = order.volume;
  enter_order.action = EnterOrderAction::kEOAClose;
  for (auto subscriber : subscribers_) {
    send(subscriber, EnterOrderAtom::value, enter_order);
  }
}

void FollowStrategy::HandleOpened(const OrderRtnData& order) {
  auto it_unfill = std::find_if(
      unfill_orders_.begin(), unfill_orders_.end(), [&](auto unfill_order) {
        return order.order_no == unfill_order.order_no;
      });
  if (it_unfill == unfill_orders_.end()) {
    // TODO:ASSERT(FALSE);
    return;
  }

  auto it_direction_pos =
      std::find_if(positions_.begin(), positions_.end(), [&](auto position) {
        return position.instrument == order.instrument &&
               position.order_direction == order.order_direction;
      });
  auto it_reverse_pos =
      std::find_if(positions_.begin(), positions_.end(), [&](auto position) {
        return position.instrument == order.instrument &&
               position.order_direction != order.order_direction;
      });

  EnterOrderData enter_order;
  enter_order.instrument = order.instrument;
  enter_order.order_no = order.order_no;
  enter_order.order_direction = order.order_direction;
  enter_order.order_price = order.order_price;
  enter_order.volume = order.volume;

  if (it_direction_pos != positions_.end() &&
      it_reverse_pos == positions_.end()) {
    enter_order.old_volume = it_direction_pos->volume;
    it_direction_pos->volume += order.volume;
    enter_order.action = EnterOrderAction::kEOAOpenConfirm;
  } else if (it_direction_pos == positions_.end() &&
             it_reverse_pos != positions_.end()) {
    enter_order.old_volume = it_reverse_pos->volume;
    enter_order.action = EnterOrderAction::kEOAOpenReverseOrderConfirm;

    PositionData position;
    position.instrument = order.instrument;
    position.order_direction = order.order_direction;
    position.volume = order.volume;
    positions_.push_back(position);
  } else if (it_direction_pos != positions_.end() &&
             it_reverse_pos != positions_.end()) {
    enter_order.old_volume = it_reverse_pos->volume;
    enter_order.action = EnterOrderAction::kEOAOpenReverseOrderConfirm;
    it_direction_pos->volume += order.volume;
  } else {
    enter_order.old_volume = 0;
    enter_order.action = EnterOrderAction::kEOAOpenConfirm;
    PositionData position;
    position.instrument = order.instrument;
    position.order_direction = order.order_direction;
    position.volume = order.volume;
    positions_.push_back(position);
  }

  for (auto subscriber : subscribers_) {
    send(subscriber, EnterOrderAtom::value, enter_order);
  }
  unfill_orders_.erase(it_unfill);
}

void FollowStrategy::HandleClosed(const OrderRtnData& order) {
  auto it_unfill = std::find_if(
      unfill_orders_.begin(), unfill_orders_.end(), [&](auto unfill_order) {
        return order.order_no == unfill_order.order_no;
      });
  if (it_unfill == unfill_orders_.end()) {
    // TODO:ASSERT(FALSE);
    return;
  }

  auto it_position =
      std::find_if(positions_.begin(), positions_.end(), [&](auto position) {
        return position.instrument == order.instrument &&
               position.order_direction != order.order_direction;
      });
  if (it_position == positions_.end()) {
    // TODO:ASSERT(FALSE);
    return;
  }

  int old_volume = it_position->volume;
  it_position->volume -= order.volume;
  // TODO:ASSERT(it_position->volume >= 0)
  if (it_position->volume <= 0) {
    positions_.erase(it_position);
  }

  EnterOrderData enter_order;
  enter_order.instrument = order.instrument;
  enter_order.order_no = order.order_no;
  enter_order.order_direction = order.order_direction;
  enter_order.order_price = order.order_price;
  enter_order.old_volume = old_volume;
  enter_order.volume = order.volume;
  enter_order.action = EnterOrderAction::kEOACloseConfirm;
  for (auto subscriber : subscribers_) {
    send(subscriber, EnterOrderAtom::value, enter_order);
  }
}

void FollowStrategy::HandleCancel(const OrderRtnData& order) {
  auto it_unfill = std::find_if(
      unfill_orders_.begin(), unfill_orders_.end(), [&](auto unfill_order) {
        return order.order_no == unfill_order.order_no;
      });
  if (it_unfill == unfill_orders_.end()) {
    // TODO:ASSERT(FALSE);
    return;
  }
  for (auto subscriber : subscribers_) {
    send(subscriber, CancelOrderAtom::value, order.order_no);
  }
}
