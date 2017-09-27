#ifndef follow_strategy_CTA_SIGNAL_DISPATCH_H
#define follow_strategy_CTA_SIGNAL_DISPATCH_H
#include <unordered_set>
#include "cta_sigle_observer.h"
#include "enter_order_observer.h"
#include "orders_context.h"
#include "rtn_order_observer.h"
#include "portfolio_observer.h"

class CTASignalDispatch : public CTASignalObserver::Observable,
                          public RtnOrderObserver {
 public:
  CTASignalDispatch(CTASignalObserver* signal_observer,
                    std::string slave_account_id);

  virtual void SubscribeEnterOrderObserver(EnterOrderObserver* observer);

  // RtnOrderObservable::Observer
  virtual void RtnOrder(
      const std::shared_ptr<const OrderField>& order) override;

  // EnterOrderObservable::Observer
  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_id,
                         OrderDirection direction,
                         double price,
                         int quantity) override;

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_id,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          double price,
                          int quantity) override;

  virtual void CancelOrder(const std::string& order_id) override;

  void SubscribePortfolioObserver(std::shared_ptr<PortfolioObserver> observer);

 private:
  enum class StragetyStatus {
    kWaitReply,
    kReady,
    kSkip,
  };

  struct OrderFieldHask {
    size_t operator()(const std::shared_ptr<const OrderField>& order) const {
      return std::hash<std::string>()(order->strategy_id + order->order_id);
    }
  };

  struct OrderFieldCompare {
    bool operator()(const std::shared_ptr<const OrderField>& l,
                    const std::shared_ptr<const OrderField>& r) const {
      return l->strategy_id == r->strategy_id && l->order_id == r->order_id;
    }
  };

  void Trade(const std::string& order_id, OrderStatus status);

  void DoHandleRtnOrder(const std::shared_ptr<const OrderField>& rtn_order);

  StragetyStatus BeforeHandleOrder(
      const std::shared_ptr<const OrderField>& order);

  OrderEventType OrdersContextHandleRtnOrder(
      const std::shared_ptr<const OrderField>& order);

  std::vector<std::pair<std::string, OrderStatus>> waiting_reply_order_;

  std::deque<std::shared_ptr<const OrderField>> outstanding_orders_;
  CTASignalObserver* signal_observer_;
  EnterOrderObserver* enter_order_observer_;
  std::string slave_account_id_;
  std::shared_ptr<PortfolioObserver> portfolio_observer_;
  std::unordered_set<std::shared_ptr<const OrderField>,
                     OrderFieldHask,
                     OrderFieldCompare>
      orders_;
};
#endif  // follow_strategy_CTA_SIGNAL_DISPATCH_H
