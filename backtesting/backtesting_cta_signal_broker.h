#ifndef BACKTESTING_BACKTESTING_CTA_SIGNAL_BROKER_H
#define BACKTESTING_BACKTESTING_CTA_SIGNAL_BROKER_H

template <class MailBox>
class BacktestingCTASignalBroker {
 public:
  BacktestingCTASignalBroker() : mail_box_(mail_box) {}

 private:
  MailBox* mail_box_;
};

#endif  // BACKTESTING_BACKTESTING_CTA_SIGNAL_BROKER_H
