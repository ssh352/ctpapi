#ifndef LIVE_TRADE_CAF_STRATEGY_AGENT_H
#define LIVE_TRADE_CAF_STRATEGY_AGENT_H
#include <boost/log/sources/logger.hpp>
#include "caf/all.hpp"

#include "follow_strategy/optimal_open_price_strategy.h"
#include "caf_common/caf_atom_defines.h"
#include "follow_strategy/delay_open_strategy_agent.h"
#include "live_trade_system.h"
#include "bft_core/channel_delegate.h"
class CAFDelayOpenStrategyAgent : public caf::event_based_actor,
                                  public bft::ChannelDelegate {
 public:
  CAFDelayOpenStrategyAgent(caf::actor_config& cfg,
                            boost::property_tree::ptree* strategy_config,
                            ProductInfoMananger* product_info_mananger,
                            const std::string& account_id,
                            LiveTradeSystem* live_trade_system);

  virtual caf::behavior make_behavior() override;

 private:
  boost::log::sources::logger log_;
  LiveTradeSystem* live_trade_system_;
  caf::message_handler message_handler_;
  DelayOpenStrategyAgent<OptimalOpenPriceStrategy> agent_;

 public:
  virtual void Subscribe(
      std::unique_ptr<bft::BasedMessageHandler> handler) override;

  virtual void Send(std::shared_ptr<bft::Message> message) override;
};

#endif  // LIVE_TRADE_CAF_STRATEGY_AGENT_H
