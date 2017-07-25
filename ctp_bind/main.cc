#include <boost/make_shared.hpp>
#include <iostream>
#include <thread>
#include <boost/format.hpp>
#include "ctpapi/ThostFtdcMdApi.h"
#include "ctpapi/ThostFtdcTraderApi.h"

#include "md.h"
#include "md_observer.h"
#include "trader.h"

int main(int argc, char* argv[]) {
  ctp_bind::Trader trader("tcp://180.168.146.187:10000", "9999", "053867",
                          "8661188");
  trader.InitAsio();

  trader.Connect([&trader](CThostFtdcRspUserLoginField* rsp_field,
                           CThostFtdcRspInfoField* rsp_info) {
    CThostFtdcInputOrderField field{0};
    strcpy(field.BrokerID, rsp_field->BrokerID);
    strcpy(field.UserID, rsp_field->UserID);
    strcpy(field.InvestorID, rsp_field->UserID);

    std::string order_ref = "100";
    {
      strcpy(field.InstrumentID, "c1709");
      strcpy(field.OrderRef, order_ref.c_str());
      field.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
      field.Direction = true ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;
      field.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
      strcpy(field.CombHedgeFlag, "1");
      field.LimitPrice = 1661.0;
      field.VolumeTotalOriginal = 1;
      field.TimeCondition = THOST_FTDC_TC_GFD;
      strcpy(field.GTDDate, "");
      field.VolumeCondition = THOST_FTDC_VC_AV;
      field.MinVolume = 1;
      field.ContingentCondition = THOST_FTDC_CC_Immediately;
      field.StopPrice = 0;
      field.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
      field.IsAutoSuspend = 0;
      field.UserForceClose = 0;

      trader.Request(&CThostFtdcTraderApi::ReqOrderInsert, &field,
                     // callback
                     [=](CThostFtdcInputOrderField* order,
                         CThostFtdcRspInfoField* rsp_info, bool is_last) {
                       std::cout << "call1111\n";

                     });
    }

    trader.SubscribeRtnOrder(
        [=,&trader](boost::shared_ptr<CThostFtdcOrderField> order) {
          if (order->FrontID == rsp_field->FrontID &&
              order->SessionID == rsp_field->SessionID) {
            trader.CancelOrder(str(boost::format("%d:%d:%s") % order->FrontID %
                                   order->SessionID % order->OrderRef));
          }
        });
  });

  trader.Run();

  /*
  {
    CThostFtdcInputOrderField field;
    CThostFtdcRspInfoField rsp_info;
    trader.OnRspOrderInsert(&field, &rsp_info, 0, true);
  }

  {
    CThostFtdcInputOrderActionField field;
    trader.Request(&CThostFtdcTraderApi::ReqOrderAction, &field,
                   // callback
                   [=](CThostFtdcInputOrderActionField* order,
                       CThostFtdcRspInfoField* rsp_info,
                       bool is_last) { std::cout << "OnRspOrderAction\n"; });
    CThostFtdcRspInfoField rsp_info;
    trader.OnRspOrderAction(&field, &rsp_info, 1, true);
  }

  {
    CThostFtdcInputOrderActionField field;
    trader.Request(&CThostFtdcTraderApi::ReqOrderAction, &field,
                   // callback
                   [=](CThostFtdcInputOrderActionField* field,
                       CThostFtdcRspInfoField* rsp_info,
                       bool is_last) { std::cout << "OnRspError\n"; });
    CThostFtdcRspInfoField rsp_info;
    trader.OnRspError(&rsp_info, 2, true);
  }
  */
  /*

  {
    ctp_bind::Md md("tcp://180.168.146.187:10011", "9999", "053867", "8661188");
    md.InitAsio();

    auto c = ctp_bind::MdObserver::Create(&md);
    md.Connect([=, &md](CThostFtdcRspUserLoginField* field,
                        CThostFtdcRspInfoField* rsp_info) {
      std::string str;
      c->Subscribe({"c1709", "m1709"},
                   [=](const CThostFtdcDepthMarketDataField* field) {
                     std::cout << "[" << field->InstrumentID << "]"
                               << " Bid:" << field->BidPrice1
                               << ", Ask:" << field->AskPrice1 << "\n";
                   });

      c->Unsubscribe({"m1709"});
    });

    md.Run();
  }
  */
  // trader.Run();

  // virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField
  // *pDepthMarketData) {}; CThostFtdcSpecificInstrumentField
  // *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
  // bool bIsLast
  //   md.subscribe(&CThostFtdcMdApi::SubscribeForQuoteRsp, {}, 10,
  //   [=](CThostFtdcDepthMarketDataField *pDepthMarketData) {
  //
  //   });

  //   md.subscribe();
  return 0;
}
