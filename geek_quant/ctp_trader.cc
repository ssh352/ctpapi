#include "geek_quant/ctp_trader.h"
#include <boost/optional.hpp>
#include <boost/log/trivial.hpp>

CtpTrader::CtpTrader(Delegate* delegate, const char* folw_path)
    : delegate_(delegate) {
  cta_api_ = CThostFtdcTraderApi::CreateFtdcTraderApi(folw_path);
}

void CtpTrader::LoginServer(const std::string& front_server,
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

void CtpTrader::OrderInsert(CThostFtdcInputOrderField order) {
  BOOST_LOG_TRIVIAL(info) << user_id_
                          << "]OrderInsert Instrument:" << order.InstrumentID
                          << ",OrderRef:" << order.OrderRef
                          << ",OC:" << order.CombOffsetFlag[0]
                          << ",Direction:" << order.Direction;
  strcpy(order.BrokerID, broker_id_.c_str());
  strcpy(order.UserID, user_id_.c_str());
  strcpy(order.InvestorID, user_id_.c_str());
  cta_api_->ReqOrderInsert(&order, 1);
}

void CtpTrader::OrderAction(CThostFtdcInputOrderActionField order) {
  BOOST_LOG_TRIVIAL(info) << user_id_
                          << "]OrderAction OrderRef:" << order.OrderRef;
  strcpy(order.BrokerID, broker_id_.c_str());
  strcpy(order.UserID, user_id_.c_str());
  strcpy(order.InvestorID, user_id_.c_str());
  cta_api_->ReqOrderAction(&order, 1);
}

void CtpTrader::QryInvestorPosition() {
  CThostFtdcQryInvestorPositionField field{0};
  strcpy(field.BrokerID, broker_id_.c_str());
  strcpy(field.InvestorID, user_id_.c_str());
  std::cout << "ReqQryInvestorPosition:"
            << cta_api_->ReqQryInvestorPosition(&field, 0) << "\n";
}

void CtpTrader::SettlementInfoConfirm() {
  CThostFtdcQrySettlementInfoConfirmField field{0};
  strcpy(field.BrokerID, broker_id_.c_str());
  strcpy(field.InvestorID, user_id_.c_str());
  cta_api_->ReqQrySettlementInfoConfirm(&field, 0);
}

void CtpTrader::OnFrontConnected() {
  std::cout << "OnFrontConnected\n";
  CThostFtdcReqUserLoginField req;
  memset(&req, 0, sizeof(req));
  strcpy(req.BrokerID, broker_id_.c_str());
  strcpy(req.UserID, user_id_.c_str());
  strcpy(req.Password, password_.c_str());
  strcpy(req.UserProductInfo, kStrategyUserProductInfo);
  int iResult = cta_api_->ReqUserLogin(&req, 0);
}

void CtpTrader::OnFrontDisconnected(int nReason) {}

void CtpTrader::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                               CThostFtdcRspInfoField* pRspInfo,
                               int nRequestID,
                               bool bIsLast) {
  if (pRspInfo->ErrorID == 0) {
    delegate_->OnLogon();
    std::cout << "OnRspUserLogin:"
              << "\n";
  } else {
    std::cout << "User Login Error:" << pRspInfo->ErrorMsg << "\n";
  }
}

void CtpTrader::OnRtnOrder(CThostFtdcOrderField* pOrder) {
  BOOST_LOG_TRIVIAL(info) << user_id_ << "]"
                          << "OrderRef:" << pOrder->OrderRef << ","
                          << "Instrument:" << pOrder->InstrumentID << ","
                          << "OC:" << pOrder->CombOffsetFlag[0] << ","
                          << "Direction:" << pOrder->Direction;
  delegate_->OnRtnOrderData(pOrder);
}

void CtpTrader::OnRtnTrade(CThostFtdcTradeField* pTrade) {
  std::cout << __FUNCTION__ << "\n";
}

void CtpTrader::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                    CThostFtdcRspInfoField* pRspInfo) {
  BOOST_LOG_TRIVIAL(warning)
      << user_id_ << "]"
      << "ErrRtnOrderInsert:" << pInputOrder->InstrumentID << ","
      << "ErrorId:" << pRspInfo->ErrorID << ","
      << "Err Message:" << pRspInfo->ErrorMsg;
}

void CtpTrader::OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction,
                                    CThostFtdcRspInfoField* pRspInfo) {
  BOOST_LOG_TRIVIAL(warning)
      << user_id_ << "]"
      << "ErrRtnOrderAction:" << pOrderAction->InstrumentID << ","
      << "ErrorId:" << pRspInfo->ErrorID << ","
      << "Err Message:" << pRspInfo->ErrorMsg;
}

void CtpTrader::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                 CThostFtdcRspInfoField* pRspInfo,
                                 int nRequestID,
                                 bool bIsLast) {}

void CtpTrader::OnRspQryInvestorPosition(
    CThostFtdcInvestorPositionField* pInvestorPosition,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  if (pRspInfo != NULL) {
    std::cout << "OnRspQryInvestorPosition:" << pRspInfo->ErrorMsg << "\n";
  }
  if (pInvestorPosition != NULL) {
    if (pInvestorPosition->YdPosition != 0) {
      std::cout << "Oops!\n";
      positions_.push_back(
          {pInvestorPosition->InstrumentID,
           pInvestorPosition->PosiDirection == THOST_FTDC_PD_Long ? OrderDirection::kBuy
                                                                  : OrderDirection::kSell,
           pInvestorPosition->YdPosition});
    }
  }

  std::cout << "Last:" << bIsLast << "\n";
  if (bIsLast) {
    std::cout << "OnPositions\n";
    delegate_->OnPositions(positions_);
    positions_.clear();
  }
}

void CtpTrader::OnRspError(CThostFtdcRspInfoField* pRspInfo,
                           int nRequestID,
                           bool bIsLast) {
  BOOST_LOG_TRIVIAL(warning) << user_id_ << "]"
                             << "RspError ErrorId:" << pRspInfo->ErrorID
                             << "Error Message:" << pRspInfo->ErrorMsg;
}

void CtpTrader::OnRspQrySettlementInfoConfirm(
    CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  if (pSettlementInfoConfirm == NULL) {
    CThostFtdcSettlementInfoConfirmField field{0};
    strcpy(field.BrokerID, broker_id_.c_str());
    strcpy(field.InvestorID, user_id_.c_str());
    cta_api_->ReqSettlementInfoConfirm(&field, 0);
  } else {
    delegate_->OnSettlementInfoConfirm();
  }
}

void CtpTrader::OnRspSettlementInfoConfirm(
    CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  if (pSettlementInfoConfirm != NULL) {
    delegate_->OnSettlementInfoConfirm();
  } else {
    // Except
  }
}

void CtpTrader::Delegate::OnSettlementInfoConfirm() {}
