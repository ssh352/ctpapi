#include "ctp_instrument_broker.h"
#include "hpt_core/order_util.h"
#include "live_trade/live_trade_logging.h"
CTPInstrumentBroker::CTPInstrumentBroker(
    CTPOrderDelegate* delegate,
    std::string instrument,
    bool close_today_aware,
    std::function<std::string(void)> generate_order_id_func)
    : order_delegate_(delegate),
      instrument_(std::move(instrument)),
      generate_order_id_func_(generate_order_id_func) {
  // if (close_today_aware) {
  //  long_ = std::make_unique<CloseTodayAwareCTPPositionAmount>();
  //  short_ = std::make_unique<CloseTodayAwareCTPPositionAmount>();
  //} else {
  long_ = std::make_unique<CTPPositionAmount>();
  short_ = std::make_unique<CTPPositionAmount>();
  //}
}

void CTPInstrumentBroker::HandleRtnOrder(
    const std::shared_ptr<CTPOrderField>& order) {
  auto ctp_it = ctp_orders_.find(order->order_id, HashCTPOrderField(),
                                 CompareCTPOrderField());
  if (ctp_it == ctp_orders_.end()) {
    ctp_orders_.insert(std::make_unique<CTPOrderField>(*order));
    auto it = ctp_order_id_to_order_id_.find(order->order_id);
    auto it_find = orders_.find(it->second, HashInnerOrderField(),
                                CompareInnerOrderField());
    BOOST_ASSERT(it_find != orders_.end());
    auto bimap_it = waiting_reply_input_orders_.left.find(order->order_id);
    waiting_reply_input_orders_.left.erase(bimap_it);
    if (0 == waiting_reply_input_orders_.right.count((*it_find)->order_id)) {
      /*   (*it_find)->input_price = order->input_price;
          (*it_find)->qty = order->qty;
          (*it_find)->trading_price = order->trading_price;
          (*it_find)->trading_qty = order->trading_qty;
      */
      if (order->input_price != (*it_find)->input_price) {
        (*it_find)->input_price = order->input_price;
      }
      (*it_find)->status = order->status;
      (*it_find)->leaves_qty -= order->trading_qty;
      (*it_find)->input_timestamp = order->input_timestamp;
      (*it_find)->update_timestamp = order->update_timestamp;
      BOOST_ASSERT((*it_find)->leaves_qty >= 0);
      order_delegate_->ReturnOrderField(
          std::make_shared<OrderField>(*(*it_find)));
    }
  } else if (order->status == OrderStatus::kCanceled) {
    if (!IsCtpOpenPositionEffect((*ctp_it)->position_effect)) {
      if ((*ctp_it)->position_effect_direction == OrderDirection::kBuy) {
        long_->Unfrozen((*ctp_it)->leaves_qty, (*ctp_it)->position_effect);
      } else {
        short_->Unfrozen((*ctp_it)->leaves_qty, (*ctp_it)->position_effect);
      }
    }
    auto it = std::find_if(
        pending_order_action_queue_.begin(), pending_order_action_queue_.end(),
        [&order](const auto& pending_order_action) {
          return pending_order_action.pending_cancel_order_ids.find(
                     order->order_id) !=
                 pending_order_action.pending_cancel_order_ids.end();
        });
    if (it != pending_order_action_queue_.end()) {
      it->pending_cancel_order_ids.erase(order->order_id);

      auto order_it =
          orders_.find(it->order_action.order_id, HashInnerOrderField(),
                       CompareInnerOrderField());
      BOOST_ASSERT(order_it != orders_.end());
      if (order_it != orders_.end() && it->pending_cancel_order_ids.empty()) {
        position_effect_strategy_->HandleInputOrder(
            InputOrder{(*order_it)->instrument_id, (*order_it)->order_id,
                       (*order_it)->position_effect, (*order_it)->direction,
                       it->order_action.new_price, it->order_action.new_qty, 0},
            *long_, *short_);
        pending_order_action_queue_.erase(it);
      }
    } else {
      auto ctp_it = ctp_orders_.find(order->order_id, HashCTPOrderField(),
                                     CompareCTPOrderField());
      BOOST_ASSERT(ctp_it != ctp_orders_.end());
      (*ctp_it)->status = order->status;

      auto it = ctp_order_id_to_order_id_.find(order->order_id);
      BOOST_ASSERT(it != ctp_order_id_to_order_id_.end());
      auto it_find = orders_.find(it->second, HashInnerOrderField(),
                                  CompareInnerOrderField());
      BOOST_ASSERT(it_find != orders_.end());
      //(*it_find)->trading_price = order->trading_price;
      //(*it_find)->trading_qty = order->trading_qty;
      //(*it_find)->leaves_qty -= order->trading_qty;
      (*it_find)->status = order->status;
      (*it_find)->update_timestamp = order->update_timestamp;
      BOOST_ASSERT((*it_find)->leaves_qty >= 0);
      order_delegate_->ReturnOrderField(
          std::make_shared<OrderField>(*(*it_find)));
    }
  } else {
  }
}

