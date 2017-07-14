#ifndef FOLLOW_TRADE_SERVER_FOLLOW_STRAGETY_SERVICE_ACTOR_H
#define FOLLOW_TRADE_SERVER_FOLLOW_STRAGETY_SERVICE_ACTOR_H
#include "caf/all.hpp"
#include "follow_strategy_mode/enter_order_observer.h"
#include "follow_trade_server/ctp_portfolio.h"
#include "follow_strategy_mode/orders_context.h"
#include "follow_strategy_mode/rtn_order_observer.h"
class FollowStragetyServiceActor : public caf::event_based_actor,
                                   EnterOrderObserver {
 public:
  FollowStragetyServiceActor(caf::actor_config& cfg,
                             const std::string& master_account_id,
                             const std::string& slave_account_id,
                             std::vector<OrderPosition> master_init_positions,
                             std::vector<OrderData> master_history_rtn_orders,
                             caf::actor ctp,
                             caf::actor follow,
                             caf::actor monitor);

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
  caf::actor cta_;
  caf::actor follow_;
  caf::actor monitor_;
  int portfolio_age_;
  std::vector<OrderPosition>  master_init_positions_;
  std::vector<OrderData> master_history_rtn_orders_;
  RtnOrderObserver* service_;
  CTPPortfolio portfolio_;
  OrdersContext master_context_;
  OrdersContext slave_context_;
  
};

#endif  // FOLLOW_TRADE_SERVER_FOLLOW_STRAGETY_SERVICE_ACTOR_H
