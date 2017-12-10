#ifndef LIVE_TRADE_LIVE_TRADE_SYSTEM_H
#define LIVE_TRADE_LIVE_TRADE_SYSTEM_H
#include <map>
#include <memory>
#include "bft_core/message.h"
#include "caf/all.hpp"
#include "live_trade_environment.h"

class LiveTradeSystem {
 public:
   LiveTradeSystem();

  int CreateEnvionment();

  int CreateNamedEnvionment(const std::string& name);

  int GetGlobalEnvionment() const;

  void Subscribe(int env_id, std::type_index type_index, caf::actor actor);

  void SendToNamed(const std::string& named,
                   std::shared_ptr<bft::Message> message);

  void SendToGlobal(std::shared_ptr<bft::Message> message);

  void Send(int env_id, std::shared_ptr<bft::Message> message);

 private:
  LiveTradeEnvironment global_env_;
  std::set<std::type_index> global_env_message_types_;
  std::map<std::string, int> env_names_;
  std::map<int, LiveTradeEnvironment> private_envs_;
  int next_env_id_= 1;
};

#endif  // LIVE_TRADE_LIVE_TRADE_SYSTEM_H
