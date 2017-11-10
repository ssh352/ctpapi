#include "ctp_instrument_broker.h"
#include "hpt_core/order_util.h"
CTPInstrumentBroker::CTPInstrumentBroker(
    CTPOrderDelegate* delegate,
    std::string instrument,
    bool close_today_aware,
    std::function<std::string(void)> generate_order_id_func)
    : order_delegate_(delegate),
      instrument_(std::move(instrument)),
      generate_order_id_func_(generate_order_id_func) {
  if (close_today_aware) {
    long_ = std::make_unique<CloseTodayAwareCTPPositionAmount>();
    short_ = std::make_unique<CloseTodayAwareCTPPositionAmount>();
  } else {
    long_ = std::make_unique<CTPPositionAmount>();
    short_ = std::make_unique<CTPPositionAmount>();
  }
}

void CTPInstrumentBroker::HandleRtnOrder(
    const std::shared_ptr<CTPOrderField>& order) {
  if (IsCtpOpenPositionEffect(order->position_effect)) {
    if (order->position_effect_direction == OrderDirection::kBuy) {
      long_->OpenTraded(order->trading_qty);
    } else {
      short_->OpenTraded(order->trading_qty);
    }
  } else {
    if (order->position_effect_direction == OrderDirection::kBuy) {
      long_->CloseTraded(order->trading_qty, order->position_effect);
    } else {
      short_->CloseTraded(order->trading_qty, order->position_effect);
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

void CTPInstrumentBroker::HandleTrader(const std::string& order_id,
                                       double trading_price,
                                       int trading_qty,
                                       TimeStamp timestamp) {}

void CTPInstrumentBroker::HandleInputOrder(const InputOrder& order) {
  position_effect_strategy_->HandleInputOrder(order, *long_, *short_);
  InsertOrderField(order.instrument, order.order_id, order.direction,
                   order.position_effect, order.price, order.qty);
}

void CTPInstrumentBroker::HandleCancel(const CancelOrderSignal& cancel) {
  BOOST_ASSERT(cancel.qty > 0);
  auto range = order_id_to_ctp_order_id_.equal_range(cancel.order_id);
  std::vector<std::tuple<std::string, CTPPositionEffect, int> > ctp_leave_order;
  int leaves_qty = 0;
  for (auto it = range.first; it != range.second; ++it) {
    auto ctp_it = ctp_orders_.find(it->second, HashCTPOrderField(),
                                   CompareCTPOrderField());
    BOOST_ASSERT(ctp_it != ctp_orders_.end());
    if (ctp_it == ctp_orders_.end() ||
        (*ctp_it)->status != OrderStatus::kActive) {
      continue;
    }
    leaves_qty += (*ctp_it)->leaves_qty;
    ctp_leave_order.emplace_back(
        (*ctp_it)->order_id, (*ctp_it)->position_effect, (*ctp_it)->leaves_qty);
  }
  BOOST_ASSERT(leaves_qty >= cancel.qty);
  std::sort(ctp_leave_order.begin(), ctp_leave_order.end(),
            [](const auto& l, const auto& r) {
              return std::make_pair(std::get<1>(l), std::get<2>(l)) <
                     std::make_pair(std::get<1>(r), std::get<2>(r));
            });

  int leaves_cancel_qty = cancel.qty;
  for (auto it = ctp_leave_order.begin(); it != ctp_leave_order.end(); ++it) {
    int cancel_qty = std::min<int>(std::get<2>(*it), leaves_cancel_qty);
    auto ctp_order_it = ctp_orders_.find(std::get<0>(*it), HashCTPOrderField(),
                                         CompareCTPOrderField());
    BOOST_ASSERT(ctp_order_it != ctp_orders_.end());
    order_delegate_->CancelOrder(
        {(*ctp_order_it)->front_id, (*ctp_order_it)->session_id,
         (*ctp_order_it)->order_id, (*ctp_order_it)->order_ref,
         (*ctp_order_it)->order_sys_id, (*ctp_order_it)->exchange_id});
    if (!IsCtpOpenPositionEffect((*ctp_order_it)->position_effect)) {
      if ((*ctp_order_it)->position_effect_direction == OrderDirection::kBuy) {
        long_->Unfrozen(std::get<2>(*it), (*ctp_order_it)->position_effect);
      } else {
        short_->Unfrozen(std::get<2>(*it), (*ctp_order_it)->position_effect);
      }
    }
    if (cancel_qty < std::get<2>(*it)) {
      // Re input leaves qty
      CTPEnterOrder enter_order;
      enter_order.order_id = generate_order_id_func_();
      enter_order.instrument = instrument_;
      enter_order.position_effect = (*ctp_order_it)->position_effect;
      enter_order.direction = (*ctp_order_it)->direction;
      enter_order.price = (*ctp_order_it)->input_price;
      enter_order.qty = std::get<2>(*it) - cancel_qty;
      BindOrderId(cancel.order_id, enter_order.order_id);

      if (!IsCtpOpenPositionEffect((*ctp_order_it)->position_effect)) {
        if ((*ctp_order_it)->position_effect_direction ==
            OrderDirection::kBuy) {
          long_->Frozen(enter_order.qty, (*ctp_order_it)->position_effect);
        } else {
          short_->Frozen(enter_order.qty, (*ctp_order_it)->position_effect);
        }
      }

      order_delegate_->EnterOrder(std::move(enter_order));
    }
    leaves_cancel_qty -= cancel_qty;
  }

  BOOST_ASSERT(leaves_cancel_qty == 0);
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
  order->position_effect_direction = direciton;
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

void CTPInstrumentBroker::InitPosition(std::pair<int, int> long_pos,
                                       std::pair<int, int> short_pos) {
  long_->Init(long_pos.first, 0, long_pos.second, 0);
  short_->Init(short_pos.first, 0, short_pos.second, 0);
}

bool CTPInstrumentBroker::IsCtpOpenPositionEffect(
    CTPPositionEffect position_effect) const {
  return position_effect == CTPPositionEffect::kOpen;
}

void CTPInstrumentBroker::PosstionEffectStrategyHandleInputOrder(
    const std::string& input_order_id,
    CTPPositionEffect position_effect,
    OrderDirection direction,
    double price,
    int qty) {
  CTPPositionAmount* position =
      (direction == OrderDirection::kBuy ? long_.get() : short_.get());
  if (!IsCtpOpenPositionEffect(position_effect)) {
    position->Frozen(qty, position_effect);
  }
  CTPEnterOrder ctp_enter_order;
  ctp_enter_order.order_id = generate_order_id_func_();
  ctp_enter_order.instrument = instrument_;
  ctp_enter_order.position_effect = position_effect;
  ctp_enter_order.direction = direction;
  ctp_enter_order.price = price;
  ctp_enter_order.qty = qty;
  BindOrderId(input_order_id, ctp_enter_order.order_id);
  order_delegate_->EnterOrder(std::move(ctp_enter_order));
}

