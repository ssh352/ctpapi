#include "cta_signal_dispatch.h"

CTASignalDispatch::CTASignalDispatch(
    std::shared_ptr<CTASignalObserver> signal_observer)
    : signal_observer_(signal_observer) {
  signal_observer_->Subscribe(this);
}

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

void CTASignalDispatch::RtnOrder(
    const boost::shared_ptr<const OrderField>& rtn_order) {
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
    portfolio_observer_->Notify(slave_context_->GetAccountPortfolios());
  }
}

void CTASignalDispatch::DoHandleRtnOrder(
    const boost::shared_ptr<const OrderField>& rtn_order) {
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

void CTASignalDispatch::SetOrdersContext(
    std::shared_ptr<OrdersContext> master_context,
    std::shared_ptr<OrdersContext> slave_context) {
  master_context_ = master_context;
  slave_context_ = slave_context;
}

void CTASignalDispatch::SubscribePortfolioObserver(
    std::shared_ptr<PortfolioObserver> observer) {
  portfolio_observer_ = observer;
}

CTASignalDispatch::StragetyStatus CTASignalDispatch::BeforeHandleOrder(
    const boost::shared_ptr<const OrderField>& order) {
  StragetyStatus status = waiting_reply_order_.empty()
                              ? StragetyStatus::kReady
                              : StragetyStatus::kWaitReply;
  if (!waiting_reply_order_.empty() &&
      order->account_id == slave_context_->account_id()) {
    auto it =
        std::find_if(waiting_reply_order_.begin(), waiting_reply_order_.end(),
                     [&](auto i) { return i.first == order->order_id; });
    if (it != waiting_reply_order_.end()) {
      if (it->second == order->status ||
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
      } else {
        status = StragetyStatus::kReady;
      }
    } else {
      status = StragetyStatus::kWaitReply;
    }
  }
  return status;
}

OrderEventType CTASignalDispatch::OrdersContextHandleRtnOrder(
    const boost::shared_ptr<const OrderField>& order) {
  return order->account_id == master_context_->account_id()
             ? master_context_->HandleRtnOrder(order)
             : slave_context_->HandleRtnOrder(order);
}
