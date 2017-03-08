#include "instrument_order_agent.h"

InstrumentOrderAgent::InstrumentOrderAgent(OrderAgentActor::base* delegate,
                                           OrderSubscriberActor subscriber,
                                           const std::string& instrument)
    : delegate_(delegate), subscriber_(subscriber), instrument_(instrument) {}

const std::string& InstrumentOrderAgent::instrument() const {
  return instrument_;
}

void InstrumentOrderAgent::AddOrderRtnData(const OrderRtnData& order) {
  pending_orders_.push_back(order);
}

void InstrumentOrderAgent::AddPositionData(const PositionData& position) {
  positions_.push_back(position);
}

void InstrumentOrderAgent::AddPendingEnterOrder(
    const EnterOrderData& enter_order) {
  pending_enter_orders_.push_back(enter_order);
}

void InstrumentOrderAgent::OnOrderOpened(const OrderRtnData& order) {
  auto it = std::find_if(
      pending_orders_.begin(), pending_orders_.end(),
      [=](auto pd_order) { return order.instrument == pd_order.instrument; });
  it->volume -= order.volume;
  if (it->volume <= 0) {
    pending_orders_.erase(it);
  }
  PositionData position;
  position.instrument = order.instrument;
  position.order_direction = order.order_direction;
  position.volume = order.volume;
  positions_.push_back(position);
}

void InstrumentOrderAgent::OnOrderClosed(const OrderRtnData& order) {
  auto it = std::find_if(pending_orders_.begin(), pending_orders_.end(),
                         [&](auto pending_order) {
                           return pending_order.order_no == order.order_no;
                         });
  if (it != pending_orders_.end()) {
    auto it_pos =
        std::find_if(positions_.begin(), positions_.end(), [&](auto pos) {
          return pos.instrument == order.instrument &&
                 pos.order_direction != order.order_direction;
        });
    if (it_pos != positions_.end()) {
      it_pos->volume -= order.volume;
      // ASSERT(it_pos->volume>=0)
      if (it_pos->volume <= 0) {
        positions_.erase(it_pos);
      }
    } else {
      // TODO:ASSERT
    }

    it->volume -= order.volume;
    if (it->volume <= 0) {
      pending_orders_.erase(it);
    }
  } else {
    // TODO: ASSERT
  }
}

void InstrumentOrderAgent::OnOrderCancel(const OrderRtnData& order) {
  auto it = std::find_if(pending_orders_.begin(), pending_orders_.end(),
                         [&](auto pending_order) {
                           return pending_order.order_no == order.order_no;
                         });

  // ASSERT(it != pending_orders_.end())
  if (it != pending_orders_.end()) {
    pending_orders_.erase(it);
  }
}

void InstrumentOrderAgent::HandleEnterOrder(const EnterOrderData& enter_order) {
  switch (enter_order.action) {
    case kEOAOpen: {
      HandleEnterOrderForOpen(enter_order);
    } break;
    case kEOAClose: {
      HandleEnterOrderForClose(enter_order);
      return;
    } break;
    case kEOAOpenReverseOrder: {
      HandleEnterOrderForOpenReverseOrder(enter_order);
      return;
    } break;
    case kEOAOpenConfirm:
    case kEOACloseConfirm:
    case kEOAOpenReverseOrderConfirm:
    default:
      break;
  }
}

