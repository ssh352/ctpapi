#ifndef FOLLOW_STRATEGY_MODE_FOLLOW_STRAGETY_FRAMEWORK_H
#define FOLLOW_STRATEGY_MODE_FOLLOW_STRAGETY_FRAMEWORK_H
#include <boost/bimap.hpp>
#include <boost/shared_ptr.hpp>
#include "follow_stragety_factory.h"
#include "follow_strategy.h"
#include "follow_strategy_mode/defines.h"
#include "follow_strategy_service.h"

class FollowStrategyRun {
 public:
  FollowStrategyRun();

  void Setup();

  void InitMasterPositions(std::vector<OrderPosition> positions);

  void InitStragetyPositions(const std::string& stragety_id,
                             std::vector<OrderPosition> positions);

  void HandleRtnOrder(OrderData rtn_order);

  class Delegate {
   public:
    virtual void OpenOrder(const std::string& instrument,
                           const std::string& order_no,
                           OrderDirection direction,
                           OrderPriceType price_type,
                           double price,
                           int quantity) = 0;

    virtual void CloseOrder(const std::string& instrument,
                            const std::string& order_no,
                            OrderDirection direction,
                            PositionEffect position_effect,
                            OrderPriceType price_type,
                            double price,
                            int quantity) = 0;

    virtual void CancelOrder(const std::string& order_no) = 0;
  };

  template <typename T>
  class StragetyProxy : FollowStragetyService::Delegate {
   public:
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
    T* delegate_;
  };

 private:
  virtual void OpenOrder(const std::string& stragety_id,
                         const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         OrderPriceType price_type,
                         double price,
                         int quantity);

  virtual void CloseOrder(const std::string& stragety_id,
                          const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          OrderPriceType price_type,
                          double price,
                          int quantity);

  virtual void CancelOrder(const std::string& stragety_id,
                           const std::string& order_no);

  struct StragetyOrder {
    std::string stragety_id;
    std::string order_id;
    bool operator<(const StragetyOrder& r) const {
      return std::make_pair(stragety_id, order_id) <
             std::make_pair(r.stragety_id, r.order_id);
    }
  };

  typedef boost::bimap<StragetyOrder, std::string> StragetyOrderBiMap;
  StragetyOrderBiMap stragety_order_;
  std::map<std::string, boost::shared_ptr<FollowStragetyService> > stragetys_;
  std::vector<std::shared_ptr<StragetyProxy<FollowStrategyRun> > >
      stragetys_proxys_;
  std::string master_account_id_;
  std::string slave_account_id_;
  Delegate* delegate_;
};

#endif  // FOLLOW_STRATEGY_MODE_FOLLOW_STRAGETY_FRAMEWORK_H
