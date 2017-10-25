#ifndef FOLLOW_STRATEGY_CTA_TRADED_SIGNAL_STRATEGY_H
#define FOLLOW_STRATEGY_CTA_TRADED_SIGNAL_STRATEGY_H

template <class MailBox>
class CTATradedSignalStrategy : public StrategyEnterOrderObservable::Observer {
 public:
  CTATradedSignalStrategy(MailBox* mail_box)
      : mail_box_(mail_box) {
    mail_box_->Subscribe(&CTATradedSignalStrategy::HandleCTASignalOrder, this);
    mail_box_->Subscribe(&CTATradedSignalStrategy::BeforeTrading, this);
    mail_box_->Subscribe(&CTATradedSignalStrategy::BeforeCloseMarket, this);
    mail_box_->Subscribe(&CTATradedSignalStrategy::HandleTick, this);
  }

  void BeforeTrading(const BeforeTradingAtom&,
                     const TradingTime& trading_time) {

  }

  void BeforeCloseMarket(const BeforeCloseMarketAtom&) {

  }

  void HandleTick(const std::shared_ptr<TickData>& tick) {

  }

  // CTASignal
  void HandleCTASignalInitPosition(
      const CTASignalAtom&,
      const std::vector<OrderPosition>& quantitys) {
  }

  void HandleCTASignalOrder(const CTASignalAtom& cta_signal_atom,
                            const std::shared_ptr<OrderField>& order) {
  }

private:
  MailBox* mail_box_;
};
#endif  // FOLLOW_STRATEGY_CTA_TRADED_SIGNAL_STRATEGY_H
