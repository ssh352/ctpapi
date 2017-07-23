#include <iostream>
#include "ctpapi/ThostFtdcMdApi.h"
#include "ctpapi/ThostFtdcTraderApi.h"
#include "trader.h"

int main(int argc, char* argv[]) {
  ctp_bind::Trader trader;
  {
    CThostFtdcInputOrderField field;
    trader.Request(
        &CThostFtdcTraderApi::ReqOrderInsert, &field,
        // callback
        [=](CThostFtdcInputOrderField* order, CThostFtdcRspInfoField* rsp_info,
            bool is_last) { std::cout << "call1111\n"; });
  }

  {
    CThostFtdcInputOrderField field;
    CThostFtdcRspInfoField rsp_info;
    trader.OnRspOrderInsert(&field, &rsp_info, 0, true);
  }

  {
    CThostFtdcInputOrderActionField field;
    trader.Request(
        &CThostFtdcTraderApi::ReqOrderAction, &field,
        // callback
        [=](CThostFtdcInputOrderActionField* order, CThostFtdcRspInfoField* rsp_info,
            bool is_last) { std::cout << "OnRspOrderAction\n"; });
    CThostFtdcRspInfoField rsp_info;
    trader.OnRspOrderAction(&field, &rsp_info, 1, true);
  }

  {
    CThostFtdcInputOrderActionField field;
    trader.Request(
        &CThostFtdcTraderApi::ReqOrderAction, &field,
        // callback
        [=](CThostFtdcInputOrderActionField* field, CThostFtdcRspInfoField* rsp_info,
            bool is_last) { 
      std::cout << "OnRspError\n"; }
    );
    CThostFtdcRspInfoField rsp_info;
    trader.OnRspError(&rsp_info, 2, true);
  }

  trader.Run();

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
