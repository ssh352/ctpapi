#ifndef HPT_CORE_SIMULATED_ALWAYS_TREADE_EXECUTION_HANDLER_H
#define HPT_CORE_SIMULATED_ALWAYS_TREADE_EXECUTION_HANDLER_H
#include <boost/assert.hpp>
#include "common/api_struct.h"
#include "bft_core/channel_delegate.h"
#include "caf_common/caf_atom_defines.h"

class SimulatedAlwaysExcutionHandler {
 public:
  SimulatedAlwaysExcutionHandler(bft::ChannelDelegate* mail_box);

  void BeforeTrading( BeforeTradingAtom,
                     const TradingTime& trading_time);

  void HandleTick(const std::shared_ptr<TickData>& tick);

  void HandlerInputOrder(const InputOrder& input_order);

  void HandleCancelOrder(const CancelOrder& cancel_order);

 private:
  void EnqueueRtnOrderEvent(const std::string& order_id,
                            const std::string& instrument,
                            PositionEffect position_effect,
                            OrderDirection direction,
                            double price,
                            int qty,
                            OrderStatus order_status);
  bft::ChannelDelegate* mail_box_;
  std::shared_ptr<Tick> current_tick_;
};

#endif  // HPT_CORE_SIMULATED_ALWAYS_TREADE_EXECUTION_HANDLER_H
