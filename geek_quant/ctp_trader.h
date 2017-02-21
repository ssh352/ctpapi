#ifndef CTP_TRADER_H
#define CTP_TRADER_H

class CTpTrader : public CThostFtdcTraderSpi {
public:
  CTPTradingSpi(actor_system& system) : system_(system) {
    cta_api_ = CThostFtdcTraderApi::CreateFtdcTraderApi();
  }
 

  void LoginServer(const std::string& front_server,
                   const std::string& broker_id,
                   const std::string& user_id,
                   const std::string& password) {
    broker_ = actor_cast<strong_actor_ptr>(system_.spawn(CtpBroker));
    broker_id_ = broker_id;
    user_id_ = user_id;
    password_ = password;

    cta_api_->RegisterSpi(this);
    char front_server_buffer[256] = { 0 };
    strcpy(front_server_buffer, front_server.c_str());

    cta_api_->RegisterFront(front_server_buffer);
    // api_->SubscribePublicTopic(THOST_TERT_RESTART);
    cta_api_->SubscribePublicTopic(THOST_TERT_RESUME);
    cta_api_->SubscribePrivateTopic(THOST_TERT_QUICK);
    cta_api_->Init();
  }

  void Join() {
    cta_api_->Join();
  }

	virtual void OnFrontConnected() override {
    std::cout << "OnFrontConnected\n";
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, broker_id_.c_str());
    strcpy(req.UserID, user_id_.c_str());
    strcpy(req.Password, password_.c_str());
    int iResult = cta_api_->ReqUserLogin(&req, 0);
  };

  virtual void OnFrontDisconnected(int nReason) override {
    
  }

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                               CThostFtdcRspInfoField *pRspInfo,
                               int nRequestID,
                               bool bIsLast) override {

    anon_send(actor_cast<actor>(broker_), LoginAtom::value);
    if (pRspInfo->ErrorID == 0) {

      std::cout << "OnRspUserLogin\n";
      
      // CThostFtdcQrySettlementInfoField req = { 0 };
      // strcpy(req.BrokerID, broker_id_.c_str());
      // strcpy(req.TradingDay, "20170217");
      // api_->ReqQrySettlementInfo(&req, 0);
    } else {
      std::cout << "User Login Error:" << pRspInfo->ErrorMsg << "\n";
    }
  }

	///报单录入请求响应
	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::cout << __FUNCTION__ << "\n";
  };

	///报单操作请求响应
	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::cout << __FUNCTION__ << "\n";
  };

	///批量报单操作请求响应
	virtual void OnRspBatchOrderAction(CThostFtdcInputBatchOrderActionField *pInputBatchOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::cout << __FUNCTION__ << "\n";
  };
  
	///请求查询报单响应
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::cout << __FUNCTION__ << "\n";
  };

	///报单通知
	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder) {
    follow_strategy_->HandleRtnOrder(pOrder);
    if (pOrder->OrderSource == THOST_FTDC_OST_NoTradeQueueing) {
    }
  };
  
	///成交通知
	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade) {
    std::cout << __FUNCTION__ << "\n";
  };

	///报单录入错误回报
	virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) {
    std::cout << __FUNCTION__ << "\n";
  };

	///报单操作错误回报
	virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo) {
    std::cout << __FUNCTION__ << "\n";
  };

	///批量报单操作错误回报
	virtual void OnErrRtnBatchOrderAction(CThostFtdcBatchOrderActionField *pBatchOrderAction, CThostFtdcRspInfoField *pRspInfo) {
    std::cout << __FUNCTION__ << "\n";
  };

	virtual void OnRspQrySettlementInfo(CThostFtdcSettlementInfoField *pSettlementInfo, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {
    std::cout << pSettlementInfo->Content << "\n";
  };
private:
  actor_system& system_;
  CThostFtdcTraderApi* cta_api_;
  CThostFtdcTraderApi* follower_api_;
  // OrderNo -> ASDF
  std::map<std::string, > map_;
  strong_actor_ptr broker_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
};

#endif /* CTP_TRADER_H */
