#include "cta_order_signal_subscriber.h"
#include "bft_core/make_message.h"
#include "caf_common/caf_atom_defines.h"
#include "common/api_struct.h"

CTAOrderSignalSubscriber::CTAOrderSignalSubscriber(
    bft::ChannelDelegate* delegate)
    : mail_box_(delegate) {
  exchange_status_ = ExchangeStatus::kNoTrading;
  bft::MessageHandler message_handler;
  message_handler.Subscribe(&CTAOrderSignalSubscriber::HandleExchangeStatus,
                            this);
  message_handler.Subscribe(&CTAOrderSignalSubscriber::HandleRtnOrder, this);
  delegate->Subscribe(std::move(message_handler));
}

void CTAOrderSignalSubscriber::LoggingBindOrderId(
    const std::string& ctp_order_id,
    const std::string& order_id) {
  if (on_process_sync_order_) {
    return;
  }
  BOOST_LOG(log_) << "[Bind OrderId]"
                  << "(CTP)" << ctp_order_id << ", (V)" << order_id;
}

std::string CTAOrderSignalSubscriber::GenerateOrderId() {
  return boost::lexical_cast<std::string>(order_id_seq_++);
}

void CTAOrderSignalSubscriber::Send(std::shared_ptr<OrderField> order,
                                    CTAPositionQty pos_qty) {
  if (on_process_sync_order_) {
    return;
  }
  BOOST_LOG(log_) << "SEND CTA SIGNAL:"
                  << "(ID)" << order->order_id << ",(I)" << order->instrument_id
                  << ", (BS)" << static_cast<int>(order->direction) << ", (OC)"
                  << static_cast<int>(order->direction) << ", (P)"
                  << order->input_price << ", (Qty)" << order->qty << ", (LQty)"
                  << order->leaves_qty << ", (TP)" << order->trading_price
                  << ", (TQty)" << order->trading_qty << ", (Pos)"
                  << pos_qty.position << ", (F)" << pos_qty.frozen;
  mail_box_->Send(bft::MakeMessage(order, pos_qty));
}

void CTAOrderSignalSubscriber::HandleTradedOrder(
    const std::shared_ptr<OrderField>& rtn_order) {
  auto range = map_order_ids_.equal_range(rtn_order->order_id);
  int pending_trading_qty = rtn_order->trading_qty;
  for (auto it = range.first; it != range.second; ++it) {
    if (it->second->leaves_qty == 0) {
      continue;
    }
    auto order_copy = std::make_shared<OrderField>(*it->second);
    order_copy->trading_price = rtn_order->trading_price;
    int trading_qty = std::min(pending_trading_qty, order_copy->leaves_qty);
    order_copy->trading_qty = trading_qty;
    order_copy->leaves_qty -= trading_qty;
    order_copy->status = order_copy->leaves_qty == 0 ? OrderStatus::kAllFilled
                                                     : order_copy->status;
    order_copy->update_timestamp = rtn_order->update_timestamp;
    virtual_porfolio_.HandleOrder(order_copy);
    it->second = order_copy;
    Send(std::move(order_copy),
         GetCTAPositionQty(order_copy->instrument_id,
                           order_copy->position_effect_direction));
    pending_trading_qty -= trading_qty;
    if (pending_trading_qty <= 0) {
      break;
    }
  }

  BOOST_ASSERT(pending_trading_qty == 0);
}

CTAPositionQty CTAOrderSignalSubscriber::GetCTAPositionQty(
    std::string instrument_id,
    OrderDirection direction) {
  return CTAPositionQty{
      virtual_porfolio_.GetPositionQty(instrument_id, direction),
      virtual_porfolio_.GetFrozenQty(instrument_id, direction)};
}

