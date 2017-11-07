#ifndef LIVE_TRADE_CTP_RTN_ORDER_SUBSCRIBER_H
#define LIVE_TRADE_CTP_RTN_ORDER_SUBSCRIBER_H
#include "caf/all.hpp"
#include "ctp_trader_api.h"
#include "live_trade_mail_box.h"
#include "caf_atom_defines.h"
#include "follow_strategy/cta_order_signal_subscriber.h"

class CtpRtnOrderSubscriber : public caf::event_based_actor,
                              public CTPTraderApi::Delegate {
 public:
  CtpRtnOrderSubscriber(caf::actor_config& cfg, LiveTradeMailBox* mail_box);

  void Connect(const std::string& server,
               const std::string& broker_id,
               const std::string& user_id,
               const std::string& password);

  virtual caf::behavior make_behavior() override;

  virtual void HandleCTPRtnOrder(
      const std::shared_ptr<CTPOrderField>& order) override;


  template <typename... Ts>
  void Send(Ts&&... args) {
    mail_box_->Send(std::forward<Ts>(args)...);
  }

  template <typename... Ts>
  void Subscribe(Ts...) {}

private:
  CTPTraderApi trade_api_;

  LiveTradeMailBox* mail_box_;

  CTAOrderSignalSubscriber<CtpRtnOrderSubscriber> signal_subscriber_;

};

#endif  // LIVE_TRADE_CTP_RTN_ORDER_SUBSCRIBER_H
