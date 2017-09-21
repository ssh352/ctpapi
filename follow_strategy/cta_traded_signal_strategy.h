#ifndef FOLLOW_STRATEGY_CTA_TRADED_SIGNAL_STRATEGY_H
#define FOLLOW_STRATEGY_CTA_TRADED_SIGNAL_STRATEGY_H

template <class MailBox>
class CTATradedSignalStrategy : public StrategyEnterOrderObservable::Observer {
 public:
  CTATradedSignalStrategy(MailBox* mail_box,
                          std::string master_account_id,
                          std::string slave_account_id,
                          int delayed_open_order)
      : mail_box_(mail_box),
        master_account_id_(master_account_id),
        slave_account_id_(slave_account_id) {
    master_context_ = std::make_shared<OrdersContext>(master_account_id_);
    slave_context_ = std::make_shared<OrdersContext>(slave_account_id_);
    cta_strategy_ = std::make_shared<CTAGenericStrategy>(slave_account_id);
    signal_ = std::make_shared<CTASignal>(delayed_open_order);
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
    mail_box_->Subscribe(&FollowStrategy::BeforeTrading, this);
    mail_box_->Subscribe(&FollowStrategy::BeforeCloseMarket, this);
    mail_box_->Subscribe(&FollowStrategy::HandleTick, this);
  }

  void BeforeTrading(const BeforeTradingAtom&,
                     const TradingTime& trading_time) {
    master_context_->Reset();
    slave_context_->Reset();
  }

  void BeforeCloseMarket(const BeforeCloseMarketAtom&) {
    signal_->BeforeCloseMarket();
  }

  void HandleTick(const std::shared_ptr<TickData>& tick) {
    signal_->HandleTick(tick);
  }

  void HandleInitPosition(const std::vector<OrderPosition>& quantitys) {
    slave_context_->InitPositions(quantitys);
  }

  void HandleOrder(const std::shared_ptr<OrderField>& order) {
    strategy_dispatch_.RtnOrder(order);
  }

  void HandleHistoryOrder(
      const std::vector<std::shared_ptr<const OrderField> >& orders) {
    for (const auto& order : orders) {
      slave_context_->HandleRtnOrder(order);
    }
  }
  // CTASignal
  void HandleCTASignalInitPosition(
      const CTASignalAtom&,
      const std::vector<OrderPosition>& quantitys) {
    master_context_->InitPositions(quantitys);
  }

  void HandleCTASignalHistoryOrder(
      const CTASignalAtom&,
      const std::vector<std::shared_ptr<const OrderField> >& orders) {
    for (const auto& order : orders) {
      master_context_->HandleRtnOrder(order);
    }
  }

  void HandleCTASignalOrder(const CTASignalAtom& cta_signal_atom,
                            const std::shared_ptr<OrderField>& order) {
    strategy_dispatch_.RtnOrder(order);
  }

  virtual void OpenOrder(const std::string& strategy_id,
                         const std::string& instrument,
                         const std::string& order_id,
                         OrderDirection direction,
                         double price,
                         int quantity) override {
    mail_box_->Send(InputOrderSignal{instrument, order_id, strategy_id,
                                     PositionEffect::kOpen, direction, price,
                                     quantity, 0});
  }

  virtual void CloseOrder(const std::string& strategy_id,
                          const std::string& instrument,
                          const std::string& order_id,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          double price,
                          int quantity) override {
    mail_box_->Send(InputOrderSignal{instrument, order_id, strategy_id,
                                     position_effect, direction, price,
                                     quantity, 0});
  }

  virtual void CancelOrder(const std::string& strategy_id,
                           const std::string& order_id) override {
    mail_box_->Send(CancelOrderSignal{order_id});
  }

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
#endif  // FOLLOW_STRATEGY_CTA_TRADED_SIGNAL_STRATEGY_H