OrderEventType CTAOrderSignalSubscriber::OrdersContextHandleRtnOrder(
    const std::shared_ptr<OrderField>& order) {
  OrderEventType ret_type = OrderEventType::kIgnore;
  auto it = orders_.find(order);
  if (it == orders_.end()) {
    ret_type = IsCloseOrder(order->position_effect) ? OrderEventType::kNewClose
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

void CTAOrderSignalSubscriber::DoHandleRtnOrder(
    const std::shared_ptr<OrderField>& rtn_order) {
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

std::string CTAOrderSignalSubscriber::StringifyOrderDirection(
    OrderDirection order_direction) const {
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

std::string CTAOrderSignalSubscriber::StringifyPositionEffect(
    PositionEffect position_effect) const {
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
    default:
      break;
  }
  return ret;
}

std::string CTAOrderSignalSubscriber::StringifyOrderStatus(
    OrderStatus status) const {
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

void CTAOrderSignalSubscriber::HandleCanceled(
    const std::shared_ptr<OrderField>& rtn_order) {
  auto range = map_order_ids_.equal_range(rtn_order->order_id);
  int pending_trading_qty = rtn_order->trading_qty;
  for (auto it = range.first; it != range.second; ++it) {
    auto order_copy = std::make_shared<OrderField>(*it->second);
    order_copy->trading_price = rtn_order->trading_price;
    order_copy->status = OrderStatus::kCanceled;
    virtual_porfolio_.HandleOrder(order_copy);
    it->second = order_copy;
    Send(std::move(order_copy),
         GetCTAPositionQty(order_copy->instrument_id,
                           order_copy->position_effect_direction));
  }

  BOOST_ASSERT(pending_trading_qty == 0);
}

void CTAOrderSignalSubscriber::HandleClosed(
    const std::shared_ptr<OrderField>& rtn_order) {
  HandleTradedOrder(rtn_order);
}

void CTAOrderSignalSubscriber::HandleOpened(
    const std::shared_ptr<OrderField>& rtn_order) {
  HandleTradedOrder(rtn_order);
}

void CTAOrderSignalSubscriber::HandleCloseing(
    const std::shared_ptr<OrderField>& rtn_order) {
  int opposition_closeable = master_portfolio_.GetPositionCloseableQty(
      rtn_order->instrument_id, rtn_order->direction);
  if (opposition_closeable >= rtn_order->qty) {
    auto order = std::make_shared<OrderField>(*rtn_order);
    order->order_id = GenerateOrderId();
    order->position_effect_direction = rtn_order->direction;
    order->position_effect = PositionEffect::kOpen;
    map_order_ids_.insert(std::make_pair(rtn_order->order_id, order));
    virtual_porfolio_.HandleOrder(order);
    LoggingBindOrderId(rtn_order->order_id, order->order_id);
    if (exchange_status_ == ExchangeStatus::kNoTrading) {
      Send(std::move(order),
           GetCTAPositionQty(rtn_order->instrument_id, rtn_order->direction));
    }
  } else {
    int close = rtn_order->qty - opposition_closeable;
    if (close > 0) {
      auto order = std::make_shared<OrderField>(*rtn_order);
      order->order_id = GenerateOrderId();
      order->qty = close;
      order->leaves_qty = close;
      map_order_ids_.insert(std::make_pair(rtn_order->order_id, order));
      virtual_porfolio_.HandleOrder(order);
      LoggingBindOrderId(rtn_order->order_id, order->order_id);
      Send(std::move(order),
           GetCTAPositionQty(rtn_order->instrument_id,
                             rtn_order->position_effect_direction));
    }
    int open = rtn_order->qty - std::max<int>(close, 0);
    if (open > 0) {
      auto order = std::make_shared<OrderField>(*rtn_order);
      order->order_id = GenerateOrderId();
      order->qty = open;
      order->leaves_qty = open;
      order->position_effect_direction = rtn_order->direction;
      order->position_effect = PositionEffect::kOpen;
      map_order_ids_.insert(std::make_pair(rtn_order->order_id, order));
      virtual_porfolio_.HandleOrder(order);
      LoggingBindOrderId(rtn_order->order_id, order->order_id);
      if (exchange_status_ == ExchangeStatus::kNoTrading) {
        Send(std::move(order),
             GetCTAPositionQty(rtn_order->instrument_id,
                               rtn_order->position_effect_direction));
      }
    }
  }
}

void CTAOrderSignalSubscriber::HandleOpening(
    const std::shared_ptr<OrderField>& rtn_order) {
  OrderDirection opposite_direction =
      OppositeOrderDirection(rtn_order->position_effect_direction);
  int opposite_closeable_qty =
      master_portfolio_.GetPositionCloseableQty(rtn_order->instrument_id,
                                                opposite_direction) -
      master_portfolio_.GetPositionCloseableQty(rtn_order->instrument_id,
                                                rtn_order->direction);
  if (opposite_closeable_qty > 0) {
    if (opposite_closeable_qty < rtn_order->qty) {
      auto close_order = std::make_shared<OrderField>(*rtn_order);
      close_order->order_id = GenerateOrderId();
      close_order->direction = rtn_order->direction;
      close_order->position_effect_direction = opposite_direction;
      close_order->position_effect = PositionEffect::kClose;
      close_order->qty = opposite_closeable_qty;
      close_order->leaves_qty = opposite_closeable_qty;
      map_order_ids_.insert(std::make_pair(rtn_order->order_id, close_order));
      virtual_porfolio_.HandleOrder(close_order);
      LoggingBindOrderId(rtn_order->order_id, close_order->order_id);
      Send(std::move(close_order),
           GetCTAPositionQty(rtn_order->instrument_id, opposite_direction));

      auto open_order = std::make_shared<OrderField>(*rtn_order);
      open_order->order_id = GenerateOrderId();
      open_order->qty = rtn_order->qty - opposite_closeable_qty;
      open_order->leaves_qty = open_order->qty;
      map_order_ids_.insert(std::make_pair(rtn_order->order_id, open_order));
      virtual_porfolio_.HandleOrder(open_order);
      LoggingBindOrderId(rtn_order->order_id, open_order->order_id);
      if (exchange_status_ == ExchangeStatus::kNoTrading) {
        Send(std::move(open_order),
             GetCTAPositionQty(rtn_order->instrument_id, rtn_order->direction));
      }

    } else {
      auto order = std::make_shared<OrderField>(*rtn_order);
      order->order_id = GenerateOrderId();
      order->direction = rtn_order->direction;
      order->position_effect_direction = opposite_direction;
      order->position_effect = PositionEffect::kClose;
      map_order_ids_.insert(std::make_pair(rtn_order->order_id, order));
      virtual_porfolio_.HandleOrder(order);
      LoggingBindOrderId(rtn_order->order_id, order->order_id);
      Send(std::move(order),
           GetCTAPositionQty(rtn_order->instrument_id, opposite_direction));
    }
  } else {
    auto order = std::make_shared<OrderField>(*rtn_order);
    order->order_id = GenerateOrderId();
    map_order_ids_.insert(std::make_pair(rtn_order->order_id, order));
    virtual_porfolio_.HandleOrder(order);
    LoggingBindOrderId(rtn_order->order_id, order->order_id);
    if (exchange_status_ == ExchangeStatus::kNoTrading) {
      Send(std::move(order),
           GetCTAPositionQty(rtn_order->instrument_id,
                             rtn_order->position_effect_direction));
    }
  }
}

std::vector<OrderPosition> CTAOrderSignalSubscriber::GetVirtualPositions()
    const {
  return virtual_porfolio_.GetPositions();
}

void CTAOrderSignalSubscriber::HandleRtnOrder(
    CTASignalAtom cta_signal_atom,
    const std::shared_ptr<OrderField>& order) {
  BOOST_LOG(log_) << "[RECV RtnOrder]"
                  << "(ID)" << order->order_id << ",(I)" << order->instrument_id
                  << ", (BS)" << static_cast<int>(order->direction) << ", (OC)"
                  << static_cast<int>(order->direction) << ", (P)"
                  << order->input_price << ", (Qty)" << order->qty << ", (LQty)"
                  << order->leaves_qty << ", (TP)" << order->trading_price
                  << ", (TQty)" << order->trading_qty;
  master_portfolio_.HandleOrder(order);
  DoHandleRtnOrder(order);
}

void CTAOrderSignalSubscriber::HandleExchangeStatus(
    ExchangeStatus exchange_status) {
  BOOST_LOG(log_) << "[RECV Exchange Status]"
                  << (exchange_status == ExchangeStatus::kNoTrading
                          ? "NoTrading"
                          : "Continous");
  exchange_status_ = exchange_status;
}

void CTAOrderSignalSubscriber::HandleSyncHistoryRtnOrder(
    CTASignalAtom,
    const std::shared_ptr<OrderField>& order) {
  on_process_sync_order_ = true;
  master_portfolio_.HandleOrder(order);
  DoHandleRtnOrder(order);
  on_process_sync_order_ = false;
}

void CTAOrderSignalSubscriber::HandleSyncYesterdayPosition(
    CTASignalAtom,
    const std::vector<OrderPosition>& positions) {
  std::set<std::string> instruments;
  for (const auto& position : positions) {
    master_portfolio_.AddPosition(position.instrument, position.order_direction,
                                  position.quantity);
    instruments.insert(position.instrument);
  }

  for (const auto& ins : instruments) {
    int long_pos = master_portfolio_.GetPositionQty(ins, OrderDirection::kBuy);
    int short_pos =
        master_portfolio_.GetPositionQty(ins, OrderDirection::kSell);
    if ((long_pos == 0 && short_pos == 0) && (long_pos == short_pos)) {
      continue;
    }
    virtual_porfolio_.AddPosition(
        ins,
        long_pos > short_pos ? OrderDirection::kBuy : OrderDirection::kSell,
        abs(long_pos - short_pos));
  }
}
