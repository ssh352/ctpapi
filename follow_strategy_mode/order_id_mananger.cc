#include "order_id_mananger.h"
#include <boost/lexical_cast.hpp>
#include "follow_strategy_mode/string_util.h"

const char kDummySession[] = "DummySession";

OrderIdMananger::OrderIdMananger(std::string master_account_id,
                                 int start_order_id_seq)
    : master_account_id_(master_account_id) {
  start_order_id_seq_ = start_order_id_seq;
}

OrderData OrderIdMananger::AdjustOrder(OrderData&& order) {
  if (order.user_product_info_ != kStrategyUserProductInfo) {
    auto key =
        order.order_id_ + boost::lexical_cast<std::string>(order.session_id_);
    if (session_corr_order_ids_.find(key) != session_corr_order_ids_.end()) {
      order.order_id_ =
          boost::lexical_cast<std::string>(session_corr_order_ids_[key]);
    } else {
      int order_id = static_cast<int>(session_corr_order_ids_.size()) +
                     start_order_id_seq_ +
                     (order.account_id() == master_account_id_ ? 0 : 10000);
      session_corr_order_ids_[key] = order_id;
      order.order_id_ = boost::lexical_cast<std::string>(order_id);
    }
  }
  return order;
}

std::string OrderIdMananger::GenerateOrderId() {
  int order_id =
      static_cast<int>(session_corr_order_ids_.size()) + start_order_id_seq_;

  session_corr_order_ids_[kDummySession,
                          boost::lexical_cast<std::string>(order_id)] =
      order_id;
  return boost::lexical_cast<std::string>(order_id);
}

std::string OrderIdMananger::GetOrderId(const std::string& instrument,
                                        OrderDirection order_direction) {
  auto key = instrument + boost::lexical_cast<std::string>(
                              static_cast<int>(order_direction));

  if (session_corr_order_ids_.find(key) == session_corr_order_ids_.end()) {
    session_corr_order_ids_[key] =
        static_cast<int>(session_corr_order_ids_.size());
    return boost::lexical_cast<std::string>(session_corr_order_ids_[key]);
  }
  return boost::lexical_cast<std::string>(session_corr_order_ids_[key]);
}
