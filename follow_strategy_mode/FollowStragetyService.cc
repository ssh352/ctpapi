#include "FollowStragetyService.h"
#include <boost/lexical_cast.hpp>

void FollowStragetyService::Setup() {}

void FollowStragetyService::InitMasterPositions(
    std::vector<OrderPosition> positions) {
  for (auto stragety : stragetys_) {
    stragety.second->InitPositions(master_account_id_, positions);
  }
}

void FollowStragetyService::InitStragetyPositions(
    const std::string& stragety_id,
    std::vector<OrderPosition> positions) {
  if (stragetys_.find(stragety_id) != stragetys_.end()) {
    stragetys_[stragety_id]->InitPositions(slave_account_id_, positions);
  }
  
}

void FollowStragetyService::HandleRtnOrder(OrderData rtn_order) {
  if (rtn_order.account_id() != master_account_id_) {
    auto it = stragety_order_.right.find(rtn_order.order_id());
    if (it != stragety_order_.right.end()) {
      rtn_order.order_id_ = it->second.order_id;
      stragetys_[it->second.stragety_id]->HandleRtnOrder(std::move(rtn_order));
    } else {
      // Exception or maybe muanul control
    }
  } else {
    for (auto it = stragetys_.begin(); it != stragetys_.end(); ++it) {
      it->second->HandleRtnOrder(std::move(rtn_order));
    }
  }
}

void FollowStragetyService::OpenOrder(const std::string& stragety_id,
                                  const std::string& instrument,
                                  const std::string& order_no,
                                  OrderDirection direction,
                                  OrderPriceType price_type,
                                  double price,
                                  int quantity) {
  std::string adjust_order_no =
      boost::lexical_cast<std::string>(stragety_order_.size());
  stragety_order_.insert(
      StragetyOrderBiMap::value_type({stragety_id, order_no}, adjust_order_no));
  delegate_->OpenOrder(instrument, adjust_order_no, direction, price_type,
                       price, quantity);
}

void FollowStragetyService::CloseOrder(const std::string& stragety_id,
                                   const std::string& instrument,
                                   const std::string& order_no,
                                   OrderDirection direction,
                                   PositionEffect position_effect,
                                   OrderPriceType price_type,
                                   double price,
                                   int quantity) {
  std::string adjust_order_no =
      boost::lexical_cast<std::string>(stragety_order_.size());
  stragety_order_.insert(
      StragetyOrderBiMap::value_type({stragety_id, order_no}, adjust_order_no));
  delegate_->CloseOrder(instrument, adjust_order_no, direction, position_effect,
                        price_type, price, quantity);
}

void FollowStragetyService::CancelOrder(const std::string& stragety_id,
                                    const std::string& order_no) {
  auto it = stragety_order_.left.find(StragetyOrder{stragety_id, order_no});
  if (it != stragety_order_.left.end()) {
    delegate_->CancelOrder(order_no);
  }
}
