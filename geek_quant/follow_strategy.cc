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

  std::vector<OrderQuantity> slave_order_quantitys = context_->GetQuantitys(
      slave_account_id_, context_->GetCloseCorrOrderIds(master_account_id_,
                                                        order_data.order_id()));

  std::vector<std::pair<std::string, int> > master_order_quantitys =
      context_->GetCorrOrderQuantiys(master_account_id_, order_data.order_id());

  int close_quantity = 0;
  for (auto master_quantity : master_order_quantitys) {
    auto it = std::find_if(slave_order_quantitys.begin(),
                           slave_order_quantitys.end(), [&](auto quantity) {
                             return quantity.order_id == master_quantity.first;
                           });
    if (it != slave_order_quantitys.end()) {
      close_quantity +=
          std::min<int>(master_quantity.second, it->closeable_quantity);
    }
  }

  delegate_->Trade(order_data.order_id());
  trade_order_delegate_->CloseOrder(
      order_data.instrument(), order_data.order_id(), order_data.direction(),
      order_data.position_effect(), order_data.price(), close_quantity);

  for (auto order_id : context_->GetCloseCorrOrderIds(master_account_id_,
                                                      order_data.order_id())) {
    if (!context_->IsUnfillOrder(master_account_id_, order_id) && 
        context_->IsUnfillOrder(slave_account_id_, order_id)) {
      delegate_->Trade(order_id);
      trade_order_delegate_->CancelOrder(order_id);
    }
  }

  // }
  /*
  delegate_->Trade(order_data.order_id());
  trade_order_delegate_->CloseOrder(
      order_data.instrument(), order_data.order_id(), order_data.direction(),
      order_data.position_effect(), order_data.price(), order_data.quanitty());
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
