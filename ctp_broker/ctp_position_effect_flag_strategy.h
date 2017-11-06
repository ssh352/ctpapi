#ifndef CTP_BROKER_CTP_POSITION_EFFECT_FLAG_STRATEGY_H
#define CTP_BROKER_CTP_POSITION_EFFECT_FLAG_STRATEGY_H
#include <string>
#include "common/api_data_type.h"
#include "ctp_position_amount.h"

class CTPPositionEffectStrategyDelegate {
 public:
  virtual void PosstionEffectStrategyHandleInputOrder(
      const std::string& input_order_id,
      CTPPositionEffect position_effect,
      OrderDirection direction,
      double price,
      int qty) = 0;
};

class CTPPositionEffectFlagStrategy {
 public:
  CTPPositionEffectFlagStrategy(CTPPositionEffectStrategyDelegate* delegate)
      : delegate_(delegate) {}

  virtual void HandleInputOrder(const std::string& input_order_id,
                                PositionEffect position_effect,
                                OrderDirection direction,
                                double price,
                                int qty,
                                const CTPPositionAmount& position_amount) = 0;

 protected:
  CTPPositionEffectStrategyDelegate* delegate_;
};

class GenericCTPPositionEffectFlagStrategy
    : public CTPPositionEffectFlagStrategy {
 public:
  GenericCTPPositionEffectFlagStrategy(
      CTPPositionEffectStrategyDelegate* delegate)
      : CTPPositionEffectFlagStrategy(delegate) {}
  virtual void HandleInputOrder(
      const std::string& input_order_id,
      PositionEffect position_effect,
      OrderDirection direction,
      double price,
      int qty,
      const CTPPositionAmount& position_amount) override;
};

class CloseTodayAwareCTPPositionEffectFlagStrategy
    : public CTPPositionEffectFlagStrategy {
 public:
  CloseTodayAwareCTPPositionEffectFlagStrategy(
      CTPPositionEffectStrategyDelegate* delegate)
      : CTPPositionEffectFlagStrategy(delegate) {}
  virtual void HandleInputOrder(
      const std::string& input_order_id,
      PositionEffect position_effect,
      OrderDirection direction,
      double price,
      int qty,
      const CTPPositionAmount& position_amount) override;
};

#endif  // CTP_BROKER_CTP_POSITION_EFFECT_FLAG_STRATEGY_H
