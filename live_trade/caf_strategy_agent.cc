#include "caf_strategy_agent.h"

CAFDelayOpenStrategyAgent::CAFDelayOpenStrategyAgent(
    caf::actor_config& cfg,
    std::unordered_map<std::string, OptimalOpenPriceStrategy::StrategyParam>
        params,
    LiveTradeMailBox* inner_mail_box,
    LiveTradeMailBox* common_mail_box)
    : caf::event_based_actor(cfg),
      agent_(this, std::move(params), &log_),
      inner_mail_box_(inner_mail_box),
      common_mail_box_(common_mail_box) {}

caf::behavior CAFDelayOpenStrategyAgent::make_behavior()
{
  return message_handler_;
}
