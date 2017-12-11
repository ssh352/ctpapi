#ifndef FOLLOW_STRATEGY_CTA_TRADED_STRATEGY_H
#define FOLLOW_STRATEGY_CTA_TRADED_STRATEGY_H
#include "common/api_struct.h"
#include "hpt_core/order_util.h"
#include "bft_core/channel_delegate.h"
#include "caf_common/caf_atom_defines.h"
class CTATradedStrategy {
 public:
  CTATradedStrategy(bft::ChannelDelegate* mail_box);

  void HandleTick(const std::shared_ptr<TickData>& tick);

  void HandleCTASignal(const std::shared_ptr<OrderField>& order,
                       const CTAPositionQty&);

  void HandleCTASignalEx(CTASignalAtom,
                         const std::shared_ptr<OrderField>& order);

 private:
  std::string GenerateOrderId();

  std::shared_ptr<Tick> last_tick_;
  int order_id_seq_ = 0;
  bft::ChannelDelegate* mail_box_;
};

#endif  // FOLLOW_STRATEGY_CTA_TRADED_STRATEGY_H
