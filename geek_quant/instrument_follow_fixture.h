#ifndef FOLLOW_TRADE_UNITTEST_INSTRUMENT_FOLLOW_FIXTURE_H
#define FOLLOW_TRADE_UNITTEST_INSTRUMENT_FOLLOW_FIXTURE_H
#include "gtest/gtest.h"
#include "geek_quant/instrument_follow.h"
#include "geek_quant/caf_defines.h"
class InstrumentFollowBaseFixture : public testing::Test {
 public:
 protected:
  OrderRtnData MakeOrderRtnData(const std::string& order_no,
                                OrderDirection order_direction,
                                OrderStatus order_status,
                                int volume = 10,
                                double order_price = 1234.1,
                                const std::string&& instrument = "abc") {
    OrderRtnData order;
    order.order_no = std::move(order_no);
    order.order_direction = order_direction;
    order.order_status = order_status;
    order.order_price = order_price;
    order.volume = volume;
    order.instrument = std::move(instrument);
    return order;
  }

  virtual void TearDown() override {}

  void OpenAndFillOrder(int open_volume,
                        int fill_open_volume,
                        int fill_follow_open_volume,
                        OrderDirection order_direction = OrderDirection::kODBuy,
                        const std::string& order_no = "0001",
                        double order_price = 1234.1,
                        const std::string& instrument = "abc") {
    EnterOrderAndFill(kEOAOpen, open_volume, fill_open_volume,
                      fill_follow_open_volume, order_direction, order_no,
                      order_price, instrument);
  }

  void CloseAndFillOrder(
      int open_volume,
      int fill_open_volume,
      int fill_follow_open_volume,
      OrderDirection order_direction = OrderDirection::kODSell,
      const std::string& order_no = "0001",
      double order_price = 1234.1,
      const std::string& instrument = "abc") {
    EnterOrderAndFill(kEOAClose, open_volume, fill_open_volume,
                      fill_follow_open_volume, order_direction, order_no,
                      order_price, instrument);
  }

  void EnterOrderAndFill(EnterOrderAction enter_order_action,
                         int open_volume,
                         int fill_open_volume,
                         int fill_follow_open_volume,
                         OrderDirection order_direction,
                         const std::string& order_no,
                         double order_price,
                         const std::string& instrument) {
    TraderOrderRtn(order_no,
                   enter_order_action == kEOAOpen ? kOSOpening : kOSCloseing,
                   open_volume, order_direction, order_price, instrument);
    TraderOrderRtn(order_no,
                   enter_order_action == kEOAOpen ? kOSOpened : kOSClosed,
                   fill_open_volume, order_direction, order_price, instrument);

    FollowerOrderRtn(
        order_no, enter_order_action == kEOAOpen ? kOSOpening : kOSCloseing,
        fill_follow_open_volume, order_direction, order_price, instrument);

    FollowerOrderRtn(
        order_no, enter_order_action == kEOAOpen ? kOSOpened : kOSClosed,
        fill_follow_open_volume, order_direction, order_price, instrument);
  }

  std::pair<EnterOrderData, std::vector<std::string>> TraderOrderRtn(
      const std::string& order_no,
      OrderStatus order_status,
      int volume = 10,
      OrderDirection order_direction = kODBuy,
      double order_price = 1234.1,
      const std::string& instrument = "abc") {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;
    DoOrderRtn(true, order_no, instrument, order_status, order_direction,
               volume, order_price, &enter_order, &cancel_order_no_list);
    return std::make_pair(enter_order, cancel_order_no_list);
  }

  std::pair<EnterOrderData, std::vector<std::string>> FollowerOrderRtn(
      const std::string& order_no,
      OrderStatus order_status,
      int volume = 10,
      OrderDirection order_direction = kODBuy,
      double order_price = 1234.1,
      const std::string& instrument = "abc") {
    EnterOrderData enter_order;
    std::vector<std::string> cancel_order_no_list;
    DoOrderRtn(false, order_no, instrument, order_status, order_direction,
               volume, order_price, &enter_order, &cancel_order_no_list);
    return std::make_pair(enter_order, cancel_order_no_list);
  }

  void DoOrderRtn(bool trader,
                  const std::string& order_no,
                  const std::string& instrument,
                  OrderStatus order_status,
                  OrderDirection order_direction,
                  int volume,
                  double order_price,
                  EnterOrderData* enter_order,
                  std::vector<std::string>* cancel_order_no_list) {
    if (trader) {
      instrument_follow.HandleOrderRtnForTrader(
          MakeOrderRtnData(order_no, order_direction, order_status, volume),
          enter_order, cancel_order_no_list);
    } else {
      instrument_follow.HandleOrderRtnForFollow(
          MakeOrderRtnData(order_no, order_direction, order_status, volume),
          enter_order, cancel_order_no_list);
    }
  }

  InstrumentFollow instrument_follow;

 private:
  virtual void TestBody() override {}
};

#endif // FOLLOW_TRADE_UNITTEST_INSTRUMENT_FOLLOW_FIXTURE_H



