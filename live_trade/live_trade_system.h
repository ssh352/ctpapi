#ifndef LIVE_TRADE_LIVE_TRADE_SYSTEM_H
#define LIVE_TRADE_LIVE_TRADE_SYSTEM_H
#include <map>
#include <memory>
#include "bft_core/message.h"
#include "caf/all.hpp"
#include "live_trade_environment.h"

class LiveTradeSystem {
 public:

  void Subscribe(std::type_index type_index, caf::actor actor);


  void SendToNamed(const std::string& named, std::shared_ptr<bft::Message> message);

  void SendToGlobal(std::shared_ptr<bft::Message> message);

  void Send(int env_id, std::shared_ptr<bft::Message> message);

 private:
  LiveTradeEnvironment global_env_;
  std::map<int, LiveTradeEnvironment> private_envs_;
};

#endif  // LIVE_TRADE_LIVE_TRADE_SYSTEM_H
