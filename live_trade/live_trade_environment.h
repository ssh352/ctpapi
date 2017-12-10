#ifndef LIVE_TRADE_LIVE_TRADE_ENVIRONMENT_H
#define LIVE_TRADE_LIVE_TRADE_ENVIRONMENT_H
#include <map>
#include <typeindex>
#include <memory>
#include "caf/all.hpp"
#include "bft_core/message.h"

class LiveTradeEnvironment {
 public:
  void Subscribe(std::type_index type_index, caf::actor actor);
  void Send(const std::shared_ptr<bft::Message>& message);

 private:
  std::map<std::type_index, caf::actor> actors_;
};

#endif  // LIVE_TRADE_LIVE_TRADE_ENVIRONMENT_H
