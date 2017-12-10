#ifndef FOLLOW_STRATEGY_CTA_ORDER_SIGNAL_SUBSCRIBER_H
#define FOLLOW_STRATEGY_CTA_ORDER_SIGNAL_SUBSCRIBER_H

#include <boost/log/sources/logger.hpp>
#include <boost/log/common.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/unordered_set.hpp>
#include "hpt_core/portfolio.h"
#include "follow_strategy/logging_defines.h"
#include "follow_strategy/string_util.h"
#include "hpt_core/time_util.h"
#include "hpt_core/order_util.h"
#include "hpt_core/simply_portfolio.h"
#include "bft_core/channel_delegate.h"
#include "common/api_struct.h"


class CTAOrderSignalSubscriber  {
 public:
  CTAOrderSignalSubscriber(bft::ChannelDelegate* delegate);
  // CTASignal
  void HandleSyncYesterdayPosition(const CTASignalAtom&,
                                   const std::vector<OrderPosition>& positions);

  void HandleSyncHistoryRtnOrder(const CTASignalAtom&,
                                 const std::shared_ptr<OrderField>& order);

  void HandleExchangeStatus(ExchangeStatus exchange_status);

  void HandleRtnOrder(const CTASignalAtom& cta_signal_atom,
                      const std::shared_ptr<OrderField>& order);

  std::vector<OrderPosition> GetVirtualPositions() const;

 private:
  void HandleOpening(const std::shared_ptr<OrderField>& rtn_order);

  void HandleCloseing(const std::shared_ptr<OrderField>& rtn_order);

  void HandleOpened(const std::shared_ptr<OrderField>& rtn_order);

  void HandleClosed(const std::shared_ptr<OrderField>& rtn_order);

  void HandleCanceled(const std::shared_ptr<OrderField>& rtn_order);
  std::string StringifyOrderStatus(OrderStatus status) const;

  std::string StringifyPositionEffect(PositionEffect position_effect) const;

  std::string StringifyOrderDirection(OrderDirection order_direction) const;

  void DoHandleRtnOrder(const std::shared_ptr<OrderField>& rtn_order);

  OrderEventType OrdersContextHandleRtnOrder(
      const std::shared_ptr<OrderField>& order);

  CTAPositionQty GetCTAPositionQty(std::string instrument_id,
                                   OrderDirection direction);

  void HandleTradedOrder(const std::shared_ptr<OrderField>& rtn_order);

  void Send(std::shared_ptr<OrderField> order, CTAPositionQty pos_qty);

  struct OrderFieldHask {
    using is_transparent = void;
    size_t operator()(const std::shared_ptr<const OrderField>& order) const {
      return std::hash<std::string>()(order->order_id);
    }
    size_t operator()(const std::string& order_id) const {
      return std::hash<std::string>()(order_id);
    }
  };

  struct OrderFieldCompare {
    using is_transparent = void;
    bool operator()(const std::shared_ptr<const OrderField>& l,
                    const std::shared_ptr<const OrderField>& r) const {
      return l->order_id == r->order_id;
    }

    bool operator()(const std::string& order_id,
                    const std::shared_ptr<const OrderField>& r) const {
      return order_id == r->order_id;
    }

    bool operator()(const std::shared_ptr<const OrderField>& l,
                    const std::string& order_id) const {
      return order_id == l->order_id;
    }
  };

  std::string GenerateOrderId();

  void LoggingBindOrderId(const std::string& ctp_order_id,
                          const std::string& order_id);
  class MailBox { 
  public:
    template <typename... Ts>
    void Subscribe(Ts&&...) {}

    template <typename... Ts>
    void Send(Ts&&...) {}
  };

  SimplyPortfolio master_portfolio_;
  SimplyPortfolio virtual_porfolio_;
  bft::ChannelDelegate* mail_box_;
  std::string master_account_id_;
  int order_id_seq_ = 0;
  boost::unordered_set<std::shared_ptr<const OrderField>,
                       OrderFieldHask,
                       OrderFieldCompare>
      orders_;
  std::multimap<std::string, std::shared_ptr<const OrderField>> map_order_ids_;
  bool on_process_sync_order_ = false;
  boost::log::sources::logger log_;
  ExchangeStatus exchange_status_;
};

#endif
