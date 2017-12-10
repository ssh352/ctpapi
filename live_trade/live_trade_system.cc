#include "live_trade_system.h"
#include <boost/assert.hpp>

void LiveTradeSystem::Send(int env_id, std::shared_ptr<bft::Message> message) {
  if (private_envs_.find(env_id) != private_envs_.end()) {
    private_envs_.at(env_id).Send(message);
  } else {
    BOOST_ASSERT(false);
  }
}

void LiveTradeSystem::SendToGlobal(std::shared_ptr<bft::Message> message) {
  global_env_.Send(message);
}
