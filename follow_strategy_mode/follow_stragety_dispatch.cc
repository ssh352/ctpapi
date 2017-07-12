#include "follow_stragety_dispatch.h"

FollowStragetyDispatch::FollowStragetyDispatch(
    const std::string& master_account,
    const std::string& slave_account,
    std::shared_ptr<BaseFollowStragety> stragety,
    std::shared_ptr<Delegate> delegate,
    std::shared_ptr<OrdersContext> master_context,
    std::shared_ptr<OrdersContext> slave_context)
    : master_account_(master_account),
      slave_account_(slave_account),
      stragety_(stragety),
      delegate_(delegate),
      master_context_(master_context),
      slave_context_(slave_context) {}

void FollowStragetyDispatch::HandleRtnOrder(OrderData rtn_order) {
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

void FollowStragetyDispatch::DoHandleRtnOrder(OrderData rtn_order) {
  switch (OrdersContextHandleRtnOrder(rtn_order)) {
    case OrderEventType::kNewOpen:
      stragety_->HandleOpening(rtn_order);
      break;
    case OrderEventType::kNewClose:
      stragety_->HandleCloseing(rtn_order);
      break;
    case OrderEventType::kOpenTraded:
      stragety_->HandleOpened(rtn_order);
      break;
    case OrderEventType::kCloseTraded:
      stragety_->HandleClosed(rtn_order);
      break;
    case OrderEventType::kCanceled:
      stragety_->HandleCanceled(rtn_order);
      break;
    default:
      break;
  }
}

void FollowStragetyDispatch::Trade(const std::string& order_no,
                                   OrderStatus status) {
  waiting_reply_order_.emplace_back(order_no, status);
}

void FollowStragetyDispatch::OpenOrder(const std::string& instrument,
                                       const std::string& order_no,
                                       OrderDirection direction,
                                       OrderPriceType price_type,
                                       double price,
                                       int quantity) {
  Trade(order_no, OrderStatus::kActive);
  if (delegate_ != nullptr) {
    delegate_->OpenOrder(instrument, order_no, direction, price_type, price,
                         quantity);
  }
}
void FollowStragetyDispatch::CloseOrder(const std::string& instrument,
                                        const std::string& order_no,
                                        OrderDirection direction,
                                        PositionEffect position_effect,
                                        OrderPriceType price_type,
                                        double price,
                                        int quantity) {
  Trade(order_no, OrderStatus::kActive);
  if (delegate_ != nullptr) {
    delegate_->CloseOrder(instrument, order_no, direction, position_effect,
                          price_type, price, quantity);
  }
}

const std::string& FollowStragetyDispatch::master_account_id() const {
  return master_account_;
}

const std::string& FollowStragetyDispatch::slave_account_id() const {
  return slave_account_;
}

void FollowStragetyDispatch::CancelOrder(const std::string& order_no) {
  Trade(order_no, OrderStatus::kCancel);

  if (delegate_ != nullptr) {
    delegate_->CancelOrder(order_no);
  }
}

FollowStragetyDispatch::StragetyStatus
FollowStragetyDispatch::BeforeHandleOrder(OrderData order) {
  StragetyStatus status = waiting_reply_order_.empty()
                              ? StragetyStatus::kReady
                              : StragetyStatus::kWaitReply;
  if (!waiting_reply_order_.empty() && order.account_id() == slave_account_) {
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

OrderEventType FollowStragetyDispatch::OrdersContextHandleRtnOrder(
    OrderData order) {
  return order.account_id() == master_account_
             ? master_context_->HandleRtnOrder(order)
             : slave_context_->HandleRtnOrder(order);
}
