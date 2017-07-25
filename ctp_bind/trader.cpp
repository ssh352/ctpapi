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
  orders_.insert_or_assign(
      MakeOrderId(order->FrontID, order->SessionID, order->OrderRef), order);

  auto order_field = boost::make_shared<OrderField>();
  // order_field->instrument_name = order->InstrumentName;
  order_field->instrument_id = order->InstrumentID;
  order_field->exchange_id = order->ExchangeID;
  order_field->direction = order->Direction == THOST_FTDC_D_Buy
                               ? OrderDirection::kBuy
                               : OrderDirection::kSell;
  order_field->qty = order->VolumeTotalOriginal;
  order_field->price = order->LimitPrice;
  order_field->position_effect =
      ParseTThostFtdcPositionEffect(order->CombOffsetFlag[0]);
  order_field->date = order->InsertDate;
  order_field->input_time = order->InsertTime;
  order_field->update_time = order->UpdateTime;
  order_field->order_id = str(boost::format("%d:%d:%s") % order->FrontID %
                              order->SessionID % order->OrderRef);

  order_field->status = ParseTThostFtdcOrderStatus(order);
  order_field->leaves_qty = order->VolumeTotal;
  order_field->traded_qty = order->VolumeTraded;
  order_field->error_id = 0;
  order_field->raw_error_id = 0;

  if (on_rtn_order_ != NULL) {
    on_rtn_order_(std::move(order_field));
  }
}

std::string ctp_bind::Trader::MakeOrderId(TThostFtdcFrontIDType front_id,
                                          TThostFtdcSessionIDType session_id,
                                          const std::string& order_ref) const {
  return str(boost::format("%d:%d:%s") % front_id % session_id % order_ref);
}

OrderStatus ctp_bind::Trader::ParseTThostFtdcOrderStatus(
    boost::shared_ptr<CThostFtdcOrderField> order) const {
  OrderStatus os = OrderStatus::kActive;
  switch (order->OrderStatus) {
    case THOST_FTDC_OST_AllTraded:
      os = OrderStatus::kAllFilled;
      break;
    case THOST_FTDC_OST_Canceled:
      os = OrderStatus::kCanceled;
      break;
    default:
      break;
  }
  return os;
}

PositionEffect ctp_bind::Trader::ParseTThostFtdcPositionEffect(
    TThostFtdcOffsetFlagType flag) {
  PositionEffect ps = PositionEffect::kUndefine;
  switch (flag) {
    case THOST_FTDC_OF_Open:
      ps = PositionEffect::kOpen;
      break;
    case THOST_FTDC_OF_Close:
    case THOST_FTDC_OF_ForceClose:
    case THOST_FTDC_OF_CloseYesterday:
    case THOST_FTDC_OF_ForceOff:
    case THOST_FTDC_OF_LocalForceClose:
      ps = PositionEffect::kClose;
      break;
    case THOST_FTDC_OF_CloseToday:
      ps = PositionEffect::kCloseToday;
      break;
  }
  return ps;
}

bool ctp_bind::Trader::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) const {
  return (pRspInfo != NULL) && (pRspInfo->ErrorID != 0);
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
  field.Direction = direction == direction;
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
    std::function<void(boost::shared_ptr<OrderField>)> callback) {
  io_service_->post([=](void) { on_rtn_order_ = callback; });
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
  if (IsErrorRspInfo(pRspInfo) && pInputOrder != NULL) {
    auto order_field = boost::make_shared<OrderField>();
    order_field->order_id =
        MakeOrderId(front_id_, session_id_, pInputOrder->OrderRef);
    order_field->status = OrderStatus::kRejected;
    order_field->raw_error_id = pRspInfo->ErrorID;
    order_field->raw_error_message = pRspInfo->ErrorMsg;
    io_service_->post([ =, order_field(std::move(order_field)) ]() {
      if (on_rtn_order_ != NULL) {
        on_rtn_order_(std::move(order_field));
      }
    });
  }
}

void ctp_bind::Trader::OnRspOrderAction(
    CThostFtdcInputOrderActionField* pInputOrderAction,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {

}

void ctp_bind::Trader::OnRspError(CThostFtdcRspInfoField* pRspInfo,
                                  int nRequestID,
                                  bool bIsLast) {}