void CTPInstrumentBroker::HandleTraded(const std::string& order_id,
                                       double trading_price,
                                       int trading_qty,
                                       TimeStamp timestamp) {
  auto ctp_it =
      ctp_orders_.find(order_id, HashCTPOrderField(), CompareCTPOrderField());
  BOOST_ASSERT(ctp_it != ctp_orders_.end());
  if (ctp_it == ctp_orders_.end()) {
    return;
  }

  if (IsCtpOpenPositionEffect((*ctp_it)->position_effect)) {
    if ((*ctp_it)->position_effect_direction == OrderDirection::kBuy) {
      long_->OpenTraded(trading_qty);
    } else {
      short_->OpenTraded(trading_qty);
    }
  } else {
    if ((*ctp_it)->position_effect_direction == OrderDirection::kBuy) {
      long_->CloseTraded(trading_qty, (*ctp_it)->position_effect);
    } else {
      short_->CloseTraded(trading_qty, (*ctp_it)->position_effect);
    }
  }

  (*ctp_it)->leaves_qty -= trading_qty;
  BOOST_ASSERT((*ctp_it)->leaves_qty >= 0);

  auto it = ctp_order_id_to_order_id_.find((*ctp_it)->order_id);
  BOOST_ASSERT(it != ctp_order_id_to_order_id_.end());
  int leaves_trading_qty = trading_qty;
  auto it_find =
      orders_.find(it->second, HashInnerOrderField(), CompareInnerOrderField());
  BOOST_ASSERT(it_find != orders_.end());
  int trading_qty_on_order =
      std::min<int>((*it_find)->leaves_qty, leaves_trading_qty);
  (*it_find)->leaves_qty -= trading_qty_on_order;
  BOOST_ASSERT((*it_find)->leaves_qty >= 0);
  (*it_find)->trading_qty = trading_qty_on_order;
  (*it_find)->trading_price = trading_price;
  (*it_find)->status = (*it_find)->leaves_qty == 0 ? OrderStatus::kAllFilled
                                                   : OrderStatus::kActive;
  (*it_find)->update_timestamp = timestamp;
  order_delegate_->ReturnOrderField(std::make_shared<OrderField>(*(*it_find)));
}

void CTPInstrumentBroker::HandleInputOrder(const InputOrder& order) {
  position_effect_strategy_->HandleInputOrder(order, *long_, *short_);
  InsertOrderField(order.instrument, order.order_id, order.direction,
                   order.position_effect, order.price, order.qty);
}

void CTPInstrumentBroker::HandleCancel(const CancelOrder& cancel) {
  auto range = order_id_to_ctp_order_id_.equal_range(cancel.order_id);
  for (auto it = range.first; it != range.second; ++it) {
    auto ctp_order_it = ctp_orders_.find(it->second, HashCTPOrderField(),
                                         CompareCTPOrderField());
    BOOST_ASSERT(ctp_order_it != ctp_orders_.end());
    if (ctp_order_it == ctp_orders_.end() ||
        (*ctp_order_it)->status != OrderStatus::kActive) {
      continue;
    }

    order_delegate_->HandleCancelOrder(
        {(*ctp_order_it)->front_id, (*ctp_order_it)->session_id,
         (*ctp_order_it)->order_id, (*ctp_order_it)->order_ref,
         (*ctp_order_it)->order_sys_id, (*ctp_order_it)->exchange_id});
  }
}

