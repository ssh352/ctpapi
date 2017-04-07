#include "follow_stragety.h"

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

  delegate_->Trade(order_data.order_id());
  trade_order_delegate_->CloseOrder(
      order_data.instrument(), order_data.order_id(), order_data.direction(),
      order_data.position_effect(), order_data.price(), order_data.quanitty());
  /*
  if (account != master_account_id_) {
    return;
  }

  int master_closeable_quantity =
      context_.GetPositionCloseableQuantity(account, rtn_order.instrument);

  if (master_closeable_quantity == 0) {
    // Close all position
  } else {
    auto slave_order_quantitys = context_.GetOpenOrderQuantitysWithOrderNos(
        slave_account_id_, context_.GetCorrOrderNosWithOrderNo(
                               master_account_id_, rtn_order.order_no));

    auto master_order_quantitys = context_.GetCorrOrderQuantiysWithOrderNo(
        master_account_id_, rtn_order.order_no);

    for (auto master_quantity : master_order_quantitys) {
    }
  }
  */
}

void FollowStragety::HandleCanceled(const OrderData& order_data) {}

void FollowStragety::HandleClosed(const OrderData& order_data) {
  if (order_data.account_id() != master_account_id_) {
    return;
  }

  //  delegate_->CloseOrder(order_data.Instrument())
}

void FollowStragety::HandleOpened(const OrderData& rtn_order) {}
