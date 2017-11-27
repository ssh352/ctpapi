#ifndef LIVE_TRADE_SERIALIZATION_RTN_ORDER_H
#define LIVE_TRADE_SERIALIZATION_RTN_ORDER_H
#include "caf/all.hpp"
#include "common/api_struct.h"

class SerializationCtaRtnOrder : public caf::event_based_actor {
 public:
  SerializationCtaRtnOrder(caf::actor_config& cfg) : event_based_actor(cfg) {}

  virtual caf::behavior make_behavior() override {}
};

class SerializationStrategyRtnOrder : public caf::event_based_actor {
 public:
  SerializationStrategyRtnOrder(caf::actor_config& cfg)
      : event_based_actor(cfg) {}

  virtual caf::behavior make_behavior() override {
    return {[](const std::shared_ptr<CTPOrderField>& order) {}};
  }

 private:
};

#endif  // LIVE_TRADE_SERIALIZATION_RTN_ORDER_H
