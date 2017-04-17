#include <boost/archive/binary_iarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include "follow_strategy_mode/defines.h"
#include "follow_strategy_mode/follow_strategy_service.h"
#include "follow_trade_server/binary_serialization.h"

class DummyServiceDelegate : public FollowStragetyService::Delegate {
 public:
  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         OrderPriceType price_type,
                         double price,
                         int quantity) override {
    // throw std::logic_error("The method or operation is not implemented.");
    std::cout << "Open Order " << instrument << "," << order_no << ","
              << static_cast<int>(direction) << ","
              << static_cast<int>(price_type) << "," << price << ","
              << quantity << "\n";
  }

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          OrderPriceType price_type,
                          double price,
                          int quantity) override {
    // throw std::logic_error("The method or operation is not implemented.");
  }

  virtual void CancelOrder(const std::string& order_no) override {
    //  throw std::logic_error("The method or operation is not implemented.");
  }
};

int main(int argc, char** argv) {
  DummyServiceDelegate delegate;
  FollowStragetyService follow_strategy_service("38030022", "181006", &delegate,
                                                1000);
  try {
    std::ifstream file("c:\\Users\\yjqpro\\Desktop\\181006-20170417-080021.bin",
                       std::ios_base::binary);
    boost::archive::binary_iarchive ia(file);
    std::string account_id;
    ia >> account_id;
    std::vector<OrderPosition> positions;
    for (int i = 0; i < 4; ++i) {
      OrderPosition position;
      ia >> position;
      positions.push_back(std::move(position));
    }
    follow_strategy_service.InitPositions(account_id, positions);
    std::vector<OrderData> orders;
    for (int i = 0; i < 278; ++i) {
      OrderData order;
      ia >> order;
      orders.push_back(std::move(order));
    }
    follow_strategy_service.InitRtnOrders(orders);
    ia >> account_id;
    positions.clear();
    follow_strategy_service.InitPositions(account_id, positions);
    // for (int i = 0; i < 4; ++i) {
    //   OrderPosition position;
    //   ia >> position;
    //   positions.push_back(std::move(position));
    // }
    orders.clear();
    follow_strategy_service.InitRtnOrders(orders);

    orders.clear();
    for (int i = 0; i < 497; ++i) {
      OrderData order;
      ia >> order;
      if (order.account_id() == "38030022" && order.instrument() == "a1709" && order.datetime() == "10:05:56") {
        std::cout << "Oops!\n";
      }
      if (order.account_id() == "38030022" && order.instrument() == "a1709" && order.datetime() == "10:06:58") {
        std::cout << "Oops!\n";
      }
      // if (order.account_id() == "38030022" && order.order_id() == "1046") {
      //   std::cout << "Oops!\n";
      // }
      follow_strategy_service.HandleRtnOrder(std::move(order));
    }
  } catch (boost::archive::archive_exception& err) {
    std::cout << "Done" << err.what() << "\n";
  }
  return 0;
}
