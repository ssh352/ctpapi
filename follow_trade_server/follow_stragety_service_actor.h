#ifndef FOLLOW_TRADE_SERVER_FOLLOW_STRAGETY_SERVICE_ACTOR_H
#define FOLLOW_TRADE_SERVER_FOLLOW_STRAGETY_SERVICE_ACTOR_H
#include "follow_strategy_mode/follow_strategy_service.h"
#include "caf/all.hpp"
class FollowStragetyServiceActor : public caf::event_based_actor,
                                   FollowStragetyService::Delegate {
 public:
  FollowStragetyServiceActor(caf::actor_config& cfg,
                             const std::string& master_account_id,
                             const std::string& slave_account_id,
                             std::vector<OrderPosition> master_init_positions,
                             std::vector<OrderData> master_history_rtn_orders,
                             caf::actor ctp,
                             caf::actor follow,
                             caf::actor binary_log);

  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         OrderPriceType price_type,
                         double price,
                         int quantity) override;

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          OrderPriceType price_type,
                          double price,
                          int quantity) override;

  virtual void CancelOrder(const std::string& order_no) override;

  void on_exit() override {
    destroy(cta_);
    destroy(follow_);
  }

 protected:
  virtual caf::behavior make_behavior() override;

 private:
  FollowStragetyService service_;
  caf::actor cta_;
  caf::actor follow_;
  caf::actor binary_log_;
};

#endif  // FOLLOW_TRADE_SERVER_FOLLOW_STRAGETY_SERVICE_ACTOR_H
