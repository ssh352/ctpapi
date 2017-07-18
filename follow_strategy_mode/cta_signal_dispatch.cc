#include "cta_signal_dispatch.h"

CTASignalDispatch::CTASignalDispatch(
    std::shared_ptr<CTASignalObserver> signal_observer)
    : signal_observer_(signal_observer) {}

void CTASignalDispatch::SubscribeEnterOrderObserver(
    std::shared_ptr<EnterOrderObserver> observer) {
  enter_order_observer_ = observer;
}

// CTASignalDispatch::CTASignalDispatch(
//     const std::string& master_account,
//     const std::string& slave_account,
//     std::shared_ptr<BaseFollowStragety> stragety,
//     std::shared_ptr<Delegate> delegate,
//     std::shared_ptr<OrdersContext> master_context,
//     std::shared_ptr<OrdersContext> slave_context)
//     : master_account_(master_account),
//       slave_account_(slave_account),
//       stragety_(stragety),
//       enter_order_observer_(delegate),
//       master_context_(master_context),
//       slave_context_(slave_context) {}

void CTASignalDispatch::RtnOrder(OrderData rtn_order) {
  switch (BeforeHandleOrder(rtn_order)) {
    case StragetyStatus::kWaitReply:
      outstanding_orders_.push_back(std::move(rtn_order));
      break;
    case StragetyStatus::kReady:
      DoHandleRtnOrder(std::move(rtn_order));
      break;
    case StragetyStatus::kSkip:
      break;
    default:
      break;
  }
}

void CTASignalDispatch::DoHandleRtnOrder(OrderData rtn_order) {
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

void CTASignalDispatch::Trade(const std::string& order_no, OrderStatus status) {
  waiting_reply_order_.emplace_back(order_no, status);
}

void CTASignalDispatch::OpenOrder(const std::string& instrument,
                                  const std::string& order_no,
                                  OrderDirection direction,
                                  OrderPriceType price_type,
                                  double price,
                                  int quantity) {
  Trade(order_no, OrderStatus::kActive);
  if (enter_order_observer_ != nullptr) {
    enter_order_observer_->OpenOrder(instrument, order_no, direction,
                                     price_type, price, quantity);
  }
}
void CTASignalDispatch::CloseOrder(const std::string& instrument,
                                   const std::string& order_no,
                                   OrderDirection direction,
                                   PositionEffect position_effect,
                                   OrderPriceType price_type,
                                   double price,
                                   int quantity) {
  Trade(order_no, OrderStatus::kActive);
  if (enter_order_observer_ != nullptr) {
    enter_order_observer_->CloseOrder(instrument, order_no, direction,
                                      position_effect, price_type, price,
                                      quantity);
  }
}

void CTASignalDispatch::CancelOrder(const std::string& order_no) {
  Trade(order_no, OrderStatus::kCancel);

  if (enter_order_observer_ != nullptr) {
    enter_order_observer_->CancelOrder(order_no);
  }
}

void CTASignalDispatch::SetOrdersContext(
    std::shared_ptr<OrdersContext> master_context,
    std::shared_ptr<OrdersContext> slave_context) {
  master_context_ = master_context;
  slave_context_ = slave_context;
}

CTASignalDispatch::StragetyStatus CTASignalDispatch::BeforeHandleOrder(
    OrderData order) {
  StragetyStatus status = waiting_reply_order_.empty()
                              ? StragetyStatus::kReady
                              : StragetyStatus::kWaitReply;
  if (!waiting_reply_order_.empty() &&
      order.account_id() == slave_context_->account_id()) {
    auto it =
        std::find_if(waiting_reply_order_.begin(), waiting_reply_order_.end(),
                     [&](auto i) { return i.first == order.order_id(); });
    if (it != waiting_reply_order_.end()) {
      if (it->second == order.status() ||
          order.status() == OrderStatus::kAllFilled) {
        waiting_reply_order_.erase(it);
        DoHandleRtnOrder(order);
        status = StragetyStatus::kSkip;
        if (waiting_reply_order_.empty()) {
          while (!outstanding_orders_.empty() && waiting_reply_order_.empty()) {
            DoHandleRtnOrder(outstanding_orders_.front());
            outstanding_orders_.pop_front();
          }
        }
      } else {
        status = StragetyStatus::kReady;
      }
    } else {
      status = StragetyStatus::kWaitReply;
    }
  }
  return status;
}

OrderEventType CTASignalDispatch::OrdersContextHandleRtnOrder(OrderData order) {
  return order.account_id() == master_context_->account_id()
             ? master_context_->HandleRtnOrder(order)
             : slave_context_->HandleRtnOrder(order);
}
