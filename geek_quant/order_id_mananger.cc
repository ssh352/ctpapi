#include "order_id_mananger.h"
#include <boost/lexical_cast.hpp>

OrderIdMananger::OrderIdMananger() {
  start_order_id_seq_ = 1000;
}

OrderData OrderIdMananger::AdjustOrder(OrderData&& order) {
  if (order.user_product_info_ != kStrategyUserProductInfo) {
    auto key = std::make_pair(order.order_id_, order.session_id_);
    if (session_corr_order_ids_.find(key) != session_corr_order_ids_.end()) {
      order.order_id_ = boost::lexical_cast<std::string>(session_corr_order_ids_[key]);
    } else {
      int order_id = static_cast<int>(session_corr_order_ids_.size()) +
                     start_order_id_seq_;
      session_corr_order_ids_[key] = order_id;
      order.order_id_ = boost::lexical_cast<std::string>(order_id);
    }
  }
  return order;
}
