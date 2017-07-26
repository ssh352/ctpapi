#include "cta_signal_no_cache_dispatch.h"

CTASignalNoCacheDispatch::CTASignalNoCacheDispatch(
    CTASignalObserver* signal_observer)
    : signal_observer_(signal_observer) {}

void CTASignalNoCacheDispatch::SubscribeEnterOrderObserver(
    EnterOrderObserver* observer) {
  enter_order_observer_ = observer;
}

// CTASignalNoCacheDispatch::CTASignalNoCacheDispatch(
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

void CTASignalNoCacheDispatch::RtnOrder(OrderData rtn_order) {
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

void CTASignalNoCacheDispatch::OpenOrder(const std::string& instrument,
                                         const std::string& order_id,
                                         OrderDirection direction,
                                         double price,
                                         int quantity) {
  slave_context_->HandleRtnOrder(
      OrderData{slave_context_->account_id(), order_id, instrument, "", "",
                "", quantity, 0, 0, price, direction, 
                OrderStatus::kActive, PositionEffect::kOpen});
  if (enter_order_observer_ != nullptr) {
    enter_order_observer_->OpenOrder(instrument, order_id, direction,
                                     price, quantity);
  }
}
void CTASignalNoCacheDispatch::CloseOrder(const std::string& instrument,
                                          const std::string& order_id,
                                          OrderDirection direction,
                                          PositionEffect position_effect,
                                          double price,
                                          int quantity) {
  slave_context_->HandleRtnOrder(
      OrderData{slave_context_->account_id(), order_id, instrument, "", "",
                "", quantity, 0, 0, price, direction, 
                OrderStatus::kActive, position_effect});
  if (enter_order_observer_ != nullptr) {
    enter_order_observer_->CloseOrder(instrument, order_id, direction,
                                      position_effect, price,
                                      quantity);
  }
}

void CTASignalNoCacheDispatch::CancelOrder(const std::string& order_id) {
  auto order = slave_context_->GetOrderData(order_id);
  order->status_ = OrderStatus::kCanceled;
  slave_context_->HandleRtnOrder(*order);
  if (enter_order_observer_ != nullptr) {
    enter_order_observer_->CancelOrder(order_id);
  }
}

void CTASignalNoCacheDispatch::SetOrdersContext(OrdersContext* master_context,
                                                OrdersContext* slave_context) {
  master_context_ = master_context;
  slave_context_ = slave_context;
}

OrderEventType CTASignalNoCacheDispatch::OrdersContextHandleRtnOrder(
    OrderData order) {
  return order.account_id() == master_context_->account_id()
             ? master_context_->HandleRtnOrder(order)
             : slave_context_->HandleRtnOrder(order);
}
