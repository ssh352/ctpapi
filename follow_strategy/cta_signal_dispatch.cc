#include "cta_signal_dispatch.h"
#include "order_util.h"

CTASignalDispatch::CTASignalDispatch(CTASignalObserver* signal_observer,
                                     std::string slave_account_id)
    : signal_observer_(signal_observer),
      slave_account_id_(std::move(slave_account_id)) {}

void CTASignalDispatch::SubscribeEnterOrderObserver(
    EnterOrderObserver* observer) {
  enter_order_observer_ = observer;
}

void CTASignalDispatch::RtnOrder(
    const std::shared_ptr<const OrderField>& rtn_order) {
  switch (BeforeHandleOrder(rtn_order)) {
    case StragetyStatus::kWaitReply:
      outstanding_orders_.push_back(rtn_order);
      break;
    case StragetyStatus::kReady:
      DoHandleRtnOrder(rtn_order);
      break;
    case StragetyStatus::kSkip:
      break;
    default:
      break;
  }

  if (portfolio_observer_ != NULL) {
    // portfolio_observer_->Notify(slave_context_->GetAccountPortfolios());
  }
}

void CTASignalDispatch::DoHandleRtnOrder(
    const std::shared_ptr<const OrderField>& rtn_order) {
  switch (OrdersContextHandleRtnOrder(rtn_order)) {
    case OrderEventType::kNewOpen:
      signal_observer_->HandleOpening(rtn_order);
      break;
    case OrderEventType::kNewClose:
      signal_observer_->HandleCloseing(rtn_order);
      break;
    case OrderEventType::kOpenTraded:
      signal_observer_->HandleOpened(rtn_order);
      break;
    case OrderEventType::kCloseTraded:
      signal_observer_->HandleClosed(rtn_order);
      break;
    case OrderEventType::kCanceled:
      signal_observer_->HandleCanceled(rtn_order);
      break;
    default:
      break;
  }
}

void CTASignalDispatch::Trade(const std::string& order_id, OrderStatus status) {
  waiting_reply_order_.emplace_back(order_id, status);
}

void CTASignalDispatch::OpenOrder(const std::string& instrument,
                                  const std::string& order_id,
                                  OrderDirection direction,
                                  double price,
                                  int quantity) {
  Trade(order_id, OrderStatus::kActive);
  if (enter_order_observer_ != nullptr) {
    enter_order_observer_->OpenOrder(instrument, order_id, direction, price,
                                     quantity);
  }
}

void CTASignalDispatch::CloseOrder(const std::string& instrument,
                                   const std::string& order_id,
                                   OrderDirection direction,
                                   PositionEffect position_effect,
                                   double price,
                                   int quantity) {
  Trade(order_id, OrderStatus::kActive);
  if (enter_order_observer_ != nullptr) {
    enter_order_observer_->CloseOrder(instrument, order_id, direction,
                                      position_effect, price, quantity);
  }
}

void CTASignalDispatch::CancelOrder(const std::string& order_id) {
  Trade(order_id, OrderStatus::kCanceled);

  if (enter_order_observer_ != nullptr) {
    enter_order_observer_->CancelOrder(order_id);
  }
}

void CTASignalDispatch::SubscribePortfolioObserver(
    std::shared_ptr<PortfolioObserver> observer) {
  portfolio_observer_ = observer;
}

CTASignalDispatch::StragetyStatus CTASignalDispatch::BeforeHandleOrder(
    const std::shared_ptr<const OrderField>& order) {
  StragetyStatus status = waiting_reply_order_.empty()
                              ? StragetyStatus::kReady
                              : StragetyStatus::kWaitReply;
  if (!waiting_reply_order_.empty() &&
      order->strategy_id == slave_account_id_) {
    status = StragetyStatus::kReady;
    auto it =
        std::find_if(waiting_reply_order_.begin(), waiting_reply_order_.end(),
                     [&](auto i) { return i.first == order->order_id; });

    if (it != waiting_reply_order_.end()) {
      if (it->second == order->status ||
          (it->second == OrderStatus::kCanceled &&
           order->status == OrderStatus::kCancelRejected) ||
          order->status == OrderStatus::kAllFilled) {
        waiting_reply_order_.erase(it);
        DoHandleRtnOrder(order);
        status = StragetyStatus::kSkip;
        if (waiting_reply_order_.empty()) {
          while (!outstanding_orders_.empty() && waiting_reply_order_.empty()) {
            DoHandleRtnOrder(outstanding_orders_.front());
            outstanding_orders_.pop_front();
          }
        }
      }
    }
  }
  return status;
}

OrderEventType CTASignalDispatch::OrdersContextHandleRtnOrder(
    const std::shared_ptr<const OrderField>& order) {
  std::unordered_set<std::shared_ptr<OrderField>> slave_orders_;
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
