#include "geek_quant/follow_strategy.h"
#include "follow_strategy.h"

FollowStrategy::FollowStrategy(caf::actor_config& cfg)
    : CtpObserver::base(cfg) {}

FollowStrategy::~FollowStrategy() {}

CtpObserver::behavior_type FollowStrategy::make_behavior() {
  return {
      [=](CtpLoginAtom) { caf::aout(this) << "Login\n"; },
      [&](CtpRtnOrderAtom, CThostFtdcOrderField order) {
        if (order.OrderStatus == THOST_FTDC_OST_Canceled) {
          if (enqueue_orders_.find(order.OrderRef) == enqueue_orders_.end()) {
            // TODO: Need log
            return;
          }
          auto msg = caf::make_message(CancelOrderAtom::value, order.OrderRef);
          for (auto listener : listeners_) {
            caf::anon_send(caf::actor_cast<caf::actor>(listener), msg);
          }
        } else if (order.OrderSubmitStatus == THOST_FTDC_OSS_InsertSubmitted) {
          if (enqueue_orders_.find(order.OrderRef) != enqueue_orders_.end()) {
            return;
          }
          if (order.CombOffsetFlag[0] == THOST_FTDC_OF_Open) {
            auto msg = caf::make_message(
                OpenOrderAtom::value, order.InstrumentID, order.OrderRef,
                order.Direction == THOST_FTDC_D_Buy ? OrderDirection::kBuy
                                                    : OrderDirection::kSell,
                order.LimitPrice, order.VolumeTotalOriginal);
            for (auto listener : listeners_) {
              caf::anon_send(caf::actor_cast<caf::actor>(listener), msg);
            }
          } else if (order.CombOffsetFlag[0] == THOST_FTDC_OF_Close) {
            auto msg = caf::make_message(
                CloseOrderAtom::value, order.InstrumentID, order.OrderRef,
                order.Direction == THOST_FTDC_D_Buy ? OrderDirection::kBuy
                                                    : OrderDirection::kSell,
                order.LimitPrice, order.VolumeTotalOriginal);
            for (auto listener : listeners_) {
              caf::anon_send(caf::actor_cast<caf::actor>(listener), msg);
            }
          } else {
          }
          enqueue_orders_[order.OrderRef] = order;
        }

      },
      [=](AddListenerAtom, caf::strong_actor_ptr listener) {
        listeners_.push_back(listener);
      },
  };
}
