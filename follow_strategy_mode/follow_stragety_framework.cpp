#include "follow_stragety_framework.h"
#include <boost/lexical_cast.hpp>

void FollowStrategyFramework::OpenOrder(const std::string& instrument,
                                        const std::string& order_no,
                                        OrderDirection direction,
                                        OrderPriceType price_type,
                                        double price,
                                        int quantity) {}

void FollowStrategyFramework::CloseOrder(const std::string& instrument,
                                         const std::string& order_no,
                                         OrderDirection direction,
                                         PositionEffect position_effect,
                                         OrderPriceType price_type,
                                         double price,
                                         int quantity) {}

void FollowStrategyFramework::CancelOrder(const std::string& order_no) {}

void FollowStrategyFramework::HandleRtnOrder(OrderData rtn_order) {
  for (auto service : services_) {
    service->HandleRtnOrder();
  }
}

void FollowStrategyFramework::StragetyWrap::OpenOrder(
    const std::string& instrument,
    const std::string& order_no,
    OrderDirection direction,
    OrderPriceType price_type,
    double price,
    int quantity) {
  int int_order_id = boost::lexical_cast<int>(order_no) + start_seq_;
}

void FollowStrategyFramework::StragetyWrap::CloseOrder(
    const std::string& instrument,
    const std::string& order_no,
    OrderDirection direction,
    PositionEffect position_effect,
    OrderPriceType price_type,
    double price,
    int quantity) {
}

void FollowStrategyFramework::StragetyWrap::CancelOrder(
    const std::string& order_no) {
}
