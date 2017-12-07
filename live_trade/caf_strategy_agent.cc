#include "caf_strategy_agent.h"

CAFDelayOpenStrategyAgent::CAFDelayOpenStrategyAgent(
    caf::actor_config& cfg,
    boost::property_tree::ptree* strategy_config,
    ProductInfoMananger* product_info_mananger,
    const std::string& account_id,
    LiveTradeMailBox* inner_mail_box,
    LiveTradeMailBox* common_mail_box)
    : caf::event_based_actor(cfg),
      agent_(this, strategy_config, product_info_mananger, &log_),
      inner_mail_box_(inner_mail_box),
      common_mail_box_(common_mail_box) {
  log_.add_attribute("log_tag",
                     boost::log::attributes::constant<std::string>(account_id));
}

caf::behavior CAFDelayOpenStrategyAgent::make_behavior() {
  return message_handler_;
}
