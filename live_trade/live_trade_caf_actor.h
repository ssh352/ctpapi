#ifndef LIVE_TRADE_LIVE_TRADE_CAF_ACTOR_H
#define LIVE_TRADE_LIVE_TRADE_CAF_ACTOR_H
#include <map>
#include "caf/all.hpp"
#include "bft_core/channel_delegate.h"
#include "bft_core/message_handler.h"
#include "live_trade_system.h"

class LiveTradeCafActor : public caf::event_based_actor,
                          private bft::ChannelDelegate {
 public:
  LiveTradeCafActor(caf::actor_config& cfg, int env_id);

  virtual caf::behavior make_behavior() override;

 private:
  virtual void Subscribe(
      std::unique_ptr<bft::BasedMessageHandler> handler) override;

  virtual void Send(bft::Message message) override;
  std::map<std::type_index, std::shared_ptr<bft::BasedMessageHandler>>
      handlers_;
  LiveTradeSystem* live_trade_system_;
  int env_id_;
};

#endif  // LIVE_TRADE_LIVE_TRADE_CAF_ACTOR_H
