#ifndef LIVE_TRADE_LIVE_TRADE_SYSTEM_H
#define LIVE_TRADE_LIVE_TRADE_SYSTEM_H
#include <map>
#include <memory>
#include <bft_core/message.h>
#include "live_trade_environment.h"

class LiveTradeSystem {
public:
  void Send(int env_id, std::shared_ptr<bft::Message> message);

private:
  LiveTradeEnvironment global_env_;
  std::map<int, LiveTradeEnvironment> private_envs_;
};

#endif // LIVE_TRADE_LIVE_TRADE_SYSTEM_H



