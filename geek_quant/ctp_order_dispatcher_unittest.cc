#include "gtest/gtest.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"
#include "geek_quant/ctp_order_dispatcher.h"

CThostFtdcOrderField MakeRtnOrderField(
    TThostFtdcOrderStatusType order_status,
    TThostFtdcVolumeType volume_traded = 0,
    TThostFtdcVolumeType volume_total = 10,
    TThostFtdcOffsetFlagType oc_flag = THOST_FTDC_OF_Open,
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
    auto order = dispatcher.HandleRtnOrder(
        MakeRtnOrderField(THOST_FTDC_OST_PartTradedQueueing, 1, 9));
    EXPECT_TRUE(order);
    EXPECT_EQ(1, order->volume);
    EXPECT_EQ(kOSOpened, order->order_status);
  }

  {
    auto order = dispatcher.HandleRtnOrder(
        MakeRtnOrderField(THOST_FTDC_OST_PartTradedQueueing, 5, 5));
    EXPECT_TRUE(order);
    EXPECT_EQ(4, order->volume);
    EXPECT_EQ(kOSOpened, order->order_status);
  }
  {
    auto order = dispatcher.HandleRtnOrder(
        MakeRtnOrderField(THOST_FTDC_OST_AllTraded, 10, 0));
    EXPECT_TRUE(order);
    EXPECT_EQ(5, order->volume);
    EXPECT_EQ(kOSOpened, order->order_status);
  }
}
