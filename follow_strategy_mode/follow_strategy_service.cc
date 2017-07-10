#include "follow_strategy_service.h"

FollowStragetyService::FollowStragetyService(
    std::shared_ptr<BaseFollowStragetyFactory> stragety_factory,
    const std::string& master_account,
    const std::string& slave_account,
    std::shared_ptr<Delegate> delegate)
    : master_account_(master_account),
      slave_account_(slave_account),
      delegate_(delegate) {
  stragety_.reset(stragety_factory->Create(master_account, slave_account_, this,
                                           &context_));
}

void FollowStragetyService::InitPositions(
    const std::string& account_id,
    std::vector<OrderPosition> positions) {
  context_.InitPositions(account_id, std::move(positions));
}

void FollowStragetyService::InitRtnOrders(std::vector<OrderData> orders) {
  for (auto order : orders) {
    (void)context_.HandlertnOrder(order);
  }
}

void FollowStragetyService::HandleRtnOrder(OrderData rtn_order) {
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

void FollowStragetyService::DoHandleRtnOrder(OrderData rtn_order) {
  switch (context_.HandlertnOrder(rtn_order)) {
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

void FollowStragetyService::Trade(const std::string& order_no,
                                  OrderStatus status) {
  waiting_reply_order_.emplace_back(order_no, status);
}

void FollowStragetyService::OpenOrder(const std::string& instrument,
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
void FollowStragetyService::CloseOrder(const std::string& instrument,
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

const Context& FollowStragetyService::context() const {
  return context_;
}

const std::string& FollowStragetyService::master_account_id() const {
  return master_account_;
}

const std::string& FollowStragetyService::slave_account_id() const {
  return slave_account_;
}

void FollowStragetyService::CancelOrder(const std::string& order_no) {
  Trade(order_no, OrderStatus::kCancel);

  if (delegate_ != nullptr) {
    delegate_->CancelOrder(order_no);
  }
}

FollowStragetyService::StragetyStatus FollowStragetyService::BeforeHandleOrder(
    OrderData order) {
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
