#include "ctp_position_effect_strategy.h"
#include <algorithm>
#include <boost/assert.hpp>
#include "hpt_core/order_util.h"

// GenericCTPPositionEffectStrategy
GenericCTPPositionEffectStrategy::GenericCTPPositionEffectStrategy(
    CTPPositionEffectFlagStrategy* strategy)
    : CTPPositionEffectStrategy(strategy) {}

void GenericCTPPositionEffectStrategy::HandleInputOrder(
    const InputOrder& order,
    const CTPPositionAmount& long_amount,
    const CTPPositionAmount& short_amount) {
  if (order.position_effect == PositionEffect::kOpen) {
    OrderDirection opposite_direction = order.direction == OrderDirection::kBuy
                                            ? OrderDirection::kSell
                                            : OrderDirection::kBuy;
    const CTPPositionAmount& position =
        (order.direction == OrderDirection::kBuy ? long_amount : short_amount);
    const CTPPositionAmount& opposite_position =
        (opposite_direction == OrderDirection::kBuy ? long_amount
                                                    : short_amount);

    if (opposite_position.Closeable() > 0) {
      int close_qty = std::min<int>(opposite_position.Closeable(), order.qty);
      position_effect_flag_strategy_->HandleInputOrder(
          order.order_id, PositionEffect::kClose, opposite_direction,
          order.price, close_qty, opposite_position);
      int leaves_open_qty = order.qty - close_qty;
      if (leaves_open_qty > 0) {
        position_effect_flag_strategy_->HandleInputOrder(
            order.order_id, PositionEffect::kOpen, order.direction, order.price,
            leaves_open_qty, position);
      }
    } else {
      position_effect_flag_strategy_->HandleInputOrder(
          order.order_id, PositionEffect::kOpen, order.direction, order.price,
          order.qty, position);
    }
  } else {
    int closeable =
        (OppositeOrderDirection(order.direction) == OrderDirection::kBuy
             ? long_amount.Closeable()
             : short_amount.Closeable());
    BOOST_ASSERT(closeable >= order.qty);
    // TODO: If assert is fail, maybe throw exception
    position_effect_flag_strategy_->HandleInputOrder(
        order.order_id, order.position_effect, order.direction, order.price,
        order.qty,
        (OppositeOrderDirection(order.direction) == OrderDirection::kBuy
             ? long_amount
             : short_amount));
  }
}

// CloseTodayCostCTPPositionEffectStrategy
CloseTodayCostCTPPositionEffectStrategy::
    CloseTodayCostCTPPositionEffectStrategy(
        CTPPositionEffectFlagStrategy* strategy)
    : CTPPositionEffectStrategy(strategy) {}

void CloseTodayCostCTPPositionEffectStrategy::HandleInputOrder(
    const InputOrder& order,
    const CTPPositionAmount& long_amount,
    const CTPPositionAmount& short_amount) {
  if (order.position_effect == PositionEffect::kClose) {
    OrderDirection opposite_direction = order.direction;
    const CTPPositionAmount& position =
        (OppositeOrderDirection(order.direction) == OrderDirection::kBuy
             ? long_amount
             : short_amount);

    const CTPPositionAmount& opposite_position =
        (opposite_direction == OrderDirection::kBuy ? long_amount
                                                    : short_amount);
    int yesterday = std::min<int>(position.YesterdayCloseable(), order.qty);
    if (yesterday > 0) {
      // Always Close yesterday position
      position_effect_flag_strategy_->HandleInputOrder(
          order.order_id, PositionEffect::kClose, order.direction, order.price,
          yesterday, position);
    }
    int leaves_close = order.qty - yesterday;
    if (leaves_close > 0) {
      // Lock open
      position_effect_flag_strategy_->HandleInputOrder(
          order.order_id, PositionEffect::kOpen, opposite_direction,
          order.price, leaves_close, opposite_position);
    }
  } else {
    OrderDirection opposite_direction = order.direction == OrderDirection::kBuy
                                            ? OrderDirection::kSell
                                            : OrderDirection::kBuy;
    const CTPPositionAmount& position =
        (order.direction == OrderDirection::kBuy ? long_amount : short_amount);
    const CTPPositionAmount& opposite_position =
        (opposite_direction == OrderDirection::kBuy ? long_amount
                                                    : short_amount);
    int yesterday =
        std::min<int>(opposite_position.YesterdayCloseable(), order.qty);

    if (yesterday > 0) {
      position_effect_flag_strategy_->HandleInputOrder(
          order.order_id, PositionEffect::kClose, opposite_direction,
          order.price, yesterday, opposite_position);
    }

    int leaves_open_qty = order.qty - yesterday;
    if (leaves_open_qty > 0) {
      position_effect_flag_strategy_->HandleInputOrder(
          order.order_id, PositionEffect::kOpen, order.direction, order.price,
          leaves_open_qty, position);
    }
  }
}
