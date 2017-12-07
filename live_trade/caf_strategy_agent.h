#ifndef LIVE_TRADE_CAF_STRATEGY_AGENT_H
#define LIVE_TRADE_CAF_STRATEGY_AGENT_H
#include <boost/log/sources/logger.hpp>
#include "caf/all.hpp"
#include "live_trade_mail_box.h"
#include "follow_strategy/optimal_open_price_strategy.h"
#include "caf_common/caf_atom_defines.h"
#include "follow_strategy/delay_open_strategy_agent.h"
class CAFDelayOpenStrategyAgent : public caf::event_based_actor {
 public:
  CAFDelayOpenStrategyAgent(caf::actor_config& cfg,
                            boost::property_tree::ptree* strategy_config,
                            ProductInfoMananger* product_info_mananger,
                            const std::string& account_id,
                            LiveTradeMailBox* inner_mail_box,
                            LiveTradeMailBox* common_mail_box);

  virtual caf::behavior make_behavior() override;

  template <typename... Ts>
  void Send(Ts&&... args) {
    inner_mail_box_->Send(std::forward<Ts>(args)...);
  }

  template <typename CLASS, typename... Ts>
  void Subscribe(void (CLASS::*pfn)(Ts...), CLASS* ptr) {
    common_mail_box_->Subscribe(typeid(std::tuple<std::decay_t<Ts>...>), this);
    message_handler_ = message_handler_.or_else(
        [ptr, pfn](Ts... args) { (ptr->*pfn)(args...); });
  }

  template <typename CLASS>
  void Subscribe(void (CLASS::*pfn)(const std::shared_ptr<OrderField>&),
                 CLASS* ptr) {
    inner_mail_box_->Subscribe(typeid(std::tuple<std::shared_ptr<OrderField>>),
                               this);
    message_handler_ = message_handler_.or_else(
        [ptr, pfn](const std::shared_ptr<OrderField>& order) {
          (ptr->*pfn)(order);
        });
  }

  template <typename CLASS>
  void Subscribe(void (CLASS::*pfn)(const std::vector<OrderPosition>&),
                 CLASS* ptr) {
    inner_mail_box_->Subscribe(typeid(std::tuple<std::vector<OrderPosition>>),
                               this);
    message_handler_ = message_handler_.or_else(
        [ptr, pfn](const std::vector<OrderPosition>& order) {
          (ptr->*pfn)(order);
        });
  }

 private:
  boost::log::sources::logger log_;
  LiveTradeMailBox* inner_mail_box_;
  LiveTradeMailBox* common_mail_box_;
  caf::message_handler message_handler_;
  DelayOpenStrategyAgent<CAFDelayOpenStrategyAgent, OptimalOpenPriceStrategy>
      agent_;
};

#endif  // LIVE_TRADE_CAF_STRATEGY_AGENT_H
