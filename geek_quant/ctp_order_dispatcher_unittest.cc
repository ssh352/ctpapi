#include "gtest/gtest.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"
#include "geek_quant/ctp_order_dispatcher.h"

CThostFtdcOrderField MakeRtnOrderField(
    TThostFtdcOrderStatusType order_status,
    TThostFtdcOffsetFlagType oc_flag = THOST_FTDC_OF_Open,
    TThostFtdcVolumeType volume_traded = 0,
    TThostFtdcVolumeType volume_total = 10,
    TThostFtdcDirectionType direction = THOST_FTDC_D_Buy,
    TThostFtdcOrderRefType order_no = "00001",
    TThostFtdcPriceType limit_price = 1234.1,
    TThostFtdcInstrumentIDType instrument = "abc") {
  CThostFtdcOrderField filed = {0};
  strcpy(filed.InstrumentID, instrument);
  strcpy(filed.OrderRef, order_no);
  filed.OrderStatus = order_status;
  filed.Direction = direction;
  filed.VolumeTotal = volume_total;
  filed.LimitPrice = limit_price;
  filed.CombOffsetFlag[0] = oc_flag;
  filed.VolumeTraded = volume_traded;
  return filed;
}

TEST(CtpOrderDispatcherTest, OpenOrder) {
  CtpOrderDispatcher dispatcher;
  {
    auto order =
        dispatcher.HandleRtnOrder(MakeRtnOrderField(THOST_FTDC_OST_Unknown));
    EXPECT_TRUE(order);
    EXPECT_EQ("abc", order->instrument);
    EXPECT_EQ("00001", order->order_no);
    EXPECT_EQ(kOSOpening, order->order_status);
    EXPECT_EQ(1234.1, order->order_price);
    EXPECT_EQ(10, order->volume);
  }

  {
    auto order = dispatcher.HandleRtnOrder(
        MakeRtnOrderField(THOST_FTDC_OST_NoTradeQueueing));
    EXPECT_FALSE(order);
  }

  {
    auto order = dispatcher.HandleRtnOrder(
        MakeRtnOrderField(THOST_FTDC_OST_NoTradeQueueing));
    EXPECT_FALSE(order);
  }

  {
    auto order = dispatcher.HandleRtnOrder(MakeRtnOrderField(
        THOST_FTDC_OST_PartTradedQueueing, THOST_FTDC_OF_Open, 1, 9));
    EXPECT_TRUE(order);
    EXPECT_EQ(1, order->volume);
    EXPECT_EQ(kOSOpened, order->order_status);
  }

  {
    auto order = dispatcher.HandleRtnOrder(MakeRtnOrderField(
        THOST_FTDC_OST_PartTradedQueueing, THOST_FTDC_OF_Open, 5, 5));
    EXPECT_TRUE(order);
    EXPECT_EQ(4, order->volume);
    EXPECT_EQ(kOSOpened, order->order_status);
  }
  {
    auto order = dispatcher.HandleRtnOrder(
        MakeRtnOrderField(THOST_FTDC_OST_AllTraded, THOST_FTDC_OF_Open, 10, 0));
    EXPECT_TRUE(order);
    EXPECT_EQ(5, order->volume);
    EXPECT_EQ(kOSOpened, order->order_status);
  }
}

TEST(CtpOrderDispatcherTest, CloseOrder) {
  CtpOrderDispatcher dispatcher;
  {
    auto order = dispatcher.HandleRtnOrder(
        MakeRtnOrderField(THOST_FTDC_OST_Unknown, THOST_FTDC_OF_Close));
    EXPECT_TRUE(order);
    EXPECT_EQ("abc", order->instrument);
    EXPECT_EQ("00001", order->order_no);
    EXPECT_EQ(kOSCloseing, order->order_status);
    EXPECT_EQ(1234.1, order->order_price);
    EXPECT_EQ(10, order->volume);
  }

  {
    auto order = dispatcher.HandleRtnOrder(
        MakeRtnOrderField(THOST_FTDC_OST_NoTradeQueueing, THOST_FTDC_OF_Close));
    EXPECT_FALSE(order);
  }

  {
    auto order = dispatcher.HandleRtnOrder(
        MakeRtnOrderField(THOST_FTDC_OST_NoTradeQueueing, THOST_FTDC_OF_Close));
    EXPECT_FALSE(order);
  }

  {
    auto order = dispatcher.HandleRtnOrder(MakeRtnOrderField(
        THOST_FTDC_OST_PartTradedQueueing, THOST_FTDC_OF_Close, 1, 9));
    EXPECT_TRUE(order);
    EXPECT_EQ(1, order->volume);
    EXPECT_EQ(kOSClosed, order->order_status);
  }

  {
    auto order = dispatcher.HandleRtnOrder(MakeRtnOrderField(
        THOST_FTDC_OST_PartTradedQueueing, THOST_FTDC_OF_Close, 5, 5));
    EXPECT_TRUE(order);
    EXPECT_EQ(4, order->volume);
    EXPECT_EQ(kOSClosed, order->order_status);
  }
  {
    auto order = dispatcher.HandleRtnOrder(MakeRtnOrderField(
        THOST_FTDC_OST_AllTraded, THOST_FTDC_OF_Close, 10, 0));
    EXPECT_TRUE(order);
    EXPECT_EQ(5, order->volume);
    EXPECT_EQ(kOSClosed, order->order_status);
  }
}
// THOST_FTDC_OST_Canceled

TEST(CtpOrderDispatcherTest, CancelOrder) {
  CtpOrderDispatcher dispatcher;
  {
    auto order =
        dispatcher.HandleRtnOrder(MakeRtnOrderField(THOST_FTDC_OST_Unknown));
    EXPECT_TRUE(order);
    EXPECT_EQ("abc", order->instrument);
    EXPECT_EQ("00001", order->order_no);
    EXPECT_EQ(kOSOpening, order->order_status);
    EXPECT_EQ(1234.1, order->order_price);
    EXPECT_EQ(10, order->volume);
  }

  {
    auto order = dispatcher.HandleRtnOrder(
        MakeRtnOrderField(THOST_FTDC_OST_NoTradeQueueing));
    EXPECT_FALSE(order);
  }

  {
    auto order = dispatcher.HandleRtnOrder(
        MakeRtnOrderField(THOST_FTDC_OST_NoTradeQueueing));
    EXPECT_FALSE(order);
  }

  {
    auto order =
        dispatcher.HandleRtnOrder(MakeRtnOrderField(THOST_FTDC_OST_Canceled));
    EXPECT_TRUE(order);
    EXPECT_EQ("00001", order->order_no);
  }
}
