#include "gtest/gtest.h"
#include "geek_quant/instrument_follow_fixture.h"

class InstrumentFollowSyncOrdersFixture : public InstrumentFollowBaseFixture {
 protected:
  virtual void SetUp() override {}
};

TEST_F(InstrumentFollowSyncOrdersFixture, SyncOpenOrderCase1) {
  (void)TraderOrderRtn("0001", kOSOpening, 10);
  EXPECT_FALSE(instrument_follow.TryCompleteSyncOrders());
  EXPECT_TRUE(instrument_follow.TryCompleteSyncOrders());
  {
    auto ret = TraderOrderRtn("0001", kOSOpened, 10);
    EnterOrderData& enter_order = ret.first;
    std::vector<std::string>& cancel_order_no_list = ret.second;
    EXPECT_EQ("", enter_order.order_no);
    EXPECT_EQ(0, cancel_order_no_list.size());
  }
}

TEST_F(InstrumentFollowSyncOrdersFixture, SyncOpenOrderCase2) {
  OpenAndFillOrder(10, 10, 5);
  EXPECT_FALSE(instrument_follow.TryCompleteSyncOrders());
  EXPECT_TRUE(instrument_follow.TryCompleteSyncOrders());
  auto ret = TraderOrderRtn("0002", kOSCloseing, 10, kODSell);
  EnterOrderData& enter_order = ret.first;
  std::vector<std::string>& cancel_order_no_list = ret.second;
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(5, enter_order.volume);
}

TEST_F(InstrumentFollowSyncOrdersFixture, SyncOpenOrderCase3) {
  OpenAndFillOrder(10, 0, 0);
  EXPECT_FALSE(instrument_follow.TryCompleteSyncOrders());
  EXPECT_TRUE(instrument_follow.TryCompleteSyncOrders());
  auto ret = TraderOrderRtn("0002", kOSOpenCanceled, 10, kODSell);
  EnterOrderData& enter_order = ret.first;
  std::vector<std::string>& cancel_order_no_list = ret.second;
  EXPECT_EQ("", enter_order.order_no);
  EXPECT_EQ(0, cancel_order_no_list.size());
}

TEST_F(InstrumentFollowSyncOrdersFixture, SyncOpenOrderCase4) {
  OpenAndFillOrder(10, 10, 5);
  EXPECT_FALSE(instrument_follow.TryCompleteSyncOrders());
  EXPECT_TRUE(instrument_follow.TryCompleteSyncOrders());
  auto ret = TraderOrderRtn("0002", kOSCloseing, 10, kODSell);
  EnterOrderData& enter_order = ret.first;
  std::vector<std::string>& cancel_order_no_list = ret.second;
  EXPECT_EQ("0002", enter_order.order_no);
  EXPECT_EQ(5, enter_order.volume);
  EXPECT_EQ(1, cancel_order_no_list.size());
  EXPECT_EQ("0001", cancel_order_no_list.at(0));
}