void CTPInstrumentBroker::HandleOrderAction(const OrderAction& order_action) {
  PendingOrderAction action;
  action.order_action = order_action;
  auto order_it = orders_.find(order_action.order_id, HashInnerOrderField(),
                               CompareInnerOrderField());
  BOOST_ASSERT(order_it != orders_.end());

  if (order_action.new_price == order_action.old_price) {
    action.order_action.new_price = (*order_it)->input_price;
  }

  if (order_action.new_qty == order_action.old_qty) {
    action.order_action.new_qty = (*order_it)->leaves_qty;
  }

  auto range = order_id_to_ctp_order_id_.equal_range(order_action.order_id);
  for (auto it = range.first; it != range.second; ++it) {
    auto ctp_order_it = ctp_orders_.find(it->second, HashCTPOrderField(),
                                         CompareCTPOrderField());
    BOOST_ASSERT(ctp_order_it != ctp_orders_.end());
    if (ctp_order_it == ctp_orders_.end() ||
        (*ctp_order_it)->status != OrderStatus::kActive) {
      auto& log = BLog::get();
      BOOST_LOG_SEV(log, SeverityLevel::kWarning) << "ActionOrder Reject!";
      continue;
    }
    order_delegate_->HandleCancelOrder(
        {(*ctp_order_it)->front_id, (*ctp_order_it)->session_id,
         (*ctp_order_it)->order_id, (*ctp_order_it)->order_ref,
         (*ctp_order_it)->order_sys_id, (*ctp_order_it)->exchange_id});
    action.pending_cancel_order_ids.insert(it->second);
  }
  pending_order_action_queue_.push_back(action);
}

void CTPInstrumentBroker::InsertOrderField(const std::string& instrument,
                                           const std::string& order_id,
                                           OrderDirection direction,
                                           PositionEffect position_effect,
                                           double price,
                                           int qty) {
  auto order = std::make_unique<OrderField>();
  order->instrument_id = instrument;
  order->order_id = order_id;
  order->direction = direction;
  order->position_effect_direction =
      AdjustDirectionByPositionEffect(position_effect, direction);
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
  waiting_reply_input_orders_.insert(Bimap::value_type(ctp_order_id, order_id));
  ctp_order_id_to_order_id_.insert({ctp_order_id, order_id});
  order_id_to_ctp_order_id_.insert({order_id, ctp_order_id});
}

void CTPInstrumentBroker::InitPosition(std::pair<int, int> long_pos,
                                       std::pair<int, int> short_pos) {
  long_->Init(long_pos.first, 0, long_pos.second, 0);
  short_->Init(short_pos.first, 0, short_pos.second, 0);
}

void CTPInstrumentBroker::InitPosition(OrderDirection direction,
                                       std::pair<int, int> pos) {
  if (direction == OrderDirection::kBuy) {
    long_->Init(pos.first, 0, pos.second, 0);
  } else {
    short_->Init(pos.first, 0, pos.second, 0);
  }
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
      (OppositeOrderDirection(direction) == OrderDirection::kBuy
           ? long_.get()
           : short_.get());
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
  order_delegate_->HandleEnterOrder(std::move(ctp_enter_order));
}

void CTPInstrumentBroker::UnfrozenByCancelCloseOrder(
    const std::string& ctp_order_id) {
  auto ctp_order_it = ctp_orders_.find(ctp_order_id, HashCTPOrderField(),
                                       CompareCTPOrderField());
  BOOST_ASSERT(ctp_order_it != ctp_orders_.end());
  if (ctp_order_it == ctp_orders_.end() ||
      (*ctp_order_it)->status != OrderStatus::kActive) {
    return;
  }
}

boost::optional<OrderPosition> CTPInstrumentBroker::GetPosition() const {
  BOOST_ASSERT(long_->Closeable() == long_->Total());
  BOOST_ASSERT(short_->Closeable() == short_->Total());

  if (long_->Closeable() == short_->Closeable()) {
    return boost::optional<OrderPosition>();
  }

  return OrderPosition{instrument_,
                       long_->Closeable() > short_->Closeable()
                           ? OrderDirection::kBuy
                           : OrderDirection::kSell,
                       abs(long_->Closeable() - short_->Closeable())};
}
