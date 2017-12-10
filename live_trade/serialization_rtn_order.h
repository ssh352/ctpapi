#ifndef LIVE_TRADE_SERIALIZATION_RTN_ORDER_H
#define LIVE_TRADE_SERIALIZATION_RTN_ORDER_H
#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include "caf/all.hpp"
#include "common/api_struct.h"

#include "bft_core/channel_delegate.h"
#include "live_trade_system.h"

class SerializationCtaRtnOrder : public caf::event_based_actor {
 public:
  SerializationCtaRtnOrder(caf::actor_config& cfg, LiveTradeSystem* mail_box,
    int env_id);

  virtual caf::behavior make_behavior() override;

  void Do(int, std::string);
 private:
  LiveTradeSystem* mail_box_;
  std::ofstream file_;
  boost::archive::binary_oarchive oa_;
  int env_id_;
};

class SerializationStrategyRtnOrder : public caf::event_based_actor {
 public:
  SerializationStrategyRtnOrder(caf::actor_config& cfg,
                                LiveTradeSystem* live_trade_system,
    int env_id,
                                std::string account_id);

  virtual caf::behavior make_behavior() override;

 private:
  std::string account_id_;
  std::ofstream file_;
  LiveTradeSystem* live_trade_system_;
  boost::archive::binary_oarchive oa_;
  int env_id_;
};

#endif  // LIVE_TRADE_SERIALIZATION_RTN_ORDER_H
