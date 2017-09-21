#include "cta_traded_signal.h"
#include <boost/format.hpp>
#include "order_util.h"
#include "string_util.h"

CTATradedSignal::CTATradedSignal(int delayed_open_order)
    : delayed_open_order_(delayed_open_order), order_id_prefix_("S") {}

void CTATradedSignal::SetOrdersContext(
    std::shared_ptr<OrdersContext> master_context,
    std::shared_ptr<OrdersContext> slave_context) {
  master_context_ = master_context;
  slave_context_ = slave_context;
}

void CTATradedSignal::BeforeCloseMarket() {
  auto order_tuples = slave_context_->GetUnfillOrders();
  for (const auto& t : order_tuples) {
    if (auto order = slave_context_->GetOrderData(std::get<0>(t))) {
      if (IsCloseOrder(order->position_effect)) {
        observer_->CancelOrder(order->order_id);
      }
    }
  }
}

void CTATradedSignal::HandleTick(const std::shared_ptr<TickData>& tick) {
  auto it_up_bound = std::find_if(
      pending_delayed_open_order_.begin(), pending_delayed_open_order_.end(),
      [=, &tick](const auto& item) {
        return item.timestamp_ + delayed_open_order_ * 1000 >=
               tick->tick->timestamp;
      });
  std::for_each(
      pending_delayed_open_order_.begin(), it_up_bound, [=](const auto& item) {
        observer_->OpenOrder(item.instrument_, item.order_id,
                             item.order_direction_, item.price_, item.qty_);
      });
  pending_delayed_open_order_.erase(pending_delayed_open_order_.begin(),
                                    it_up_bound);
}

void CTATradedSignal::HandleOpening(
    const std::shared_ptr<const OrderField>& order_data) {
  if (order_data->strategy_id != master_context_->account_id()) {
    return;
  }
  if (master_context_->IsOppositeOpen(order_data->instrument_id,
                                      order_data->direction)) {
    int master_quantity =
        master_context_->GetCloseableQuantityWithOrderDirection(
            order_data->instrument_id,
            OppositeOrderDirection(order_data->direction));

    int slave_quantity = slave_context_->GetCloseableQuantityWithOrderDirection(
        order_data->instrument_id,
        OppositeOrderDirection(order_data->direction));

    if (master_quantity == order_data->qty) {
      if (slave_quantity > 0) {
        // Fully lock
        observer_->OpenOrder(order_data->instrument_id, order_data->order_id,
                             order_data->direction, order_data->input_price,
                             slave_quantity);
      }

      if (master_context_->ActiveOrderCount(
              order_data->instrument_id,
              OppositeOrderDirection(order_data->direction)) == 0 &&
          slave_context_->ActiveOrderCount(
              order_data->instrument_id,
              OppositeOrderDirection(order_data->direction)) != 0) {
        for (auto order_id : slave_context_->ActiveOrderIds(
                 order_data->instrument_id,
                 OppositeOrderDirection(order_data->direction))) {
          observer_->CancelOrder(order_id);
        }
      }
    } else if (slave_quantity > 0) {
      observer_->OpenOrder(order_data->instrument_id, order_data->order_id,
                           order_data->direction, order_data->input_price,
                           order_data->qty);
    } else {
    }
  } else {
    if (delayed_open_order_ > 0) {
      pending_delayed_open_order_.insert(InputOrderSignal{
          order_data->instrument_id, order_data->order_id, "",
          PositionEffect::kOpen, order_data->direction, order_data->input_price,
          order_data->qty, order_data->update_timestamp});
    } else {
      observer_->OpenOrder(order_data->instrument_id, order_data->order_id,
                           order_data->direction, order_data->input_price,
                           order_data->qty);
    }
  }
}

