#ifndef FOLLOW_STRATEGY_MODE_STRATEGY_ORDER_DISPATCH_H
#define FOLLOW_STRATEGY_MODE_STRATEGY_ORDER_DISPATCH_H
#include <boost/bimap.hpp>
#include "rtn_order_observer.h"
#include "strategy_enter_order_observable.h"
#include "enter_order_observer.h"
class StrategyOrderDispatch : public StrategyEnterOrderObservable::Observer,
                              public RtnOrderObserver {
 public:
  virtual void OpenOrder(const std::string& strategy_id,
                         const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         OrderPriceType price_type,
                         double price,
                         int quantity) override;

  virtual void CloseOrder(const std::string& strategy_id,
                          const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          OrderPriceType price_type,
                          double price,
                          int quantity) override;

  virtual void CancelOrder(const std::string& strategy_id,
                           const std::string& order_no) override;

  virtual void RtnOrder(OrderData order) override;

  void SubscribeEnterOrderObserver(EnterOrderObserver* observer);
 private:
  struct StragetyOrder {
    std::string strategy_id;
    std::string order_id;
    bool operator<(const StragetyOrder& r) const {
      return std::make_pair(strategy_id, order_id) <
             std::make_pair(r.strategy_id, r.order_id);
    }
  };

  typedef boost::bimap<StragetyOrder, std::string> StragetyOrderBiMap;
  StragetyOrderBiMap stragety_orders_;
  EnterOrderObserver* enter_order_;
  std::map<std::string, RtnOrderObserver*> stragetys_;
};

#endif  // FOLLOW_STRATEGY_MODE_STRATEGY_ORDER_DISPATCH_H
