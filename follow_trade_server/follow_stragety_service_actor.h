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
                             caf::actor trader,
                             caf::actor cta_signal,
                             std::string master_account_id);

  void on_exit() override {
    destroy(cta_signal_);
    destroy(trader_);
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

  enum class InitState {
    kCTAOrderLists = 0x1,
    kCTAPosition = 0x2,
    
  };

  StrategyOrderDispatch strategy_server_;
  std::string master_account_id_;
  caf::strong_actor_ptr db_;
  caf::actor trader_;
  caf::actor cta_signal_;
  caf::behavior init_;
};

#endif  // FOLLOW_TRADE_SERVER_FOLLOW_STRAGETY_SERVICE_ACTOR_H
