#ifndef CTP_BROKER_CTP_POSITION_EFFECT_STRATEGY_H
#define CTP_BROKER_CTP_POSITION_EFFECT_STRATEGY_H
#include "ctp_position_amount.h"
#include "common/api_struct.h"
#include "ctp_position_effect_flag_strategy.h"

class CTPPositionEffectStrategy {
 public:
  CTPPositionEffectStrategy(CTPPositionEffectFlagStrategy* strategy)
      : position_effect_flag_strategy_(strategy) {}

  virtual void HandleInputOrder(const InputOrder& order,
                                const CTPPositionAmount& long_amount,
                                const CTPPositionAmount& short_amount) = 0;

 protected:
  CTPPositionEffectFlagStrategy* position_effect_flag_strategy_;
};

class GenericCTPPositionEffectStrategy : public CTPPositionEffectStrategy {
 public:
  GenericCTPPositionEffectStrategy(CTPPositionEffectFlagStrategy* strategy);

  virtual void HandleInputOrder(const InputOrder& order,
                                const CTPPositionAmount& long_amount,
                                const CTPPositionAmount& short_amount) override;
};

class CloseTodayCostCTPPositionEffectStrategy
    : public CTPPositionEffectStrategy {
 public:
  CloseTodayCostCTPPositionEffectStrategy(
      CTPPositionEffectFlagStrategy* strategy);

  virtual void HandleInputOrder(const InputOrder& order,
                                const CTPPositionAmount& long_amount,
                                const CTPPositionAmount& short_amount) override;
};

#endif  // CTP_BROKER_CTP_POSITION_EFFECT_STRATEGY_H
