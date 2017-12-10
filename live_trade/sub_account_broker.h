#ifndef LIVE_TRADE_SUB_ACCOUNT_BROKER_H
#define LIVE_TRADE_SUB_ACCOUNT_BROKER_H
#include <memory>
#include <boost/assert.hpp>
#include <boost/log/sources/logger.hpp>
#include "caf/all.hpp"
#include "ctp_broker/ctp_order_delegate.h"

#include "ctp_broker/ctp_instrument_broker.h"
#include "follow_strategy/product_info_manager.h"
#include "live_trade_system.h"

class CAFSubAccountBroker : public caf::event_based_actor,
                            public CTPOrderDelegate {
 public:
  CAFSubAccountBroker(
      caf::actor_config& cfg,
      LiveTradeSystem* live_trade_system,
      ProductInfoMananger* product_info_mananger,
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
  LiveTradeSystem* live_trade_system_;
  std::unordered_map<std::string, std::unique_ptr<CTPInstrumentBroker>>
      instrument_brokers_;
  std::unordered_set<std::string> close_today_cost_of_product_codes_;
  std::string account_id_;
  int order_seq_ = 0;
  ProductInfoMananger* product_info_mananger_;
  boost::log::sources::logger log_;
  int env_id_;
};

#endif  // LIVE_TRADE_SUB_ACCOUNT_BROKER_H