void InstrumentOrderAgent::HandleEnterOrderForOpenReverseOrder(
    const EnterOrderData& enter_order) {
  auto it_pos =
      std::find_if(positions_.begin(), positions_.end(), [&](auto position) {
        return enter_order.instrument == position.instrument &&
               enter_order.order_direction != position.order_direction;
      });
  if (it_pos != positions_.end()) {
    if (enter_order.old_volume == it_pos->volume) {
      delegate_->send(subscriber_, EnterOrderAtom::value, enter_order);
    } else if (enter_order.old_volume < enter_order.volume) {
      EnterOrderData real_enter_order(enter_order);
      real_enter_order.volume =
          it_pos->volume + (enter_order.old_volume - enter_order.volume);
      delegate_->send(subscriber_, EnterOrderAtom::value, real_enter_order);
    } else {
      if (enter_order.volume > it_pos->volume) {
        EnterOrderData real_enter_order(enter_order);
        real_enter_order.volume = it_pos->volume;
        delegate_->send(subscriber_, EnterOrderAtom::value, real_enter_order);
        auto pending_order = std::find_if(
            pending_orders_.begin(), pending_orders_.end(), [&](auto order) {
              return order.instrument == enter_order.instrument &&
                     order.order_direction != enter_order.order_direction;
            });
        if (pending_order != pending_orders_.end()) {
          if (pending_order->volume == enter_order.volume - it_pos->volume) {
            delegate_->send(subscriber_, CancelOrderAtom::value,
                            pending_order->instrument, pending_order->order_no);
            pending_orders_.erase(pending_order);
          } else {
            // TODO:ASSERT()
          }
        } else {
          // TODO:ASSERT()
        }
      } else {
        // enter_order.volume < it_pos->volume
        EnterOrderData real_enter_order(enter_order);
        real_enter_order.volume =
            it_pos->volume - (enter_order.old_volume - enter_order.volume);
        delegate_->send(subscriber_, EnterOrderAtom::value, real_enter_order);
      }
    }
  } else {
    while (true) {
      auto pending_order = std::find_if(
          pending_orders_.begin(), pending_orders_.end(), [&](auto order) {
            return order.order_direction != enter_order.order_direction;
          });
      if (pending_order != pending_orders_.end()) {
        delegate_->send(subscriber_, CancelOrderAtom::value,
                        pending_order->instrument, pending_order->order_no);
        pending_orders_.erase(pending_order);
      } else {
        break;
      }
    }
  }
}

void InstrumentOrderAgent::HandleEnterOrderForClose(
    const EnterOrderData& enter_order) {
  // ASSERT(not in pending orders)
  auto it_pos =
      std::find_if(positions_.begin(), positions_.end(), [&](auto position) {
        return enter_order.instrument == position.instrument &&
               enter_order.order_direction != position.order_direction;
      });
  if (it_pos != positions_.end()) {
    EnterOrderData real_enter_order(enter_order);
    real_enter_order.volume = it_pos->volume;
    delegate_->send(subscriber_, EnterOrderAtom::value, real_enter_order);
    OrderRtnData order;
    order.instrument = enter_order.instrument;
    order.order_no = enter_order.order_no;
    order.order_status = OrderStatus::kOSCloseing;
    order.order_direction = enter_order.order_direction;
    order.order_price = enter_order.order_price;
    order.volume = enter_order.volume;
    pending_orders_.push_back(order);
  }
}

void InstrumentOrderAgent::HandleEnterOrderForOpen(
    const EnterOrderData& enter_order) {
  // ASSERT(enter_order.order_no not in pending_orders_);
  delegate_->send(subscriber_, EnterOrderAtom::value, enter_order);
  OrderRtnData order;
  order.instrument = enter_order.instrument;
  order.order_no = enter_order.order_no;
  order.order_status = OrderStatus::kOSOpening;
  order.order_direction = enter_order.order_direction;
  order.order_price = enter_order.order_price;
  order.volume = enter_order.volume;
  pending_orders_.push_back(order);
}

void InstrumentOrderAgent::HandleCancelOrder(const std::string& instrument,
                                             const std::string& order_no) {
  auto it =
      std::find_if(pending_orders_.begin(), pending_orders_.end(),
                   [&](auto order) { return order_no == order.order_no; });
  if (it != pending_orders_.end()) {
    delegate_->send(subscriber_, CancelOrderAtom::value, instrument, order_no);
    pending_orders_.erase(it);
  } else {
    // TODO
  }
}

void InstrumentOrderAgent::ProcessPendingEnterOrder() {
  std::for_each(
      pending_enter_orders_.begin(), pending_enter_orders_.end(),
      [=](auto enter_order) {
        auto it = std::find_if(
            pending_orders_.begin(), pending_orders_.end(),
            [&](auto order) { return order.order_no == enter_order.order_no; });
        if (it == pending_orders_.end()) {
          HandleEnterOrder(enter_order);
        }
      });
  pending_enter_orders_.clear();
}

bool InstrumentOrderAgent::IsEmpty() const {
  return pending_orders_.empty() && pending_enter_orders_.empty() &&
         positions_.empty();
}
