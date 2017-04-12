#include "geek_quant/follow_strategy.h"
#include "geek_quant/order_util.h"

FollowStragety::FollowStragety(const std::string& master_account_id,
                               const std::string& slave_account_id,
                               Delegate* delegate,
                               Context* context)
    : delegate_(delegate),
      master_account_id_(master_account_id),
      slave_account_id_(slave_account_id),
      context_(context) {}

void FollowStragety::HandleOpening(const OrderData& order_data) {
  if (order_data.account_id_ != master_account_id_) {
    return;
  }
  if (context_->IsOppositeOpen(order_data.account_id(), order_data.instrument(),
                               order_data.direction())) {
    int master_quantity = context_->GetCloseableQuantityWithOrderDirection(
        master_account_id_, order_data.instrument(),
        OppositeOrderDirection(order_data.direction()));

    int slave_quantity = context_->GetCloseableQuantityWithOrderDirection(
        slave_account_id_, order_data.instrument(),
        OppositeOrderDirection(order_data.direction()));

    if (master_quantity == order_data.quanitty()) {
      if (slave_quantity > 0) {
        // Fully lock
        delegate_->OpenOrder(order_data.instrument(), order_data.order_id(),
                             order_data.direction(), OrderPriceType::kLimit,
                             order_data.price(), slave_quantity);
      }

      if (context_->ActiveOrderCount(
              master_account_id_, order_data.instrument(),
              OppositeOrderDirection(order_data.direction())) == 0 &&
          context_->ActiveOrderCount(
              slave_account_id_, order_data.instrument(),
              OppositeOrderDirection(order_data.direction())) != 0) {
        for (auto order_id : context_->ActiveOrderIds(
                 slave_account_id_, order_data.instrument(),
                 OppositeOrderDirection(order_data.direction()))) {
          delegate_->CancelOrder(order_id);
        }
      }
    } else {
      delegate_->OpenOrder(order_data.instrument(), order_data.order_id(),
                           order_data.direction(), OrderPriceType::kLimit,
                           order_data.price(),
                           order_data.quanitty());
    }
  } else {
    delegate_->OpenOrder(order_data.instrument(), order_data.order_id(),
                         order_data.direction(), OrderPriceType::kLimit,
                         order_data.price(),
                         order_data.quanitty());
  }
}

void FollowStragety::HandleCloseing(const OrderData& order_data) {
  if (order_data.account_id() != master_account_id_) {
    return;
  }

  std::vector<std::pair<std::string, int> > master_corr_order_quantitys =
      context_->GetCorrOrderQuantiys(master_account_id_, order_data.order_id());

  int close_quantity = 0;
  for (auto master_corr_quantity : master_corr_order_quantitys) {
    close_quantity += std::min<int>(
        master_corr_quantity.second,
        std::max<int>(
            0, context_->GetCloseableQuantity(slave_account_id_,
                                              master_corr_quantity.first) -
                   context_->GetCloseableQuantity(master_account_id_,
                                                  master_corr_quantity.first)));

    if (!context_->IsActiveOrder(master_account_id_,
                                 master_corr_quantity.first) &&
        context_->IsActiveOrder(slave_account_id_,
                                master_corr_quantity.first)) {
      delegate_->CancelOrder(master_corr_quantity.first);
    }
  }

  if (close_quantity > 0) {
    delegate_->CloseOrder(order_data.instrument(), order_data.order_id(),
                          order_data.direction(), order_data.position_effect(),
                          OrderPriceType::kLimit, order_data.price(),
                          close_quantity);
  }
}

void FollowStragety::HandleCanceled(const OrderData& order_data) {
  if (order_data.account_id() != master_account_id_) {
    return;
  }

  if (context_->IsActiveOrder(slave_account_id_, order_data.order_id())) {
    delegate_->CancelOrder(order_data.order_id());
  }
}

void FollowStragety::HandleClosed(const OrderData& order_data) {
  if (order_data.account_id() != master_account_id_) {
    return;
  }

  if (context_->GetCloseableQuantityWithOrderDirection(
          master_account_id_, order_data.instrument(),
          OppositeOrderDirection(order_data.direction())) == 0 &&
      context_->GetCloseableQuantityWithOrderDirection(
          slave_account_id_, order_data.instrument(),
          OppositeOrderDirection(order_data.direction())) != 0) {
    // Close all position
    auto quantitys = context_->GetQuantitysIf(
        slave_account_id_, order_data.instrument(),
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
        delegate_->CloseOrder(order_data.instrument(),
                              context_->GenerateOrderId(),
                              order_data.direction(), PositionEffect::kClose,
                              OrderPriceType::kMarket, 0, yesterday_quantity);
      }

      if (today_quantity > 0) {
        delegate_->CloseOrder(
            order_data.instrument(), context_->GenerateOrderId(),
            order_data.direction(), PositionEffect::kCloseToday,
            OrderPriceType::kMarket, 0, today_quantity);
      }
    } else {
      int quantity = std::accumulate(quantitys.begin(), quantitys.end(), 0,
                                     [](int val, auto quantity) {
                                       return val + quantity.closeable_quantity;
                                     });

      delegate_->CloseOrder(order_data.instrument(),
                            context_->GenerateOrderId(), order_data.direction(),
                            PositionEffect::kCloseToday,
                            OrderPriceType::kMarket, 0, quantity);
    }
  }
  //  delegate_->CloseOrder(order_data.Instrument())
}

void FollowStragety::HandleOpened(const OrderData& rtn_order) {}
