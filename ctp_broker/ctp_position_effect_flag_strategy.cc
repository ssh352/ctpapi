#include "ctp_position_effect_flag_strategy.h"
#include <boost/assert.hpp>

// GenericCTPPositionEffectFlagStrategy
void GenericCTPPositionEffectFlagStrategy::HandleInputOrder(
    const std::string& input_order_id,
    PositionEffect position_effect,
    OrderDirection direction,
    double price,
    int qty,
    const CTPPositionAmount& /*position_amount*/) {
  delegate_->PosstionEffectStrategyHandleInputOrder(
      input_order_id,
      (position_effect == PositionEffect::kOpen ? CTPPositionEffect::kOpen
                                                : CTPPositionEffect::kClose),
      direction, price, qty);
}

// CloseTodayAwareCTPPositionEffectFlagStrategy
void CloseTodayAwareCTPPositionEffectFlagStrategy::HandleInputOrder(
    const std::string& input_order_id,
    PositionEffect position_effect,
    OrderDirection direction,
    double price,
    int qty,
    const CTPPositionAmount& position_amount) {
  if (position_effect == PositionEffect::kOpen) {
    delegate_->PosstionEffectStrategyHandleInputOrder(
        input_order_id, CTPPositionEffect::kOpen, direction, price, qty);
  } else {
    BOOST_ASSERT(position_amount.Closeable() >= qty);
    int yesterday = std::min<int>(qty, position_amount.YesterdayCloseable());
    if (yesterday > 0) {
      delegate_->PosstionEffectStrategyHandleInputOrder(
          input_order_id, CTPPositionEffect::kClose, direction, price, yesterday);
    }
    int leaves_closeable = qty - yesterday;
    if (leaves_closeable > 0) {
      delegate_->PosstionEffectStrategyHandleInputOrder(
          input_order_id, CTPPositionEffect::kCloseToday, direction, price,
          leaves_closeable);
    }
  }
}
