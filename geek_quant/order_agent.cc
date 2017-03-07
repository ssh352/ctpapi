#include "order_agent.h"

OrderAgentActor::behavior_type OrderAgent::make_behavior() {
  return {
      [=](TAPositionAtom, std::vector<PositionData> positions) {
        wait_for_until_receive_positions_ = true;
        std::for_each(positions.begin(), positions.end(), [=](auto position) {
          auto it_find =
              std::find_if(instrument_orders_.begin(), instrument_orders_.end(),
                           [&](auto order) {
                             return position.instrument == order.instrument();
                           });
          if (it_find != instrument_orders_.end()) {
            it_find->AddPositionData(position);
          } else {
            InstrumentOrderAgent inst_order(this, subscriber_,
                                            position.instrument);
            inst_order.AddPositionData(position);
            instrument_orders_.push_back(std::move(inst_order));
          }
        });
        TryProcessPendingEnterOrder();
      },
      [=](TAUnfillOrdersAtom, std::vector<OrderRtnData> orders) {
        std::for_each(orders.begin(), orders.end(), [=](auto order) {
          auto it_find = std::find_if(
              instrument_orders_.begin(), instrument_orders_.end(),
              [&](auto instrument_order) {
                return instrument_order.instrument() == order.instrument;
              });
          if (it_find != instrument_orders_.end()) {
            it_find->AddOrderRtnData(order);
          } else {
            InstrumentOrderAgent inst_order(this, subscriber_,
                                            order.instrument);
            inst_order.AddOrderRtnData(order);
            instrument_orders_.push_back(std::move(inst_order));
          }
        });
        wait_for_until_receive_unfill_orders_ = true;
        TryProcessPendingEnterOrder();
      },
      [=](TARtnOrderAtom, OrderRtnData order) {
        switch (order.order_status) {
          case kOSOpening:
            break;
          case kOSCloseing:
            break;
          case kOSOpened:
            OnOrderOpened(order);
            break;
          case kOSClosed:
            OnOrderClosed(order);
            break;
          case kOSCancel:
            OnOrderCanceled(order);
            break;
          default:
            break;
        }
      },
      [=](EnterOrderAtom, EnterOrderData enter_order) {
        if (!ReadyToEnterOrder()) {
          auto it_find = std::find_if(
              instrument_orders_.begin(), instrument_orders_.end(),
              [=](auto order) {
                return order.instrument() == enter_order.instrument;
              });
          if (it_find != instrument_orders_.end()) {
            it_find->AddPendingEnterOrder(enter_order);
          } else {
            InstrumentOrderAgent inst_order(this, subscriber_,
                                            enter_order.instrument);
            inst_order.AddPendingEnterOrder(enter_order);
            instrument_orders_.push_back(std::move(inst_order));
          }
        } else {
          HandleEnterOrder(enter_order);
        }
      },
      [=](CancelOrderAtom, std::string instrument, std::string order_no) {
        auto it =
            std::find_if(instrument_orders_.begin(), instrument_orders_.end(),
                         [=](auto instrument_order) {
                           return instrument_order.instrument() == instrument;
                         });
        if (it != instrument_orders_.end()) {
          it->HandleCancelOrder(instrument, order_no);
          if (it->IsEmpty()) {
            instrument_orders_.erase(it);
          }
        } else {
          // TODO: LOG
        }
      },
      [=](AddStrategySubscriberAtom, OrderSubscriberActor actor) {
        subscriber_ = actor;
      }};
}

void OrderAgent::OnOrderOpened(const OrderRtnData& order) {
  auto it = std::find_if(instrument_orders_.begin(), instrument_orders_.end(),
                         [=](auto inst_order) {
                           return inst_order.instrument() == order.instrument;
                         });

  if (it != instrument_orders_.end()) {
    it->OnOrderOpened(order);
  } else {
    // ASSERT(it != pending_orders_.end())
  }
}

void OrderAgent::OnOrderClosed(const OrderRtnData& order) {
  auto it = std::find_if(instrument_orders_.begin(), instrument_orders_.end(),
                         [=](auto inst_order) {
                           return inst_order.instrument() == order.instrument;
                         });

  if (it != instrument_orders_.end()) {
    it->OnOrderClosed(order);
  }
}

void OrderAgent::OnOrderCanceled(const OrderRtnData& order) {
  auto it = std::find_if(instrument_orders_.begin(), instrument_orders_.end(),
                         [=](auto inst_order) {
                           return inst_order.instrument() == order.instrument;
                         });

  if (it != instrument_orders_.end()) {
    it->OnOrderCancel(order);
  } else {
  }
}

void OrderAgent::TryProcessPendingEnterOrder() {
  if (!ReadyToEnterOrder()) {
    return;
  }
  std::for_each(
      instrument_orders_.begin(), instrument_orders_.end(),
      [=](auto inst_order) { inst_order.ProcessPendingEnterOrder(); });
}

bool OrderAgent::ReadyToEnterOrder() const {
  return wait_for_until_receive_unfill_orders_ &&
         wait_for_until_receive_positions_;
}

void OrderAgent::HandleEnterOrder(const EnterOrderData& enter_order) {
  auto it =
      std::find_if(instrument_orders_.begin(), instrument_orders_.end(),
                   [=](auto inst_order) {
                     return inst_order.instrument() == enter_order.instrument;
                   });

  if (it != instrument_orders_.end()) {
    it->HandleEnterOrder(enter_order);
  } else {
    InstrumentOrderAgent inst_order(this, subscriber_, enter_order.instrument);
    inst_order.HandleEnterOrder(enter_order);
    instrument_orders_.push_back(std::move(inst_order));
  }
}
