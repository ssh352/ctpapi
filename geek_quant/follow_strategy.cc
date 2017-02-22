#include "geek_quant/follow_strategy.h"
#include "follow_strategy.h"

FollowStrategy::FollowStrategy(caf::actor_config& cfg)
    : CtpObserver::base(cfg) {}

FollowStrategy::~FollowStrategy() {}

CtpObserver::behavior_type FollowStrategy::make_behavior() {
  return {
      [=](LoginAtom) { caf::aout(this) << "Login\n"; },
      [=](RtnOrderAtom, CThostFtdcOrderField order) {
        for (auto listener : listeners_) {
          auto msg = caf::make_message(
              RtnOrderAtom::value, order.InstrumentID,
              order.Direction == THOST_FTDC_D_Buy ? kDirectionBuy
                                                  : kDirectionSell,
              order.LimitPrice, order.VolumeTotalOriginal);
          // caf::anon_send(caf::actor_cast<caf::actor>(listener), msg);
          caf::anon_send(caf::actor_cast<caf::actor>(listener), msg);
        }
      },
      [=](AddListenerAtom, caf::strong_actor_ptr listener) {
        listeners_.push_back(listener);
      },
  };
}
