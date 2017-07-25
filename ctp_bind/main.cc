#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <iostream>
#include <thread>
#include "ctpapi/ThostFtdcMdApi.h"
#include "ctpapi/ThostFtdcTraderApi.h"

#include "api_struct.h"
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
      trader.InputOrder("c1709", PositionEffect::kOpen, OrderDirection::kBuy,
                        1640, 1, order_ref);
    }
    trader.SubscribeRtnOrder([=, &trader](boost::shared_ptr<OrderField> order) {
      if (order->addition_info == order_ref) {
        trader.CancelOrder(order->order_id);
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
