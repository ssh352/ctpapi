#ifndef STRATEGIES_DELAYED_OPEN_STRATEGY_H
#define STRATEGIES_DELAYED_OPEN_STRATEGY_H
#include "common/api_struct.h"

template <typename MailBox>
class DelayedOpenStrategy {
 public:
  DelayedOpenStrategy(MailBox* mail_box, int delayed_open_of_seconds)
      : mail_box_(mail_box), delayed_open_of_seconds_(delayed_open_of_seconds) {
    mail_box_->Subscribe(&DelayedOpenStrategy::HandleCTAPosition, this);
    mail_box_->Subscribe(&DelayedOpenStrategy::HandleCTASyncHistoryOrder, this);
    mail_box_->Subscribe(&DelayedOpenStrategy::HandleCTAOrder, this);
  }

 private:
  void HandleCTAPosition(
      CTASignalAtom,
      const std::shared_ptr<std::vector<InvestorPositionField> >& positions) {}

  void HandleCTASyncHistoryOrder(
      CTASignalAtom,
      const std::shared_ptr<std::vector<std::shared_ptr<OrderField> > >&
          history_orders) {}

  void HandleCTAOrder(CTASignalAtom, const std::shared_ptr<OrderField>& order) {
  }

  MailBox* mail_box_;
  int delayed_open_of_seconds_;
};

#endif  // STRATEGIES_DELAYED_OPEN_STRATEGY_H
