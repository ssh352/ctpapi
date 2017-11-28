#ifndef LIVE_TRADE_SERIALIZATION_RTN_ORDER_H
#define LIVE_TRADE_SERIALIZATION_RTN_ORDER_H
#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include "caf/all.hpp"
#include "common/api_struct.h"
#include "live_trade_mail_box.h"

class SerializationCtaRtnOrder : public caf::event_based_actor {
 public:
  SerializationCtaRtnOrder(caf::actor_config& cfg, LiveTradeMailBox* mail_box);

  virtual caf::behavior make_behavior() override;

 private:
  LiveTradeMailBox* mail_box_;
  std::fstream file_;
  boost::archive::binary_oarchive oa_;
};

class SerializationStrategyRtnOrder : public caf::event_based_actor {
 public:
  SerializationStrategyRtnOrder(caf::actor_config& cfg,
                                LiveTradeMailBox* mail_box,
                                std::string account_id);

  virtual caf::behavior make_behavior() override;

 private:
  std::string account_id_;
  std::fstream file_;
  LiveTradeMailBox* mail_box_;
  boost::archive::binary_oarchive oa_;
};

#endif  // LIVE_TRADE_SERIALIZATION_RTN_ORDER_H
