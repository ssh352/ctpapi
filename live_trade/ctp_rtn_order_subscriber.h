#ifndef LIVE_TRADE_CTP_RTN_ORDER_SUBSCRIBER_H
#define LIVE_TRADE_CTP_RTN_ORDER_SUBSCRIBER_H
#include "caf/all.hpp"
#include "ctp_trader_api.h"

#include "caf_common/caf_atom_defines.h"
#include "follow_strategy/cta_order_signal_subscriber.h"
#include "bft_core/channel_delegate.h"
#include "bft_core/message.h"
#include "bft_core/message_handler.h"
#include "live_trade_system.h"

class CAFCTAOrderSignalBroker : public caf::event_based_actor,
                                public CTPTraderApi::Delegate,
                                public bft::ChannelDelegate {
 public:
  CAFCTAOrderSignalBroker(caf::actor_config& cfg,
                          LiveTradeSystem* live_trade_system,
                          int env_id);

  void Connect(const std::string& server,
               const std::string& broker_id,
               const std::string& user_id,
               const std::string& password);

  virtual caf::behavior make_behavior() override;

  virtual void HandleCTPRtnOrder(
      const std::shared_ptr<CTPOrderField>& order) override;

  virtual void HandleCTPTradeOrder(const std::string& instrument,
                                   const std::string& order_id,
                                   double trading_price,
                                   int trading_qty,
                                   TimeStamp timestamp) override;

  virtual void HandleCtpLogon(int front_id, int session_id) override;

  virtual void HandleRspYesterdayPosition(
      std::vector<OrderPosition> yesterday_positions) override;

  virtual void HandleExchangeStatus(ExchangeStatus exchange_status) override;

  virtual void Subscribe(
      std::unique_ptr<bft::BasedMessageHandler> handler) override;

  virtual void Send(std::shared_ptr<bft::Message> message) override;

 private:
  struct HashCTPOrderField {
    size_t operator()(const std::shared_ptr<CTPOrderField>& order) const {
      return std::hash<std::string>()(order->order_id);
    }
    size_t operator()(const std::string& order_id) const {
      return std::hash<std::string>()(order_id);
    }
  };

  struct CompareCTPOrderField {
    size_t operator()(const std::shared_ptr<CTPOrderField>& l,
                      const std::shared_ptr<CTPOrderField>& r) const {
      return l->order_id == r->order_id;
    }

    size_t operator()(const std::string& l,
                      const std::shared_ptr<CTPOrderField>& r) const {
      return l == r->order_id;
    }
  };

  std::shared_ptr<OrderField> MakeOrderField(
      const std::shared_ptr<CTPOrderField>& ctp_order,
      double trading_price,
      int trading_qty,
      TimeStamp timestamp = 0) const;
  void LoggingVirtualPositions();
  std::unique_ptr<CTPTraderApi> trade_api_;

  CTAOrderSignalSubscriber signal_subscriber_;

  boost::unordered_set<std::shared_ptr<CTPOrderField>,
                       HashCTPOrderField,
                       CompareCTPOrderField>
      ctp_orders_;
  int sync_rtn_order_count_ = 0;
  LiveTradeSystem* live_trade_system_;
  int env_id_;
};

#endif  // LIVE_TRADE_CTP_RTN_ORDER_SUBSCRIBER_H
