#include "caf_strategy_agent.h"

CAFDelayOpenStrategyAgent::CAFDelayOpenStrategyAgent(
    caf::actor_config& cfg,
    boost::property_tree::ptree* strategy_config,
    ProductInfoMananger* product_info_mananger,
    const std::string& account_id,
    LiveTradeSystem* live_trade_system)
    : caf::event_based_actor(cfg),
      agent_(this, strategy_config, product_info_mananger, &log_),
      live_trade_system_(live_trade_system) {
  log_.add_attribute("log_tag",
                     boost::log::attributes::constant<std::string>(account_id));
}

caf::behavior CAFDelayOpenStrategyAgent::make_behavior() {
  return message_handler_;
}

void CAFDelayOpenStrategyAgent::Subscribe(
    std::unique_ptr<bft::BasedMessageHandler> handler) {
}

void CAFDelayOpenStrategyAgent::Send(std::shared_ptr<bft::Message> message) {
}
