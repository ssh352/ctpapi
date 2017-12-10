#include "caf_strategy_agent.h"

CAFDelayOpenStrategyAgent::CAFDelayOpenStrategyAgent(
    caf::actor_config& cfg,
    boost::property_tree::ptree* strategy_config,
    ProductInfoMananger* product_info_mananger,
    const std::string& account_id,
    LiveTradeSystem* live_trade_system,
    int env_id)
    : caf::event_based_actor(cfg),
      live_trade_system_(live_trade_system),
      env_id_(env_id) {
  agent_ = std::make_unique<DelayOpenStrategyAgent<OptimalOpenPriceStrategy> >(
      this, strategy_config, product_info_mananger, &log_);
  log_.add_attribute("log_tag",
                     boost::log::attributes::constant<std::string>(account_id));
}

caf::behavior CAFDelayOpenStrategyAgent::make_behavior() {
  return {[=](const std::shared_ptr<bft::Message>& message) {
    if (message_handlers_.find(message->TypeIndex()) !=
        message_handlers_.end()) {
      message_handlers_.at(message->TypeIndex())->ApplyMessage(*message);
    }
  }};
}

void CAFDelayOpenStrategyAgent::Subscribe(
    std::unique_ptr<bft::BasedMessageHandler> handler) {
  live_trade_system_->Subscribe(env_id_, handler->TypeIndex(), this);
  message_handlers_.insert({handler->TypeIndex(), std::move(handler)});
}

void CAFDelayOpenStrategyAgent::Send(bft::Message message) {
  live_trade_system_->Send(env_id_, message);
}
