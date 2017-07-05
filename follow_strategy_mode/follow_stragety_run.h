#ifndef FOLLOW_STRATEGY_MODE_FOLLOW_STRAGETY_FRAMEWORK_H
#define FOLLOW_STRATEGY_MODE_FOLLOW_STRAGETY_FRAMEWORK_H
#include <boost/shared_ptr.hpp>
#include "follow_strategy_mode/defines.h"
#include "follow_strategy_service.h"

class FollowStrategyRun {
 public:
  FollowStrategyRun();

  void Setup();

  void HandleRtnOrder(OrderData rtn_order);

  class Delegate1 {
   public:
    virtual void OpenOrder(const std::string& stragety_id,
                           const std::string& instrument,
                           const std::string& order_no,
                           OrderDirection direction,
                           OrderPriceType price_type,
                           double price,
                           int quantity) = 0;

    virtual void CloseOrder(const std::string& stragety_id,
                            const std::string& instrument,
                            const std::string& order_no,
                            OrderDirection direction,
                            PositionEffect position_effect,
                            OrderPriceType price_type,
                            double price,
                            int quantity) = 0;

    virtual void CancelOrder(const std::string& stragety_id,
                             const std::string& order_no) = 0;
  };

  template<typename T>
  class StragetyProxy : FollowStragetyService::Delegate {
   public:
    StragetyProxy(std::string stragety_id,
                  boost::shared_ptr<FollowStragetyService> stragety,
                  Delegate1* delegate)
        : stragety_id_(std::move(stragety_id)), delegate_(delegate) {}

    virtual void OpenOrder(const std::string& instrument,
                           const std::string& order_no,
                           OrderDirection direction,
                           OrderPriceType price_type,
                           double price,
                           int quantity) override {
      delegate_->OpenOrder(stragety_id_, instrument, order_no, direction,
                           price_type, price, quantity);
    }

    virtual void CloseOrder(const std::string& instrument,
                            const std::string& order_no,
                            OrderDirection direction,
                            PositionEffect position_effect,
                            OrderPriceType price_type,
                            double price,
                            int quantity) override {
      delegate_->CloseOrder(stragety_id_, instrument, order_no, direction,
                            position_effect, price_type, price, quantity);
    }

    virtual void CancelOrder(const std::string& order_no) override {
      delegate_->CancelOrder(stragety_id_, order_no);
    }

   private:
    std::string stragety_id_;
    Delegate1* delegate_;
  };

 private:
  std::map<std::string, boost::shared_ptr<FollowStragetyService> > stragetys_;
  std::string master_account_id_;
  std::string slave_account_id_;
};

#endif  // FOLLOW_STRATEGY_MODE_FOLLOW_STRAGETY_FRAMEWORK_H
