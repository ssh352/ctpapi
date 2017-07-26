#ifndef FOLLOW_TRADE_SERVER_FOLLOW_STRAGETY_SERVICE_ACTOR_H
#define FOLLOW_TRADE_SERVER_FOLLOW_STRAGETY_SERVICE_ACTOR_H
#include "caf/all.hpp"
#include "follow_strategy_mode/cta_generic_strategy.h"
#include "follow_strategy_mode/cta_signal.h"
#include "follow_strategy_mode/cta_signal_dispatch.h"
#include "follow_strategy_mode/enter_order_observer.h"
#include "follow_strategy_mode/orders_context.h"
#include "follow_strategy_mode/rtn_order_observer.h"
#include "follow_strategy_mode/strategy_order_dispatch.h"
#include "follow_trade_server/ctp_portfolio.h"
#include "websocket_typedef.h"
class FollowStragetyServiceActor : public caf::event_based_actor,
                                   StrategyEnterOrderObservable::Observer {
 public:
  FollowStragetyServiceActor(caf::actor_config& cfg,
                             Server* websocket_server,
                             const std::string& master_account_id,
                             const std::string& slave_account_id,
                             std::vector<OrderPosition> master_init_positions,
                             std::vector<OrderData> master_history_rtn_orders,
                             caf::actor ctp,
                             caf::actor follow,
                             caf::actor monitor);

  void on_exit() override {
    destroy(cta_);
    destroy(follow_);
  }

  virtual void OpenOrder(const std::string& strategy_id,
                         const std::string& instrument,
                         const std::string& order_id,
                         OrderDirection direction,
                         double price,
                         int quantity) override;

  virtual void CloseOrder(const std::string& strategy_id,
                          const std::string& instrument,
                          const std::string& order_id,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          double price,
                          int quantity) override;

  virtual void CancelOrder(const std::string& strategy_id,
                           const std::string& order_id) override;

 protected:
  virtual caf::behavior make_behavior() override;

 private:
  template <typename Atom>
  class PortfolioProxy : public PortfolioObserver {
   public:
    PortfolioProxy(caf::actor self, std::string strategy_id)
        : self_(self), strategy_id_(strategy_id) {}

    virtual void Notify(std::vector<AccountPortfolio> portfolio) override {
      caf::anon_send(self_, Atom::value, strategy_id_, std::move(portfolio));
    }

   private:
    caf::actor self_;
    std::string strategy_id_;
  };

  caf::actor cta_;
  caf::actor follow_;
  caf::actor monitor_;
  int portfolio_age_;
  std::vector<OrderPosition> master_init_positions_;
  std::vector<OrderData> master_history_rtn_orders_;
  CTPPortfolio portfolio_;
  std::shared_ptr<OrdersContext> master_context_;
  std::string master_account_id_;
  std::string slave_account_id_;
  std::map<std::pair<TThostFtdcSessionIDType, std::string>, std::string>
      master_adjust_order_ids_;
  Server* websocket_server_;
  connection_hdl hdl_;
  std::vector<std::shared_ptr<CTASignalDispatch>> signal_dispatchs_;
};

#endif  // FOLLOW_TRADE_SERVER_FOLLOW_STRAGETY_SERVICE_ACTOR_H
