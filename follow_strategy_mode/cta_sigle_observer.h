#ifndef FOLLOW_STRATEGY_MODE_CTA_SIGLE_OBSERVER_H
#define FOLLOW_STRATEGY_MODE_CTA_SIGLE_OBSERVER_H
#include <boost/shared_ptr.hpp>
#include "common/api_struct.h"

class CTASignalObserver {
public:
  class Observable {
   public:
    virtual void OpenOrder(const std::string& instrument,
                           const std::string& order_id,
                           OrderDirection direction,
                           double price,
                           int quantity) = 0;

    virtual void CloseOrder(const std::string& instrument,
                            const std::string& order_id,
                            OrderDirection direction,
                            PositionEffect position_effect,
                            double price,
                            int quantity) = 0;

    virtual void CancelOrder(const std::string& order_id) = 0;
  };

  virtual void HandleOpening(const boost::shared_ptr<const OrderField>& order_data) = 0;
  virtual void HandleCloseing(const boost::shared_ptr<const OrderField>& order_data) = 0;
  virtual void HandleCanceled(const boost::shared_ptr<const OrderField>& order_data) = 0;
  virtual void HandleClosed(const boost::shared_ptr<const OrderField>& order_data) = 0;
  virtual void HandleOpened(const boost::shared_ptr<const OrderField>& order_data) = 0;

  virtual void Subscribe(Observable* observer) = 0;
};
#endif  // FOLLOW_STRATEGY_MODE_CTA_SIGLE_OBSERVER_H
