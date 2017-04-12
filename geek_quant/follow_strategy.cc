#include "geek_quant/follow_strategy.h"
#include "geek_quant/order_util.h"

FollowStragety::FollowStragety(const std::string& master_account_id,
                               const std::string& slave_account_id,
                               TradeOrderDelegate* trade_order_delegate,
                               Delegate* delegate,
                               Context* context)
    : trade_order_delegate_(trade_order_delegate),
      delegate_(delegate),
      master_account_id_(master_account_id),
      slave_account_id_(slave_account_id),
      context_(context) {}

void FollowStragety::HandleOpening(const OrderData& order_data) {
  if (order_data.account_id_ != master_account_id_) {
    return;
  }
  delegate_->Trade(order_data.order_id());
  trade_order_delegate_->OpenOrder(
      order_data.instrument(), order_data.order_id(), order_data.direction(),
      order_data.price(), order_data.quanitty());
}

void FollowStragety::HandleCloseing(const OrderData& order_data) {
  if (order_data.account_id() != master_account_id_) {
    return;
  }

  // int master_closeable_quantity = context_->GetPositionCloseableQuantity(
  //     master_account_id_, order_data.instrument(),
  //     OppositeOrderDirection(order_data.direction()));

  // if (master_closeable_quantity == 0) {
  //   // Close all position
  //   context_->GetPositionCloseableQuantityWithPositionEffect(
  //       master_account_id_, order_data.instrument(),
  //       OppositeOrderDirection(order_data.direction()),
  //     order_data.);
  // } else {

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
      delegate_->Trade(master_corr_quantity.first);
      trade_order_delegate_->CancelOrder(master_corr_quantity.first);
    }
  }

  if (close_quantity > 0) {
    delegate_->Trade(order_data.order_id());
    trade_order_delegate_->CloseOrder(
        order_data.instrument(), order_data.order_id(), order_data.direction(),
        order_data.position_effect(), order_data.price(), close_quantity);
  }

  // }
  /*
  delegate_->Trade(order_data.order_id());
  trade_order_delegate_->CloseOrder(
      order_data.instrument(), order_data.order_id(), order_data.direction(),
      order_data.position_effect(), order_data.price(), order_data.quanitty());
  */
}

void FollowStragety::HandleCanceled(const OrderData& order_data) {
  if (order_data.account_id() != master_account_id_) {
    return;
  }

  if (context_->IsActiveOrder(slave_account_id_, order_data.order_id())) {
    delegate_->Trade(order_data.order_id());
    trade_order_delegate_->CancelOrder(order_data.order_id());
  }
}

void FollowStragety::HandleClosed(const OrderData& order_data) {
  if (order_data.account_id() != master_account_id_) {
    return;
  }

  //  delegate_->CloseOrder(order_data.Instrument())
}

void FollowStragety::HandleOpened(const OrderData& rtn_order) {}
