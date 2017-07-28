#ifndef FOLLOW_STRATEGY_MODE_STRATEGY_ORDER_DISPATCH_H
#define FOLLOW_STRATEGY_MODE_STRATEGY_ORDER_DISPATCH_H
#include <boost/bimap.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sources/logger.hpp>
#include "enter_order_observer.h"
#include "rtn_order_observer.h"
#include "strategy_enter_order_observable.h"
class StrategyOrderDispatch : public StrategyEnterOrderObservable::Observer,
                              public RtnOrderObserver {
 public:
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

  virtual void RtnOrder(
      const boost::shared_ptr<const OrderField>& order) override;

  void SubscribeEnterOrderObserver(
      StrategyEnterOrderObservable::Observer* observer);

  void SubscribeRtnOrderObserver(const std::string& strategy_id,
                                 std::shared_ptr<RtnOrderObserver> observer);

 private:
  StrategyEnterOrderObservable::Observer* enter_order_;
  std::map<std::string, std::shared_ptr<RtnOrderObserver> >
      rtn_order_observers_;
  boost::log::sources::logger log_;
};

#endif  // FOLLOW_STRATEGY_MODE_STRATEGY_ORDER_DISPATCH_H
