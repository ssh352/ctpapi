#include "live_trade_system.h"
#include <boost/assert.hpp>
#include "common/api_struct.h"
#include "caf_common/caf_atom_defines.h"

LiveTradeSystem::LiveTradeSystem() {
  global_env_message_types_.insert(
      typeid(std::tuple<std::shared_ptr<OrderField>, CTAPositionQty>));
  global_env_message_types_.insert(
      typeid(std::tuple<std::shared_ptr<TickData>>));
  global_env_message_types_.insert(typeid(std::tuple<ExchangeStatus>));
  global_env_message_types_.insert(typeid(std::tuple<SerializationFlushAtom>));
}

void LiveTradeSystem::Subscribe(int env_id,
                                std::type_index type_index,
                                caf::actor actor) {
  if (global_env_message_types_.find(type_index) !=
      global_env_message_types_.end()) {
    global_env_.Subscribe(type_index, actor);
  } else {
    BOOST_ASSERT(private_envs_.find(env_id) != private_envs_.end());
    private_envs_.at(env_id).Subscribe(type_index, actor);
  }
}

void LiveTradeSystem::Send(int env_id, std::shared_ptr<bft::Message> message) {
  if (env_id == GetGlobalEnvionment()) {
    global_env_.Send(message);
  } else {
    if (private_envs_.find(env_id) != private_envs_.end()) {
      private_envs_.at(env_id).Send(message);
    } else {
      BOOST_ASSERT(false);
    }
  }
}

void LiveTradeSystem::SendToGlobal(std::shared_ptr<bft::Message> message) {
  global_env_.Send(message);
}

void LiveTradeSystem::SendToNamed(const std::string& named,
                                  std::shared_ptr<bft::Message> message) {}

int LiveTradeSystem::CreateEnvionment() {
  private_envs_.insert({next_env_id_, LiveTradeEnvironment()});
  return next_env_id_++;
}

int LiveTradeSystem::CreateNamedEnvionment(const std::string& name) {
  env_names_.insert({name, next_env_id_});
  return CreateEnvionment();
}

int LiveTradeSystem::GetGlobalEnvionment() const {
  return 0;
}
