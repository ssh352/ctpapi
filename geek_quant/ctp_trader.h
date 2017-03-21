#ifndef CTP_TRADER_H
#define CTP_TRADER_H
#include "geek_quant/caf_defines.h"
#include <fstream>
#include <boost/lexical_cast.hpp>
#include "geek_quant/ctp_order_dispatcher.h"

#include "ctpapi/ThostFtdcTraderApi.h"
class CtpTrader : public CThostFtdcTraderSpi {
 public:
  class Delegate {
   public:
    virtual void OnLogon() = 0;
    virtual void OnRtnOrderData(CThostFtdcOrderField* order) = 0;
  };
  //////////////////////////////////////////////////////////////////////////
  // CtpTrader
  CtpTrader(Delegate* delegate, const char* folw_path) : delegate_(delegate) {
    cta_api_ = CThostFtdcTraderApi::CreateFtdcTraderApi(folw_path);
  }

  void LoginServer(const std::string& front_server,
                   const std::string& broker_id,
                   const std::string& user_id,
                   const std::string& password) {
    broker_id_ = broker_id;
    user_id_ = user_id;
    password_ = password;

    cta_api_->RegisterSpi(this);
    char front_server_buffer[256] = {0};
    strcpy(front_server_buffer, front_server.c_str());

    cta_api_->RegisterFront(front_server_buffer);
    // api_->SubscribePublicTopic(THOST_TERT_RESTART);
    cta_api_->SubscribePublicTopic(THOST_TERT_RESUME);
    cta_api_->SubscribePrivateTopic(THOST_TERT_RESTART);
    cta_api_->Init();
  }

  void OrderInsert(CThostFtdcInputOrderField order) {
    strcpy(order.BrokerID, broker_id_.c_str());
    strcpy(order.UserID, user_id_.c_str());
    strcpy(order.InvestorID, user_id_.c_str());
    cta_api_->ReqOrderInsert(&order, 1);
  }

  void OrderAction(CThostFtdcInputOrderActionField order) {
    strcpy(order.BrokerID, broker_id_.c_str());
    strcpy(order.UserID, user_id_.c_str());
    strcpy(order.InvestorID, user_id_.c_str());
    cta_api_->ReqOrderAction(&order, 1);
  }

  virtual void OnFrontConnected() override {
    std::cout << "OnFrontConnected\n";
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, broker_id_.c_str());
    strcpy(req.UserID, user_id_.c_str());
    strcpy(req.Password, password_.c_str());
    int iResult = cta_api_->ReqUserLogin(&req, 0);
  }

  virtual void OnFrontDisconnected(int nReason) override {}

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override {
    if (pRspInfo->ErrorID == 0) {
      delegate_->OnLogon();
      std::cout << "OnRspUserLogin\n";
    } else {
      std::cout << "User Login Error:" << pRspInfo->ErrorMsg << "\n";
    }
  }

  virtual void OnRtnOrder(CThostFtdcOrderField* pOrder) override {
    delegate_->OnRtnOrderData(pOrder);
  }

  virtual void OnRtnTrade(CThostFtdcTradeField* pTrade) {
    std::cout << __FUNCTION__ << "\n";
  }

  virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                   CThostFtdcRspInfoField* pRspInfo) {
    std::cout << __FUNCTION__ << "ErrorID:" << pRspInfo->ErrorID
              << ","
                 "ErrorMsg:"
              << pRspInfo->ErrorMsg << "\n";
  }

  virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction,
                                   CThostFtdcRspInfoField* pRspInfo) {
    std::cout << __FUNCTION__ << "\n";
  }

  virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                CThostFtdcRspInfoField* pRspInfo,
                                int nRequestID,
                                bool bIsLast) {
    std::cout << __FUNCTION__ << "\n";
  };

 private:
  CThostFtdcTraderApi* cta_api_;
  CThostFtdcTraderApi* follower_api_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  Delegate* delegate_;
};

#endif /* CTP_TRADER_H */
