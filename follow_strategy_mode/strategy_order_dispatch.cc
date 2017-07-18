#include "strategy_order_dispatch.h"
#include <boost/lexical_cast.hpp>

void StrategyOrderDispatch::OpenOrder(const std::string& strategy_id,
                                      const std::string& instrument,
                                      const std::string& order_no,
                                      OrderDirection direction,
                                      OrderPriceType price_type,
                                      double price,
                                      int quantity) {
  std::string adjust_order_no =
      boost::lexical_cast<std::string>(stragety_orders_.size() + 1);
  stragety_orders_.insert(
      StragetyOrderBiMap::value_type({strategy_id, order_no}, adjust_order_no));
  enter_order_->OpenOrder(instrument, adjust_order_no, direction, price_type,
                          price, quantity);
}

void StrategyOrderDispatch::CloseOrder(const std::string& strategy_id,
                                       const std::string& instrument,
                                       const std::string& order_no,
                                       OrderDirection direction,
                                       PositionEffect position_effect,
                                       OrderPriceType price_type,
                                       double price,
                                       int quantity) {
  std::string adjust_order_no =
      boost::lexical_cast<std::string>(stragety_orders_.size() + 1);
  stragety_orders_.insert(
      StragetyOrderBiMap::value_type({strategy_id, order_no}, adjust_order_no));
  enter_order_->CloseOrder(instrument, adjust_order_no, direction,
                           position_effect, price_type, price, quantity);
}

void StrategyOrderDispatch::CancelOrder(const std::string& strategy_id,
                                        const std::string& order_no) {
  auto it = stragety_orders_.left.find(StragetyOrder{strategy_id, order_no});
  if (it != stragety_orders_.left.end()) {
    enter_order_->CancelOrder(order_no);
  }
}

void StrategyOrderDispatch::RtnOrder(OrderData order) {
  auto it = stragety_orders_.right.find(order.order_id());
  if (it != stragety_orders_.right.end()) {
    order.order_id_ = it->second.order_id;
    rtn_order_observers_[it->second.strategy_id]->RtnOrder(std::move(order));
  } else {
    // Exception or maybe muanul control
  }
}

void StrategyOrderDispatch::SubscribeEnterOrderObserver(
    EnterOrderObserver* observer) {
  enter_order_ = observer;
}

void StrategyOrderDispatch::SubscribeRtnOrderObserver(
    const std::string& strategy_id,
    std::shared_ptr<RtnOrderObserver> observer) {
  if (rtn_order_observers_.find(strategy_id) == rtn_order_observers_.end()) {
    rtn_order_observers_.insert({strategy_id, observer});
  }
}
