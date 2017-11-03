#include "ctp_instrument_broker.h"
CTPInstrumentBroker::CTPInstrumentBroker(
    CTPOrderDelegate* delegate,
    std::function<std::string(void)> generate_order_id_func)
    : order_delegate_(delegate),
      generate_order_id_func_(generate_order_id_func) {}

void CTPInstrumentBroker::HandleRtnOrder(
    const std::shared_ptr<CTPOrderField>& order) {
  if (IsCtpOpenPositionEffect(order->position_effect)) {
    if (order->direction == OrderDirection::kBuy) {
      long_.qty += order->trading_qty;
    } else {
      short_.qty += order->trading_qty;
    }
  } else {
    if (order->direction == OrderDirection::kBuy) {
      long_.qty -= order->trading_qty;
      long_.frozen -= order->trading_qty;
    } else {
      short_.qty -= order->trading_qty;
      short_.frozen -= order->trading_qty;
    }
  }

  auto range = ctp_order_id_to_order_id_.equal_range(order->order_id);
  for (auto it = range.first; it != range.second; ++it) {
    auto it_find = orders_.find(it->second, HashInnerOrderField(),
                                CompareInnerOrderField());
    BOOST_ASSERT(it_find != orders_.end());
    (*it_find)->trading_price = order->trading_price;
    (*it_find)->trading_qty = order->trading_qty;
    (*it_find)->status = order->status;
    (*it_find)->leaves_qty -= order->trading_qty;
    BOOST_ASSERT((*it_find)->leaves_qty >= 0);
    order_delegate_->ReturnOrderField(
        std::make_shared<OrderField>(*(*it_find)));
  }
}

void CTPInstrumentBroker::HandleInputOrder(const InputOrder& order) {
  if (order.position_effect == PositionEffect::kClose &&
      order.qty > CloseableQty(order.direction == OrderDirection::kBuy
                                   ? long_
                                   : short_)) {
    // TODO: reply error
    return;
  }
  if (order.position_effect == PositionEffect::kClose) {
    if (order.direction == OrderDirection::kBuy) {
      long_.frozen += order.qty;
    } else {
      short_.frozen += order.qty;
    }
  }
  CTPEnterOrder ctp_enter_order;
  ctp_enter_order.order_id = generate_order_id_func_();
  ctp_enter_order.instrument = order.instrument;
  ctp_enter_order.position_effect =
      order.position_effect == PositionEffect::kOpen
          ? CTPPositionEffect::kOpen
          : CTPPositionEffect::kClose;
  ctp_enter_order.direction = order.direction;
  ctp_enter_order.price = order.price;
  ctp_enter_order.qty = order.qty;
  InsertOrderField(ctp_enter_order.instrument, order.order_id, order.direction,
                   order.position_effect, order.price, order.qty);
  BindOrderId(order.order_id, ctp_enter_order.order_id);
  order_delegate_->EnterOrder(std::move(ctp_enter_order));
}

void CTPInstrumentBroker::HandleCancel(const CancelOrderSignal& cancel) {
  auto it = orders_.find(cancel.order_id, HashInnerOrderField(),
                         CompareInnerOrderField());
  BOOST_ASSERT(it != orders_.end());
  if ((*it)->position_effect == PositionEffect::kClose) {
    if ((*it)->direction == OrderDirection::kBuy) {
      long_.frozen -= (*it)->leaves_qty;
    } else {
      short_.frozen -= (*it)->leaves_qty;
    }
  }
  order_delegate_->CancelOrder(cancel.account_id, cancel.order_id);
}

void CTPInstrumentBroker::InsertOrderField(const std::string& instrument,
                                           const std::string& order_id,
                                           OrderDirection direciton,
                                           PositionEffect position_effect,
                                           double price,
                                           int qty) {
  auto order = std::make_unique<OrderField>();
  order->instrument_id = instrument;
  order->order_id = order_id;
  order->direction = direciton;
  order->position_effect = position_effect;
  order->input_price = price;
  order->trading_price = 0.0;
  order->qty = qty;
  order->leaves_qty = qty;
  order->trading_qty = 0;
  order->status = OrderStatus::kActive;
  orders_.insert(std::move(order));
}

void CTPInstrumentBroker::BindOrderId(const std::string& order_id,
                                      const std::string& ctp_order_id) {
  ctp_order_id_to_order_id_.insert({ctp_order_id, order_id});
  order_id_to_ctp_order_id_.insert({order_id, ctp_order_id});
}

void CTPInstrumentBroker::InitPosition(int long_qty, int short_qty) {
  long_.qty = long_qty;
  short_.qty = short_qty;
}

int CTPInstrumentBroker::CloseableQty(const Position& pos) const {
  return pos.qty - pos.frozen;
}

bool CTPInstrumentBroker::IsCtpOpenPositionEffect(
    CTPPositionEffect position_effect) const {
  return position_effect == CTPPositionEffect::kOpen;
}
