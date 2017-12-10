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
  return caf_message_handler_;
}

void CAFDelayOpenStrategyAgent::Subscribe(bft::MessageHandler handler) {
  for (const auto& type_index : handler.TypeIndexs()) {
    live_trade_system_->Subscribe(env_id_, type_index, this);
  }
  caf_message_handler_ = handler.message_handler();
}

void CAFDelayOpenStrategyAgent::Send(bft::Message message) {
  live_trade_system_->Send(env_id_, std::move(message));
}
