#include <boost/archive/binary_iarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include "follow_strategy_mode/defines.h"
#include "follow_strategy_mode/follow_strategy_service.h"
#include "follow_strategy_mode/print_portfolio_helper.h"
#include "follow_strategy_mode/string_util.h"
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
    // std::cout << "Open Order " << instrument << "," << order_no << ","
    //           << static_cast<int>(direction) << ","
    //           << static_cast<int>(price_type) << "," << price << "," <<
    //           quantity
    //           << "\n";

      if (order_no == "1208") {
        std::cout << "Oops!\n";
      }
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
  std::vector<std::pair<int, int> > res;
  DummyServiceDelegate delegate;
  FollowStragetyService follow_strategy_service("38030022", "120350655",
                                                &delegate, 1000);
  std::vector<OrderData> view_orders;
  try {
    std::ifstream file(
//         "c://Users//yjqpro//Desktop//fd//120350655-20170609-204000.bin",
         // "c://Users//yjqpro//Desktop//fd//120350655-20170612-083501.bin",
      // "c://Users//yjqpro//Desktop//fd//120350655-20170609-083501.bin",
      "c://Users//yjqpro//Desktop//fd//120350655-20170615-113242.bin",
        std::ios_base::binary);
    boost::archive::binary_iarchive ia(file);
    std::string account_id;
    ia >> account_id;
    size_t size = 0;
    ia >> size;
    std::vector<OrderPosition> positions;
    for (size_t i = 0; i < size; ++i) {
      OrderPosition position;
      ia >> position;
      positions.push_back(std::move(position));
    }
    follow_strategy_service.InitPositions(account_id, positions);
    std::vector<OrderData> orders;
    ia >> size;
    for (size_t i = 0; i < size; ++i) {
      OrderData order;
      ia >> order;
      orders.push_back(std::move(order));
    }
    ia >> account_id;
    positions.clear();
    ia >> size;
    for (size_t i = 0; i < size; ++i) {
      OrderPosition position;
      ia >> position;
      positions.push_back(std::move(position));
    }
    follow_strategy_service.InitPositions(account_id, positions);
    follow_strategy_service.InitRtnOrders(orders);
    ia >> size;
    orders.clear();
    for (size_t i = 0; i < size; ++i) {
      OrderData order;
      ia >> order;
      orders.push_back(std::move(order));
    }
    follow_strategy_service.InitRtnOrders(orders);

    orders.clear();
    for (int i = 0; i < 9999; ++i) {
      OrderData order;
      ia >> order;
      view_orders.push_back(order);
      // if (order.user_product_info() == kStrategyUserProductInfo) {
      //   order.order_id_ = boost::lexical_cast<std::string>(
      //       boost::lexical_cast<int>(order.order_id()) + 9000);
      // }

      //       if (order.account_id() == "38030022" && order.instrument() ==
      //       "a1709" && order.datetime() == "10:06:58") {
      //         std::cout << "Oops!\n";
      //       }
      // if (order.account_id() == "38030022" && order.order_id() == "1046") {
      //   std::cout << "Oops!\n";
      // }

      if (order.account_id() == "38030022" && order.order_id() == "1451") {
        std::cout << "Oops! " << order.order_id() << "\n";
      }
      

      if (order.order_id() == "1208") {
        std::cout << "Oops!\n";
      }

      follow_strategy_service.HandleRtnOrder(std::move(order));
    }

  } catch (boost::archive::archive_exception& err) {
    std::cout << "Done" << err.what() << "\n";
  }

  /*
    std::ofstream f("d:\\orders2.csv");
    for (auto order : view_orders) {
      f << order.account_id() << "," << order.order_id() << "," << order.instrument() << "," << order.datetime() << "<" << order.user_product_info() << "\n";
    }
  */

  return 0;
}
