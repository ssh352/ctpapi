#ifndef FOLLOW_STRATEGY_FOLLOW_STRATEGY_H
#define FOLLOW_STRATEGY_FOLLOW_STRATEGY_H

#include "follow_strategy/cta_generic_strategy.h"
#include "follow_strategy/cta_signal.h"
#include "follow_strategy/cta_signal_dispatch.h"
#include "follow_strategy/logging_defines.h"
#include "follow_strategy/strategy_order_dispatch.h"
#include "follow_strategy/string_util.h"

template <typename MailBox>
class FollowStrategy : public StrategyEnterOrderObservable::Observer {
 public:
  FollowStrategy(MailBox* mail_box,
                 std::string master_account_id,
                 std::string slave_account_id)
      : mail_box_(mail_box),
        master_account_id_(master_account_id),
        slave_account_id_(slave_account_id) {
    master_context_ = std::make_shared<OrdersContext>(master_account_id_);
    slave_context_ = std::make_shared<OrdersContext>(slave_account_id_);
    cta_strategy_ = std::make_shared<CTAGenericStrategy>(slave_account_id);
    signal_ = std::make_shared<CTASignal>();
    signal_->SetOrdersContext(master_context_, slave_context_);
    signal_dispatch_ = std::make_shared<CTASignalDispatch>(signal_);
    signal_dispatch_->SetOrdersContext(master_context_, slave_context_);
    cta_strategy_->Subscribe(&strategy_dispatch_);
    signal_dispatch_->SubscribeEnterOrderObserver(cta_strategy_);
    strategy_dispatch_.SubscribeEnterOrderObserver(this);
    strategy_dispatch_.SubscribeRtnOrderObserver(slave_account_id_,
                                                 signal_dispatch_);

    mail_box_->Subscribe(&FollowStrategy::HandleInitPosition, this);
    mail_box_->Subscribe(&FollowStrategy::HandleHistoryOrder, this);
    mail_box_->Subscribe(&FollowStrategy::HandleOrder, this);
    mail_box_->Subscribe(&FollowStrategy::HandleCTASignalInitPosition, this);
    mail_box_->Subscribe(&FollowStrategy::HandleCTASignalHistoryOrder, this);
    mail_box_->Subscribe(&FollowStrategy::HandleCTASignalOrder, this);
  }

  void HandleInitPosition(const std::vector<OrderPosition>& quantitys) {
    slave_context_->InitPositions(quantitys);
  }

  void HandleHistoryOrder(
      const std::vector<std::shared_ptr<const OrderField> >& orders) {
    for (const auto& order : orders) {
      slave_context_->HandleRtnOrder(order);
    }
  }

  void HandleOrder(const std::shared_ptr<const OrderField>& order) {
    strategy_dispatch_.RtnOrder(order);
  }

  // CTASignal
  void HandleCTASignalInitPosition(
      const std::vector<OrderPosition>& quantitys) {
    master_context_->InitPositions(quantitys);
  }

  void HandleCTASignalHistoryOrder(
      CTASignalAtom cta_signal_atom,
      const std::vector<std::shared_ptr<const OrderField> >& orders) {
    for (const auto& order : orders) {
      master_context_->HandleRtnOrder(order);
    }
  }

  void HandleCTASignalOrder(CTASignalAtom cta_signal_atom,
                            std::shared_ptr<const OrderField>& order) {
    strategy_dispatch_.RtnOrder(order);
  }

  virtual void OpenOrder(const std::string& strategy_id,
                         const std::string& instrument,
                         const std::string& order_id,
                         OrderDirection direction,
                         double price,
                         int quantity) override {}

  virtual void CloseOrder(const std::string& strategy_id,
                          const std::string& instrument,
                          const std::string& order_id,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          double price,
                          int quantity) override {}

  virtual void CancelOrder(const std::string& strategy_id,
                           const std::string& order_id) override {}

 private:
  std::string default_order_exchange_id_;
  std::string master_account_id_;
  std::string slave_account_id_;
  std::shared_ptr<OrdersContext> master_context_;
  std::shared_ptr<OrdersContext> slave_context_;
  std::shared_ptr<CTASignal> signal_;
  std::shared_ptr<CTAGenericStrategy> cta_strategy_;
  std::shared_ptr<CTASignalDispatch> signal_dispatch_;
  StrategyOrderDispatch strategy_dispatch_;
  MailBox* mail_box_;
};

#endif  // FOLLOW_STRATEGY_FOLLOW_STRATEGY_H
