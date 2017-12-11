#include "cta_traded_strategy.h"
#include <boost/lexical_cast.hpp>

std::string CTATradedStrategy::GenerateOrderId() {
  return boost::lexical_cast<std::string>(order_id_seq_++);
}

void CTATradedStrategy::HandleCTASignalEx(
    CTASignalAtom,
    const std::shared_ptr<OrderField>& order) {
  if (order->status == OrderStatus::kAllFilled) {
    mail_box_->Send(InputOrder{order->instrument_id, GenerateOrderId(),
                               order->position_effect, order->direction,
                               order->input_price, order->qty,
                               last_tick_->timestamp});
  }
}

void CTATradedStrategy::HandleCTASignal(
    const std::shared_ptr<OrderField>& order,
    const CTAPositionQty&) {
  if (order->status == OrderStatus::kAllFilled) {
    mail_box_->Send(InputOrder{order->instrument_id, GenerateOrderId(),
                               order->position_effect, order->direction,
                               order->input_price, order->qty,
                               last_tick_->timestamp});
  }
}

void CTATradedStrategy::HandleTick(const std::shared_ptr<TickData>& tick) {
  last_tick_ = tick->tick;
}

CTATradedStrategy::CTATradedStrategy(bft::ChannelDelegate* mail_box)
    : mail_box_(mail_box) {
  bft::MessageHandler handler;
  handler.Subscribe(&CTATradedStrategy::HandleCTASignal, this);
  handler.Subscribe(&CTATradedStrategy::HandleCTASignalEx, this);
  handler.Subscribe(&CTATradedStrategy::HandleTick, this);
  mail_box_->Subscribe(std::move(handler));
}
