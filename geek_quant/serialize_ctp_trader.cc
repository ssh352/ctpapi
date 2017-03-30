#include "serialize_ctp_trader.h"
#include <iostream>

SerializeCtpTrader::SerializeCtpTrader(const std::string& file_prefix) {
  rtn_order_file_.open(file_prefix + "rtn_order.txt");
  err_rtn_order_insert_file_.open(file_prefix + "err_rtn_order_insert.txt");
  err_rtn_order_action_file_.open(file_prefix + "err_rtn_order_action.txt");
  inverstor_position_file_.open(file_prefix + "inverstor_position_file.txt");
  qry_order_file_.open(file_prefix + "qry_order_file.txt");
  rtn_order_oa_ =
      boost::make_shared<boost::archive::text_oarchive>(rtn_order_file_);
  err_rtn_order_insert_oa_ = boost::make_shared<boost::archive::text_oarchive>(
      err_rtn_order_insert_file_);
  err_rtn_order_action_oa_ = boost::make_shared<boost::archive::text_oarchive>(
      err_rtn_order_action_file_);
  inverstor_position_oa_ = boost::make_shared<boost::archive::text_oarchive>(
      inverstor_position_file_);
  qry_order_oa_ =
      boost::make_shared<boost::archive::text_oarchive>(qry_order_file_);
  cta_api_ = CThostFtdcTraderApi::CreateFtdcTraderApi();
}

void SerializeCtpTrader::LoginServer(const std::string& front_server,
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

void SerializeCtpTrader::OnFrontConnected() {
  CThostFtdcReqUserLoginField req;
  memset(&req, 0, sizeof(req));
  strcpy(req.BrokerID, broker_id_.c_str());
  strcpy(req.UserID, user_id_.c_str());
  strcpy(req.Password, password_.c_str());
  // strcpy(req.UserProductInfo, "Q7");
  int iResult = cta_api_->ReqUserLogin(&req, 0);
}

void SerializeCtpTrader::OnFrontDisconnected(int nReason) {}

void SerializeCtpTrader::OnRspUserLogin(
    CThostFtdcRspUserLoginField* pRspUserLogin,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  std::cout << "Logon\n";
  /*
  
  {
  CThostFtdcQryInvestorPositionField filed{0};
  strcpy(filed.BrokerID, broker_id_.c_str());
  strcpy(filed.InvestorID, user_id_.c_str());
  cta_api_->ReqQryInvestorPosition(&filed, 0);
  }
  */

  {
    CThostFtdcQryOrderField field{0};
    strcpy(field.BrokerID, broker_id_.c_str());
    strcpy(field.InvestorID, user_id_.c_str());
    cta_api_->ReqQryOrder(&field, 0);
  }
}

void SerializeCtpTrader::OnRtnOrder(CThostFtdcOrderField* pOrder) {
  *rtn_order_oa_&* pOrder;
  std::cout << "RtnOrder:" << pOrder->OrderRef << "," << pOrder->InstrumentID
            << "\n";
}

void SerializeCtpTrader::OnRtnTrade(CThostFtdcTradeField* pTrade) {}

void SerializeCtpTrader::OnErrRtnOrderInsert(
    CThostFtdcInputOrderField* pInputOrder,
    CThostFtdcRspInfoField* pRspInfo) {
  *err_rtn_order_insert_oa_&* pInputOrder;
  *err_rtn_order_insert_oa_&* pRspInfo;
  std::cout << "OnErrRtnOrderInsert:" << pInputOrder->OrderRef << "\n";
}

void SerializeCtpTrader::OnErrRtnOrderAction(
    CThostFtdcOrderActionField* pOrderAction,
    CThostFtdcRspInfoField* pRspInfo) {
  static int i = 0;
  // *err_rtn_order_action_oa_&* pOrderAction;
  *err_rtn_order_action_oa_&* pRspInfo;
  std::cout << "OnErrRtnOrderInsert:" << ++i << "\n";
}

void SerializeCtpTrader::OnRspQryInvestorPosition(
    CThostFtdcInvestorPositionField* pInvestorPosition,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  *inverstor_position_oa_&* pInvestorPosition;
}

void SerializeCtpTrader::OnRspQryOrder(CThostFtdcOrderField* pOrder,
                                       CThostFtdcRspInfoField* pRspInfo,
                                       int nRequestID,
                                       bool bIsLast) {
  *qry_order_oa_&* pOrder;
}
