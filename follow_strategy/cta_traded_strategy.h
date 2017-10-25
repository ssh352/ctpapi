#ifndef FOLLOW_STRATEGY_CTA_TRADED_STRATEGY_H
#define FOLLOW_STRATEGY_CTA_TRADED_STRATEGY_H
#include "common/api_struct.h"
#include "order_util.h"

template <class MailBox>
class CTATradedStrategy {
 public:
  CTATradedStrategy(MailBox* mail_box) : mail_box_(mail_box) {
    mail_box_->Subscribe(&CTATradedStrategy::HandleCTASignal, this);
    mail_box_->Subscribe(&CTATradedStrategy::HandleTick, this);
  }

  void HandleTick(const std::shared_ptr<TickData>& tick) {
    last_tick_ = tick->tick;
  }

  void HandleCTASignal(const CTASignalAtom&, const std::shared_ptr<OrderField>& order) {
    if (order->status == OrderStatus::kActive) {
      mail_box_->Send(InputOrderSignal{
          order->instrument_id, GenerateOrderId(), "cta",
          order->position_effect, order->direction,
          order->input_price, order->qty, last_tick_->timestamp});
    }
  }

 private:
  std::string GenerateOrderId() { return boost::lexical_cast<std::string>(order_id_seq_++); }

  std::shared_ptr<Tick> last_tick_;
  int order_id_seq_ = 0;
  MailBox* mail_box_;
};

#endif  // FOLLOW_STRATEGY_CTA_TRADED_STRATEGY_H
