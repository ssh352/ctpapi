#include "trader.h"
#include <boost/format.hpp>
#include <boost/make_shared.hpp>

void ctp_bind::Trader::OnRspAuthenticate(
    CThostFtdcRspAuthenticateField* pRspAuthenticateField,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {}

void ctp_bind::Trader::OnFrontConnected() {
  CThostFtdcReqUserLoginField field{0};
  strcpy(field.UserID, user_id_.c_str());
  strcpy(field.Password, password_.c_str());
  strcpy(field.BrokerID, broker_id_.c_str());
  api_->ReqUserLogin(&field, 0);
}

void ctp_bind::Trader::OnFrontDisconnected(int nReason) {}

void ctp_bind::Trader::OnRspUserLogin(
    CThostFtdcRspUserLoginField* pRspUserLogin,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  front_id_ = pRspUserLogin->FrontID;
  session_id_ = pRspUserLogin->SessionID;
  if (on_connect_ != NULL) {
    on_connect_(pRspUserLogin, pRspInfo);
  }
}

void ctp_bind::Trader::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                                       CThostFtdcRspInfoField* pRspInfo,
                                       int nRequestID,
                                       bool bIsLast) {}

void ctp_bind::Trader::OnRtnOrder(CThostFtdcOrderField* pOrder) {
  io_service_->post(
      boost::bind(&Trader::OnRtnOrderOnIOThread, this,
                  boost::make_shared<CThostFtdcOrderField>(*pOrder)));
}

void ctp_bind::Trader::OnRtnOrderOnIOThread(
    boost::shared_ptr<CThostFtdcOrderField> order) {
  orders_.insert_or_assign(MakeOrderId(order), order);
}

std::string ctp_bind::Trader::MakeOrderId(
    boost::shared_ptr<CThostFtdcOrderField> order) {
  return str(boost::format("%d:%d:%s") % order->FrontID % order->SessionID %
             order->OrderRef);
}

ctp_bind::Trader::Trader(std::string server,
                         std::string broker_id,
                         std::string user_id,
                         std::string password)
    : server_(server),
      broker_id_(broker_id),
      user_id_(user_id),
      password_(password) {}

void ctp_bind::Trader::InitAsio(
    boost::asio::io_service* io_service /*= nullptr*/) {
  if (io_service == NULL) {
    io_service_ = new boost::asio::io_service();
  } else {
    io_service_ = io_service;
  }
  io_worker_ = std::make_shared<boost::asio::io_service::work>(*io_service_);
}

void ctp_bind::Trader::Connect(
    std::function<void(CThostFtdcRspUserLoginField*, CThostFtdcRspInfoField*)>
        callback) {
  on_connect_ = callback;

  api_ = CThostFtdcTraderApi::CreateFtdcTraderApi();
  api_->RegisterSpi(this);
  api_->RegisterFront(const_cast<char*>(server_.c_str()));
  api_->Init();
}

void ctp_bind::Trader::Run() {
  io_service_->run();
}

void ctp_bind::Trader::OpenOrder(std::string instrument,
                                 TThostFtdcDirectionType direction,
                                 double price,
                                 int volume,
                                 std::string order_ref /*= ""*/) {
  CThostFtdcInputOrderField field = {0};
  // strcpy(filed.BrokerID, "");
  // strcpy(filed.InvestorID, "");
  strcpy(field.InstrumentID, instrument.c_str());
  strcpy(field.OrderRef, order_ref.c_str());
  // strcpy(filed.UserID, );
  field.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
  field.Direction =
      direction == direction;
  field.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
  strcpy(field.CombHedgeFlag, "1");
  field.LimitPrice = price;
  field.VolumeTotalOriginal = volume;
  field.TimeCondition = THOST_FTDC_TC_GFD;
  strcpy(field.GTDDate, "");
  field.VolumeCondition = THOST_FTDC_VC_AV;
  field.MinVolume = 1;
  field.ContingentCondition = THOST_FTDC_CC_Immediately;
  field.StopPrice = 0;
  field.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
  field.IsAutoSuspend = 0;
  field.UserForceClose = 0;
  strcpy(field.BrokerID, broker_id_.c_str());
  strcpy(field.UserID, user_id_.c_str());
  strcpy(field.InvestorID, user_id_.c_str());
  api_->ReqOrderInsert(&field, 0);
  
}

void ctp_bind::Trader::CloseOrder(std::string instrument,
                                  TThostFtdcDirectionType direction,
                                  double price,
                                  int volume,
                                  std::string order_ref /*= ""*/) {}

void ctp_bind::Trader::CancelOrder(std::string order_id) {
  io_service_->post(
      boost::bind(&Trader::CancelOrderOnIOThread, this, std::move(order_id)));
}

void ctp_bind::Trader::SubscribeRtnOrder(
    std::function<void(boost::shared_ptr<CThostFtdcOrderField>)> callback) {
  io_service_->post([=](void){
    on_rtn_order_ = callback;
  });
}

void ctp_bind::Trader::CancelOrderOnIOThread(std::string order_id) {
  if (orders_.find(order_id) != orders_.end()) {
    ReqCancelOrder(orders_[order_id]);
  }
}

void ctp_bind::Trader::ReqCancelOrder(
    boost::shared_ptr<CThostFtdcOrderField> order) {
  CThostFtdcInputOrderActionField field = {0};
  field.ActionFlag = THOST_FTDC_AF_Delete;
  field.FrontID = front_id_;
  field.SessionID = session_id_;
  strcpy(field.OrderRef, order->OrderRef);
  strcpy(field.ExchangeID, order->ExchangeID);
  strcpy(field.OrderSysID, order->OrderSysID);
  strcpy(field.BrokerID, order->BrokerID);
  api_->ReqOrderAction(&field, request_id_.fetch_add(1));
}

void ctp_bind::Trader::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                        CThostFtdcRspInfoField* pRspInfo,
                                        int nRequestID,
                                        bool bIsLast) {
  io_service_->post(MakeHandleResponse(CThostFtdcInputOrderField(*pInputOrder),
                                       CThostFtdcRspInfoField(*pRspInfo),
                                       nRequestID, bIsLast));
}

void ctp_bind::Trader::OnRspOrderAction(
    CThostFtdcInputOrderActionField* pInputOrderAction,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  io_service_->post(MakeHandleResponse(
      CThostFtdcInputOrderActionField(*pInputOrderAction),
      CThostFtdcRspInfoField(*pRspInfo), nRequestID, bIsLast));
}

void ctp_bind::Trader::OnRspError(CThostFtdcRspInfoField* pRspInfo,
                                  int nRequestID,
                                  bool bIsLast) {
  CThostFtdcRspInfoField rsp_info(*pRspInfo);
  io_service_->post([ =, rsp_info{std::move(rsp_info)} ](void) mutable {
    if (ctp_callbacks_.find(nRequestID) != ctp_callbacks_.end()) {
      boost::apply_visitor(CTPRspErrorCallbackVisitor(&rsp_info, bIsLast),
                           ctp_callbacks_[nRequestID]);
    }
  });
}
