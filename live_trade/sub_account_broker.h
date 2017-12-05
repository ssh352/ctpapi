#ifndef LIVE_TRADE_SUB_ACCOUNT_BROKER_H
#define LIVE_TRADE_SUB_ACCOUNT_BROKER_H
#include <boost/assert.hpp>
#include <memory>
#include "caf/all.hpp"
#include "ctp_broker/ctp_order_delegate.h"
#include "live_trade_mail_box.h"
#include "ctp_broker/ctp_instrument_broker.h"

class CAFSubAccountBroker : public caf::event_based_actor,
                            public CTPOrderDelegate {
 public:
  CAFSubAccountBroker(caf::actor_config& cfg,
                      LiveTradeMailBox* inner_mail_box,
                      LiveTradeMailBox* common_mail_box,
    std::unordered_set<std::string> close_today_cost_of_product_codes,
                      std::string account_id);

  virtual caf::behavior make_behavior() override;

  void MakeCtpInstrumentBrokerIfNeed(const std::string& instrument);

  virtual void HandleEnterOrder(const CTPEnterOrder& enter_order) override;

  virtual void HandleCancelOrder(const CTPCancelOrder& cancel_order) override;

  virtual void ReturnOrderField(
      const std::shared_ptr<OrderField>& order) override;

 private:
  std::string GenerateOrderId();
  caf::actor ctp_actor_;
  LiveTradeMailBox* inner_mail_box_;
  LiveTradeMailBox* common_mail_box_;
  std::unordered_map<std::string, std::unique_ptr<CTPInstrumentBroker>>
      instrument_brokers_;
  std::unordered_set<std::string> close_today_cost_of_product_codes_;
  std::string account_id_;
  int order_seq_ = 0;
};

#endif  // LIVE_TRADE_SUB_ACCOUNT_BROKER_H
