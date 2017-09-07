#ifndef STRATEGIES_KEY_INPUT_STRATEGY_H
#define STRATEGIES_KEY_INPUT_STRATEGY_H
#include "common//api_struct.h"

template <typename MailBox>
class KeyInputStrategy {
 public:
  KeyInputStrategy(MailBox* mail_box, std::string instrument)
      : mail_box_(mail_box), instrument_(std::move(instrument)) {
    mail_box_->Subscribe(&KeyInputStrategy::HandleKeyInput, this);
    mail_box_->Subscribe(&KeyInputStrategy::HandleTick, this);
  }

  void HandleTick(const std::shared_ptr<TickData>& tick) { last_tick_ = tick; }

  void HandleKeyInput(int qty) {
    if (last_tick_ == nullptr) {
      return;
    }

    mail_box_->Send(InputOrderSignal{
        instrument_, (qty > 0 ? PositionEffect::kOpen : PositionEffect::kClose),
        (qty > 0 ? OrderDirection::kBuy : OrderDirection::kSell),
        last_tick_->tick->last_price, std::abs(qty),
        last_tick_->tick->timestamp});
  }

 private:
  MailBox* mail_box_;
  std::string instrument_;
  std::shared_ptr<TickData> last_tick_;
};

#endif  // STRATEGIES_KEY_INPUT_STRATEGY_H
