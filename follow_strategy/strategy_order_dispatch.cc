#include "strategy_order_dispatch.h"
#include <boost/lexical_cast.hpp>
#include <boost/log/common.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include "follow_strategy/logging_defines.h"

void StrategyOrderDispatch::OpenOrder(const std::string& strategy_id,
                                      const std::string& instrument,
                                      const std::string& order_id,
                                      OrderDirection direction,
                                      double price,
                                      int quantity) {
  enter_order_->OpenOrder(strategy_id, instrument, order_id, direction, price,
                          quantity);
  BOOST_LOG(log_) << boost::log::add_value("strategy_id", strategy_id)
                  << "OpenOrder:" << instrument << "," << order_id << ","
                  << (direction == OrderDirection::kBuy ? "B" : "S") << ","
                  << price << "," << quantity;
}

void StrategyOrderDispatch::CloseOrder(const std::string& strategy_id,
                                       const std::string& instrument,
                                       const std::string& order_id,
                                       OrderDirection direction,
                                       PositionEffect position_effect,
                                       double price,
                                       int quantity) {
  enter_order_->CloseOrder(strategy_id, instrument, order_id, direction,
                           position_effect, price, quantity);
  BOOST_LOG(log_) << "CloseOrder:"
                  << boost::log::add_value("strategy_id", strategy_id)
                  << instrument << "," << order_id << ","
                  << (direction == OrderDirection::kBuy ? "B" : "S") << ","
                  << price << "," << quantity;
}

void StrategyOrderDispatch::CancelOrder(const std::string& strategy_id,
                                        const std::string& order_id) {
  enter_order_->CancelOrder(strategy_id, order_id);
  BOOST_LOG(log_) << "CancelOrder:"
                  << boost::log::add_value("strategy_id", strategy_id)
                  << order_id;
}

void StrategyOrderDispatch::RtnOrder(
    const std::shared_ptr<const OrderField>& order) {
  BOOST_LOG(log_) << boost::log::add_value("strategy_id", order->strategy_id)
                  << "RtnOrder:" << order->instrument_id << ","
                  << order->order_id << ","
                  << (order->direction == OrderDirection::kBuy ? "B" : "S")
                  << "," << order->input_price;
  auto it = rtn_order_observers_.find(order->strategy_id);
  if (it != rtn_order_observers_.end()) {
    it->second->RtnOrder(order);
  } else {
    for (auto o : rtn_order_observers_) {
      o.second->RtnOrder(order);
    }
  }
}

void StrategyOrderDispatch::SubscribeEnterOrderObserver(
    StrategyEnterOrderObservable::Observer* observer) {
  enter_order_ = observer;
}

void StrategyOrderDispatch::SubscribeRtnOrderObserver(
    const std::string& strategy_id,
    std::shared_ptr<RtnOrderObserver> observer) {
  if (rtn_order_observers_.find(strategy_id) == rtn_order_observers_.end()) {
    rtn_order_observers_.insert({strategy_id, observer});
  }
}
