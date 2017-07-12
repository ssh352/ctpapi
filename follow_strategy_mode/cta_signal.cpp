#include "cta_signal.h"
#include "string_util.h"
#include "order_util.h"


// CTASignal::CTASignal(const std::string& master_account_id,
//                      const std::string& slave_account_id,
//                      Delegate* delegate,
//                      std::shared_ptr<OrdersContext> master_context,
//                      std::shared_ptr<OrdersContext> slave_context)
//     : delegate_(delegate),
//       master_account_id_(master_account_id),
//       slave_account_id_(slave_account_id),
//       master_context_(master_context),
//       slave_context_(slave_context) {}


void CTASignal::SetObserver(EnterOrderObserver* observer) {
  observer_ = observer;
}

void CTASignal::HandleOpening(const OrderData& order_data) {
  if (order_data.account_id_ != master_account_id_) {
    return;
  }
  if (master_context_->IsOppositeOpen(order_data.instrument(),
                                      order_data.direction())) {
    int master_quantity =
        master_context_->GetCloseableQuantityWithOrderDirection(
            order_data.instrument(),
            OppositeOrderDirection(order_data.direction()));

    int slave_quantity = slave_context_->GetCloseableQuantityWithOrderDirection(
        order_data.instrument(),
        OppositeOrderDirection(order_data.direction()));

    if (master_quantity == order_data.quanitty()) {
      if (slave_quantity > 0) {
        // Fully lock
        observer_->OpenOrder(order_data.instrument(), order_data.order_id(),
                             order_data.direction(), OrderPriceType::kLimit,
                             order_data.price(), slave_quantity);
      }

      if (master_context_->ActiveOrderCount(
              order_data.instrument(),
              OppositeOrderDirection(order_data.direction())) == 0 &&
          slave_context_->ActiveOrderCount(
              order_data.instrument(),
              OppositeOrderDirection(order_data.direction())) != 0) {
        for (auto order_id : slave_context_->ActiveOrderIds(
                 order_data.instrument(),
                 OppositeOrderDirection(order_data.direction()))) {
          observer_->CancelOrder(order_id);
        }
      }
    } else {
      observer_->OpenOrder(order_data.instrument(), order_data.order_id(),
                           order_data.direction(), OrderPriceType::kLimit,
                           order_data.price(), order_data.quanitty());
    }
  } else {
    observer_->OpenOrder(order_data.instrument(), order_data.order_id(),
                         order_data.direction(), OrderPriceType::kLimit,
                         order_data.price(), order_data.quanitty());
  }
}

void CTASignal::HandleCloseing(const OrderData& order_data) {
  if (order_data.account_id() != master_account_id_) {
    return;
  }

  std::vector<std::pair<std::string, int> > master_corr_order_quantitys =
      master_context_->GetCorrOrderQuantiys(order_data.order_id());

  int close_quantity = 0;
  for (auto master_corr_quantity : master_corr_order_quantitys) {
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
    observer_->CloseOrder(order_data.instrument(), order_data.order_id(),
                          order_data.direction(), order_data.position_effect(),
                          OrderPriceType::kLimit, order_data.price(),
                          close_quantity);
  }
}

void CTASignal::HandleCanceled(const OrderData& order_data) {
  if (order_data.account_id() != master_account_id_) {
    return;
  }

  if (slave_context_->IsActiveOrder(order_data.order_id())) {
    observer_->CancelOrder(order_data.order_id());
  }
}

void CTASignal::HandleClosed(const OrderData& order_data) {
  if (order_data.account_id() != master_account_id_) {
    return;
  }

  if (master_context_->GetCloseableQuantityWithOrderDirection(
          order_data.instrument(),
          OppositeOrderDirection(order_data.direction())) == 0 &&
      slave_context_->GetCloseableQuantityWithOrderDirection(
          order_data.instrument(),
          OppositeOrderDirection(order_data.direction())) != 0) {
    // Close all position
    auto quantitys = slave_context_->GetQuantitysIf(
        order_data.instrument(),
        [](auto quantity) { return quantity.closeable_quantity != 0; });

    if (order_data.exchange_id() == kSHFEExchangeId) {
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
        observer_->CloseOrder(order_data.instrument(), order_data.order_id(),
                              order_data.direction(), PositionEffect::kClose,
                              OrderPriceType::kMarket, 0, yesterday_quantity);
      }

      if (today_quantity > 0) {
        observer_->CloseOrder(order_data.instrument(), order_data.order_id(),
                              order_data.direction(),
                              PositionEffect::kCloseToday,
                              OrderPriceType::kMarket, 0, today_quantity);
      }
    } else {
      int quantity = std::accumulate(quantitys.begin(), quantitys.end(), 0,
                                     [](int val, auto quantity) {
                                       return val + quantity.closeable_quantity;
                                     });

      observer_->CloseOrder(order_data.instrument(), order_data.order_id(),
                            order_data.direction(), PositionEffect::kCloseToday,
                            OrderPriceType::kMarket, 0, quantity);
    }
  }
  //  delegate_->CloseOrder(order_data.Instrument())
}

void CTASignal::HandleOpened(const OrderData& rtn_order) {}
