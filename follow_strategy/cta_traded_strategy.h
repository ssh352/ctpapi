#ifndef FOLLOW_STRATEGY_CTA_TRADED_STRATEGY_H
#define FOLLOW_STRATEGY_CTA_TRADED_STRATEGY_H
#include "common/api_struct.h"
#include "order_util.h"

template <class MailBox>
class CTATradedStrategy {
 public:
  CTATradedStrategy(MailBox* mail_box,
                    AbstractCTATradedStrategyDelegate* delegate)
      : mail_box_(mail_box) {
    mail_box_->Subscribe(&CTATradedStrategy::HandleCTASignal, this);
  }

  void HandleTick(const std::shared_ptr<TickData>& tick) {
    auto it_end = std::find_if(
        delayed_open_queue_.begin(), delayed_open_queue_.end(),
        [=](const auto& item) {
          return tick->tick->timestamp >
                 item.time_stamp + delayed_open_order_after_seconds_
        });
    for (auto it = delayed_open_queue_.begin(); it != it_end; ++it) {
      mail_box_->Send(InputOrderSignal{it->instrument, GenerateOrderId(),
                                       "dealyed_open", PositionEffect::kOpen,
                                       it->direction, it->price, it->qty,
                                       tick->tick->timestamp});
    }

    delayed_open_queue_.erase(delayed_open_queue_.begin(), it_end);
  }

  void HandleCTASignal(const std::shared_ptr<CTAOrderSignalField>& cta_signal) {
    if (cta_signal->order_status == OrderStatus::kCanceled) {
      HandleCanceled(cta_signal);
    } else if (IsOpenOrder(cta_signal->position_effect)) {
      if (cta_signal->order_status == OrderStatus::kActive &&
          cta_signal->trading_qty == 0) {
        HandleOpening(cta_signal);
      } else if ((cta_signal->order_status == OrderStatus::kActive ||
                  cta_signal->order_status == OrderStatus::kAllFilled) &&
                 cta_signal->trading_qty != 0) {
        HandleOpened(cta_signal);
      } else {
      }
    } else if (IsCloseOrder(cta_signal->position_effect)) {
      if (cta_signal->order_status == OrderStatus::kActive &&
          cta_signal->trading_qty == 0) {
        HandleCloseing(cta_signal);
      } else if ((cta_signal->order_status == OrderStatus::kActive ||
                  cta_signal->order_status == OrderStatus::kAllFilled) &&
                 cta_signal->trading_qty != 0) {
        HandleClosed(cta_signal);
      } else {
      }
    }
  }

 private:
  virtual void HandleOpening(
      const std::shared_ptr<CTAOrderSignalField>& cta_signal) {}

  virtual void HandleOpened(
      const std::shared_ptr<CTAOrderSignalField>& cta_signal) {
    if (!IsOppositeOpen(cta_signal)) {
    } else {
      delayed_open_queue_.push(DelayedInputOrder{
          cta_signal->instrument, cta_signal->direction,
          cta_signal->position_effect, cta_signal->trading_price,
          cta_signal->trading_qty, cta_signal->timestamp});
    }
  }

  virtual void HandleCloseing(
      const std::shared_ptr<CTAOrderSignalField>& cta_signal) {}

  virtual void HandleClosed(
      const std::shared_ptr<CTAOrderSignalField>& cta_signal) {}

  virtual void HandleCanceled(
      const std::shared_ptr<CTAOrderSignalField>& cta_signal) {}

  bool IsOppositeOpen(
      const std::shared_ptr<CTAOrderSignalField>& cta_signal) const {
    BOOST_ASSERT(IsOpenOrder(cta_signal->position_effect));
    if (cta_signal->direction == OrderDirection::kBuy) {
      return cta_signal->short_qty > 0;
    } else if (cta_signal->direction == OrderDirection::kSell) {
      return cta_signal->long_qty > 0;
    } else {
    }

    BOOST_ASSERT(false);
    return false;
  }

  std::string GenerateOrderId() { return std : string(); }

  struct DelayedInputOrder {
    std::string instrument;
    OrderDirection direction;
    PositionEffect position_effect;
    double price;
    int qty;
    TimeStamp time_stamp;
  };

  std::list<DelayedInputOrder> delayed_open_queue_;
  MailBox* mail_box_;
};

#endif  // FOLLOW_STRATEGY_CTA_TRADED_STRATEGY_H
