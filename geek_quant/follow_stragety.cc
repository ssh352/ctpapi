#include "follow_stragety.h"

FollowStragety::FollowStragety(const std::string& master_account_id,
                               const std::string& slave_account_id,
                               FollowStragety::OrderDelegate* del)
    : delegate_(del),
      master_account_id_(master_account_id),
      slave_account_id_(slave_account_id) {}

void FollowStragety::HandleOpening(RtnOrderData rtn_order,
                                   const Context& context_) {
  delegate_->OpenOrder(rtn_order.instrument, rtn_order.order_no,
                       rtn_order.order_direction, rtn_order.order_price,
                       rtn_order.volume);
}

void FollowStragety::HandleCloseing(RtnOrderData rtn_order,
                                    const Context& context_) {
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

void FollowStragety::HandleCanceled(RtnOrderData rtn_order,
                                    const Context& context) {
  /*
  if (account != master_account_id_) {
    return;
  }
  
  */
}
