#ifndef FOLLOW_STRATEGY_CTA_ORDER_SIGNAL_SUBSCRIBER_H
#define FOLLOW_STRATEGY_CTA_ORDER_SIGNAL_SUBSCRIBER_H

#include <boost/log/sources/logger.hpp>
#include <boost/log/common.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/unordered_set.hpp>
#include "hpt_core/portfolio.h"
#include "follow_strategy/logging_defines.h"
#include "follow_strategy/string_util.h"
#include "follow_strategy/logging_defines.h"
#include "hpt_core/time_util.h"
#include "order_util.h"

template <typename MailBox>
class CTAOrderSignalSubscriber {
 public:
  CTAOrderSignalSubscriber(MailBox* mail_box, const std::string& instrument)
      : mail_box_(mail_box),
        master_portfolio_(1000000),
        inner_size_portfolio_(1000000) {
    master_portfolio_.InitInstrumentDetail(
        instrument, 0.02, 10, CostBasis{CommissionType::kFixed, 0, 0, 0});
    inner_size_portfolio_.InitInstrumentDetail(
        instrument, 0.02, 10, CostBasis{CommissionType::kFixed, 0, 0, 0});
    mail_box_->Subscribe(&CTAOrderSignalSubscriber::HandleCTASignalInitPosition,
                         this);
    mail_box_->Subscribe(&CTAOrderSignalSubscriber::HandleCTASignalHistoryOrder,
                         this);
    mail_box_->Subscribe(&CTAOrderSignalSubscriber::HandleCTASignalOrder, this);
  }

  // CTASignal
  void HandleCTASignalInitPosition(
      const CTASignalAtom&,
      const std::vector<OrderPosition>& quantitys) {}

  void HandleCTASignalHistoryOrder(
      const CTASignalAtom&,
      const std::vector<std::shared_ptr<const OrderField>>& orders) {}

  void HandleCTASignalOrder(const CTASignalAtom& cta_signal_atom,
                            const std::shared_ptr<OrderField>& order) {
    master_portfolio_.HandleOrder(order);
    HandleRtnOrder(order);
  }

 private:
  void HandleOpening(const std::shared_ptr<OrderField>& rtn_order) {
    OrderDirection opposite_direction =
        OppositeOrderDirection(rtn_order->direction);
    int opposite_closeable_qty = master_portfolio_.GetPositionCloseableQty(
        rtn_order->instrument_id, opposite_direction);
    if (opposite_closeable_qty > 0) {
      if (opposite_closeable_qty < rtn_order->qty) {
        auto close_order = std::make_shared<OrderField>(*rtn_order);
        close_order->order_id = GenerateOrderId();
        close_order->direction = opposite_direction;
        close_order->position_effect = PositionEffect::kClose;
        close_order->qty = opposite_closeable_qty;
        close_order->leaves_qty = opposite_closeable_qty;
        map_order_ids_.insert(std::make_pair(rtn_order->order_id, close_order));
        inner_size_portfolio_.HandleOrder(close_order);
        mail_box_->Send(std::move(close_order),
                        GetCTAPositionQty(close_order->instrument_id,
                                          close_order->direction));

        auto open_order = std::make_shared<OrderField>(*rtn_order);
        open_order->order_id = GenerateOrderId();
        open_order->qty = rtn_order->qty - opposite_closeable_qty;
        open_order->leaves_qty = open_order->qty;
        map_order_ids_.insert(std::make_pair(rtn_order->order_id, open_order));
        inner_size_portfolio_.HandleOrder(open_order);
        mail_box_->Send(std::move(open_order),
                        GetCTAPositionQty(open_order->instrument_id,
                                          open_order->direction));

      } else {
        auto order = std::make_shared<OrderField>(*rtn_order);
        order->order_id = GenerateOrderId();
        order->direction = opposite_direction;
        order->position_effect = PositionEffect::kClose;
        map_order_ids_.insert(std::make_pair(rtn_order->order_id, order));
        inner_size_portfolio_.HandleOrder(order);
        mail_box_->Send(
            std::move(order),
            GetCTAPositionQty(order->instrument_id, order->direction));
      }
    } else {
      auto order = std::make_shared<OrderField>(*rtn_order);
      order->order_id = GenerateOrderId();
      map_order_ids_.insert(std::make_pair(rtn_order->order_id, order));
      inner_size_portfolio_.HandleOrder(std::move(order));
    }
  }

  void HandleCloseing(const std::shared_ptr<OrderField>& rtn_order) {
    auto order = std::make_shared<OrderField>(*rtn_order);
    order->order_id = GenerateOrderId();
    map_order_ids_.insert(std::make_pair(rtn_order->order_id, order));
    inner_size_portfolio_.HandleOrder(order);
    mail_box_->Send(std::move(order), GetCTAPositionQty(order->instrument_id,
                                                       order->direction));
  }

  void HandleOpened(const std::shared_ptr<OrderField>& rtn_order) {
    HandleTradedOrder(rtn_order);
  }


