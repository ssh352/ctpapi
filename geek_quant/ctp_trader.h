#ifndef CTP_TRADER_H
#define CTP_TRADER_H
#include "geek_quant/caf_defines.h"
#include <fstream>
#include <boost/lexical_cast.hpp>
#include "geek_quant/ctp_order_dispatcher.h"

#include "ctpapi/ThostFtdcTraderApi.h"
class CtpTrader : public CThostFtdcTraderSpi {
 public:
  CtpTrader();

  void LoginServer(const std::string& front_server,
                   const std::string& broker_id,
                   const std::string& user_id,
                   const std::string& password);

  void Join();

  virtual void OnFrontConnected() override;;

  virtual void OnFrontDisconnected(int nReason) override;

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override;

  virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                CThostFtdcRspInfoField* pRspInfo,
                                int nRequestID,
                                bool bIsLast);;

  virtual void OnRspOrderAction(
      CThostFtdcInputOrderActionField* pInputOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast);;

  virtual void OnRspBatchOrderAction(
      CThostFtdcInputBatchOrderActionField* pInputBatchOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast);;

  virtual void OnRspQryOrder(CThostFtdcOrderField* pOrder,
                             CThostFtdcRspInfoField* pRspInfo,
                             int nRequestID,
                             bool bIsLast);;

  virtual void OnRtnOrder(CThostFtdcOrderField* pOrder);

  virtual void OnRtnTrade(CThostFtdcTradeField* pTrade);

  virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                   CThostFtdcRspInfoField* pRspInfo);

  virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction,
                                   CThostFtdcRspInfoField* pRspInfo);;

  virtual void OnErrRtnBatchOrderAction(
      CThostFtdcBatchOrderActionField* pBatchOrderAction,
      CThostFtdcRspInfoField* pRspInfo);;

  virtual void OnRspQrySettlementInfo(
      CThostFtdcSettlementInfoField* pSettlementInfo,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast);;

  virtual void OnRspQryInvestorPosition(
      CThostFtdcInvestorPositionField* pInvestorPosition,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast);;

  virtual void OnRspQryInvestorPositionDetail(
      CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast);;

 private:
  CThostFtdcTraderApi* cta_api_;
  CThostFtdcTraderApi* follower_api_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  std::ofstream fstream_;
  std::ofstream outstream_;
  std::ofstream position_detail_fstream_;
  std::ofstream position_order_fstream_;
  CtpOrderDispatcher ctp_order_dispatcher_;
};

#endif /* CTP_TRADER_H */