void CTATradedSignal::HandleCloseing(
    const std::shared_ptr<const OrderField>& order_data) {
  if (order_data->strategy_id != master_context_->account_id()) {
    return;
  }

  std::vector<std::pair<std::string, int> > master_corr_order_quantitys =
      master_context_->GetCorrOrderQuantiys(order_data->order_id);

  int close_quantity = 0;
  for (auto master_corr_quantity : master_corr_order_quantitys) {
    auto it = pending_delayed_open_order_.find(master_corr_quantity.first);
    if (it != pending_delayed_open_order_.end()) {
      pending_delayed_open_order_.erase(it);
    }

    int master_closeable_quantity =
        master_context_->GetCloseableQuantity(master_corr_quantity.first);
    if (master_closeable_quantity == 0) {
      close_quantity +=
          slave_context_->GetCloseableQuantity(master_corr_quantity.first);
    } else {
      close_quantity += std::min<int>(
          master_corr_quantity.second,
          std::max<int>(0, slave_context_->GetCloseableQuantity(
                               master_corr_quantity.first) -
                               master_context_->GetCloseableQuantity(
                                   master_corr_quantity.first)));
    }

    if (!master_context_->IsActiveOrder(master_corr_quantity.first) &&
        slave_context_->IsActiveOrder(master_corr_quantity.first)) {
      observer_->CancelOrder(master_corr_quantity.first);
    }
  }

  if (close_quantity > 0) {
    observer_->CloseOrder(order_data->instrument_id, order_data->order_id,
                          order_data->direction, order_data->position_effect,
                          order_data->input_price, close_quantity);
  }
}

void CTATradedSignal::HandleCanceled(
    const std::shared_ptr<const OrderField>& order_data) {
  if (order_data->strategy_id != master_context_->account_id()) {
    return;
  }

  auto it = pending_delayed_open_order_.find(order_data->order_id);
  if (it != pending_delayed_open_order_.end()) {
    pending_delayed_open_order_.erase(it);
  }

  if (slave_context_->IsActiveOrder(order_data->order_id)) {
    observer_->CancelOrder(order_data->order_id);
  }
}

void CTATradedSignal::HandleClosed(
    const std::shared_ptr<const OrderField>& order_data) {
  if (order_data->strategy_id != master_context_->account_id()) {
    return;
  }

  if (master_context_->GetCloseableQuantityWithOrderDirection(
          order_data->instrument_id,
          OppositeOrderDirection(order_data->direction)) == 0 &&
      slave_context_->GetCloseableQuantityWithOrderDirection(
          order_data->instrument_id,
          OppositeOrderDirection(order_data->direction)) != 0) {
    // Close all position
    auto quantitys = slave_context_->GetQuantitysIf(
        order_data->instrument_id,
        [](auto quantity) { return quantity.closeable_quantity != 0; });

    if (order_data->exchange_id == kSHFEExchangeId) {
      int today_quantity = std::accumulate(
          quantitys.begin(), quantitys.end(), 0, [](int val, auto quantity) {
            return quantity.is_today_quantity
                       ? val + quantity.closeable_quantity
                       : val;
          });
      int yesterday_quantity = std::accumulate(
          quantitys.begin(), quantitys.end(), 0, [](int val, auto quantity) {
            return !quantity.is_today_quantity
                       ? val + quantity.closeable_quantity
                       : val;
          });
      if (yesterday_quantity > 0) {
        observer_->CloseOrder(order_data->instrument_id, GenerateOrderId(),
                              order_data->direction, PositionEffect::kClose, 0,
                              yesterday_quantity);
      }

      if (today_quantity > 0) {
        observer_->CloseOrder(order_data->instrument_id, GenerateOrderId(),
                              order_data->direction,
                              PositionEffect::kCloseToday, 0, today_quantity);
      }
    } else {
      int quantity = std::accumulate(quantitys.begin(), quantitys.end(), 0,
                                     [](int val, auto quantity) {
                                       return val + quantity.closeable_quantity;
                                     });

      // observer_->CloseOrder(order_data->instrument_id, order_data->order_id,
      //                      order_data->direction,
      //                      PositionEffect::kCloseToday, 0, quantity);

      observer_->CloseOrder(order_data->instrument_id, GenerateOrderId(),
                            order_data->direction, PositionEffect::kCloseToday,
                            order_data->input_price, quantity);
    }
  }
  //  delegate_->CloseOrder(order_data->Instrument())
}

void CTATradedSignal::HandleOpened(
    const std::shared_ptr<const OrderField>& order_data) {
  if (master_context_->IsOppositeOpen(order_data->instrument_id,
                                      order_data->direction)) {
    // observer_->OpenOrder(order_data->instrument_id, GenerateOrderId(),
    //                     order_data->direction, order_data->trading_price,
    //                     order_data->trading_qty);
  }
}

// void CTATradedSignal::Subscribe(CTATradedSignalObserver::Observable*
// observer) {
//  observer_ = observer;
//}
//
std::string CTATradedSignal::GenerateOrderId() {
  return str(boost::format("%s%d") % order_id_prefix_ % order_id_seq_++);
}