  void HandleClosed(const std::shared_ptr<OrderField>& rtn_order) {
      HandleTradedOrder(rtn_order);
  }

  void HandleCanceled(const std::shared_ptr<OrderField>& rtn_order) {}
  std::string StringifyOrderStatus(OrderStatus status) const {
    std::string ret = "Unkown";
    switch (status) {
      case OrderStatus::kActive:
        ret = "Action";
        break;
      case OrderStatus::kAllFilled:
        ret = "AllFilled";
        break;
      case OrderStatus::kCanceled:
        ret = "Canceled";
        break;
      case OrderStatus::kInputRejected:
        ret = "InputRejected";
        break;
      case OrderStatus::kCancelRejected:
        ret = "CancelRejected";
        break;
      default:
        break;
    }
    return ret;
  }

  std::string StringifyPositionEffect(PositionEffect position_effect) const {
    std::string ret = "Unkown";
    switch (position_effect) {
      case PositionEffect::kUndefine:
        ret = "Undefine";
        break;
      case PositionEffect::kOpen:
        ret = "Open";
        break;
      case PositionEffect::kClose:
        ret = "Close";
        break;
      case PositionEffect::kCloseToday:
        ret = "CloseToday";
        break;
      default:
        break;
    }
    return ret;
  }

  std::string StringifyOrderDirection(OrderDirection order_direction) const {
    std::string ret = "Unkown";
    switch (order_direction) {
      case OrderDirection::kUndefine:
        break;
      case OrderDirection::kBuy:
        ret = "Buy";
        break;
      case OrderDirection::kSell:
        ret = "Sell";
        break;
      default:
        break;
    }
    return ret;
  }

  void HandleRtnOrder(const std::shared_ptr<OrderField>& rtn_order) {
    switch (OrdersContextHandleRtnOrder(rtn_order)) {
      case OrderEventType::kNewOpen:
        HandleOpening(rtn_order);
        break;
      case OrderEventType::kNewClose:
        HandleCloseing(rtn_order);
        break;
      case OrderEventType::kOpenTraded:
        HandleOpened(rtn_order);
        break;
      case OrderEventType::kCloseTraded:
        HandleClosed(rtn_order);
        break;
      case OrderEventType::kCanceled:
        HandleCanceled(rtn_order);
        break;
      default:
        break;
    }
  }

  OrderEventType OrdersContextHandleRtnOrder(
      const std::shared_ptr<OrderField>& order) {
    OrderEventType ret_type = OrderEventType::kIgnore;
    auto it = orders_.find(order);
    if (it == orders_.end()) {
      ret_type = IsCloseOrder(order->position_effect)
                     ? OrderEventType::kNewClose
                     : OrderEventType::kNewOpen;
    } else if (order->status == OrderStatus::kCanceled) {
      ret_type = OrderEventType::kCanceled;
    } else if (order->trading_qty != 0) {
      ret_type = IsCloseOrder(order->position_effect)
                     ? OrderEventType::kCloseTraded
                     : OrderEventType::kOpenTraded;
    }
    orders_.insert(order);
    return ret_type;
  }

  CTAPositionQty GetCTAPositionQty(std::string instrument_id,
                                   OrderDirection direction) {
    return CTAPositionQty{
        inner_size_portfolio_.GetPositionQty(instrument_id, direction),
        inner_size_portfolio_.GetFrozenQty(instrument_id, direction)};
  }

  void HandleTradedOrder(const std::shared_ptr<OrderField>& rtn_order)
  {
    auto range = map_order_ids_.equal_range(rtn_order->order_id);
    int pending_trading_qty = rtn_order->trading_qty;
    for (auto it = range.first; it != range.second; ++it) {
      auto order_copy = std::make_shared<OrderField>(*it->second);
      order_copy->trading_price = rtn_order->trading_price;
      int trading_qty = std::min(pending_trading_qty, order_copy->leaves_qty);
      order_copy->trading_qty = trading_qty;
      order_copy->leaves_qty -= trading_qty;
      order_copy->status = order_copy->leaves_qty == 0 ? OrderStatus::kAllFilled
        : order_copy->status;
      inner_size_portfolio_.HandleOrder(order_copy);
      it->second = order_copy;
      mail_box_->Send(
        std::move(order_copy),
        GetCTAPositionQty(order_copy->instrument_id, order_copy->direction));
      pending_trading_qty -= trading_qty;
      if (pending_trading_qty <= 0) {
        break;
      }
    }

    BOOST_ASSERT(pending_trading_qty == 0);
  }

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

  std::string GenerateOrderId() {
    return boost::lexical_cast<std::string>(order_id_seq_++);
  }

  Portfolio master_portfolio_;
  Portfolio inner_size_portfolio_;
  MailBox* mail_box_;
  std::string master_account_id_;
  int order_id_seq_ = 0;
  boost::unordered_set<std::shared_ptr<const OrderField>,
                       OrderFieldHask,
                       OrderFieldCompare>
      orders_;
  std::multimap<std::string, std::shared_ptr<const OrderField>> map_order_ids_;
};

#endif
