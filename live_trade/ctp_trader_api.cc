#include "ctp_trader_api.h"

CTPTraderApi::CTPTraderApi(Delegate* delegate) : delegate_(delegate) {
  api_ = CThostFtdcTraderApi::CreateFtdcTraderApi();
  api_->RegisterSpi(this);
}

TThostFtdcOffsetFlagType CTPTraderApi::PositionEffectToTThostOffsetFlag(
    CTPPositionEffect position_effect) {
  return position_effect == CTPPositionEffect::kOpen
             ? THOST_FTDC_OF_Open
             : (position_effect == CTPPositionEffect::kCloseToday
                    ? THOST_FTDC_OF_CloseToday
                    : THOST_FTDC_OF_Close);
}

TThostFtdcDirectionType CTPTraderApi::OrderDirectionToTThostOrderDireciton(
    OrderDirection direction) {
  return direction == OrderDirection::kBuy ? THOST_FTDC_D_Buy
                                           : THOST_FTDC_D_Sell;
}

OrderStatus CTPTraderApi::ParseTThostFtdcOrderStatus(
    CThostFtdcOrderField* order) const {
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

CTPPositionEffect CTPTraderApi::ParseTThostFtdcPositionEffect(
    TThostFtdcOffsetFlagType flag) {
  CTPPositionEffect ps = CTPPositionEffect::kUndefine;
  switch (flag) {
    case THOST_FTDC_OF_Open:
      ps = CTPPositionEffect::kOpen;
      break;
    case THOST_FTDC_OF_Close:
    case THOST_FTDC_OF_ForceClose:
    case THOST_FTDC_OF_CloseYesterday:
    case THOST_FTDC_OF_ForceOff:
    case THOST_FTDC_OF_LocalForceClose:
      ps = CTPPositionEffect::kClose;
      break;
    case THOST_FTDC_OF_CloseToday:
      ps = CTPPositionEffect::kCloseToday;
      break;
  }
  return ps;
}

std::string CTPTraderApi::MakeOrderId(TThostFtdcFrontIDType front_id,
                                      TThostFtdcSessionIDType session_id,
                                      const std::string& order_ref) const {
  return str(boost::format("%d:%d:%s") % front_id % session_id % order_ref);
}

std::string CTPTraderApi::MakeOrderId(const std::string& order_ref) const
{
  return MakeOrderId(front_id_, session_id_, order_ref);
}

void CTPTraderApi::OnRspOrderAction(
    CThostFtdcInputOrderActionField* pInputOrderAction,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
}

void CTPTraderApi::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                    CThostFtdcRspInfoField* pRspInfo,
                                    int nRequestID,
                                    bool bIsLast) {
}

void CTPTraderApi::OnRspError(CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) {}

void CTPTraderApi::OnRtnOrder(CThostFtdcOrderField* pOrder) {
  auto order_field = std::make_shared<CTPOrderField>();
  // order_field->instrument_name = order->InstrumentName;
  order_field->instrument = pOrder->InstrumentID;
  order_field->exchange_id = pOrder->ExchangeID;
  order_field->direction = pOrder->Direction == THOST_FTDC_D_Buy
                               ? OrderDirection::kBuy
                               : OrderDirection::kSell;
  order_field->qty = pOrder->VolumeTotalOriginal;
  order_field->input_price = pOrder->LimitPrice;
  order_field->position_effect =
      ParseTThostFtdcPositionEffect(pOrder->CombOffsetFlag[0]);
  order_field->date = pOrder->InsertDate;
  // order_field->input_timestamp = pOrder->InsertTime;
  // order_field->update_timestamp = pOrder->UpdateTime;

  order_field->status = ParseTThostFtdcOrderStatus(pOrder);
  order_field->leaves_qty = pOrder->VolumeTotal;
  order_field->order_id = pOrder->OrderRef;
  auto it = order_traded_qty_set_.find(order_field->order_id);
  if (it != order_traded_qty_set_.end()) {
    order_field->trading_qty = pOrder->VolumeTraded - it->second;
    it->second = pOrder->VolumeTraded;
  } else {
    order_field->trading_qty = 0;
  }
  order_field->error_id = 0;
  order_field->raw_error_id = 0;
  order_field->front_id = pOrder->FrontID;
  order_field->session_id = pOrder->SessionID;
  
  delegate_->HandleCTPRtnOrder(std::move(order_field));
}

void CTPTraderApi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                                   CThostFtdcRspInfoField* pRspInfo,
                                   int nRequestID,
                                   bool bIsLast) {}

void CTPTraderApi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                                  CThostFtdcRspInfoField* pRspInfo,
                                  int nRequestID,
                                  bool bIsLast) {
  if (pRspInfo == NULL || pRspInfo->ErrorID != 0) {
    // TODO: Logon Fail
  } else {
    session_id_ = pRspUserLogin->SessionID;
    front_id_ = pRspUserLogin->FrontID;
  }
}

void CTPTraderApi::OnFrontConnected() {
  CThostFtdcReqUserLoginField field{0};
  strcpy(field.UserID, user_id_.c_str());
  strcpy(field.Password, password_.c_str());
  strcpy(field.BrokerID, broker_id_.c_str());
  api_->ReqUserLogin(&field, 0);
}

void CTPTraderApi::HandleInputOrder(const CTPEnterOrder& input_order, const std::string& order_id) {
  CThostFtdcInputOrderField field = {0};
  strcpy(field.InstrumentID, input_order.instrument.c_str());
  strcpy(field.OrderRef, order_id.c_str());
  field.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
  field.Direction =
      OrderDirectionToTThostOrderDireciton(input_order.direction);
  field.CombOffsetFlag[0] =
      PositionEffectToTThostOffsetFlag(input_order.position_effect);
  strcpy(field.CombHedgeFlag, "1");
  field.LimitPrice = input_order.price;
  field.VolumeTotalOriginal = input_order.qty;
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

  if (api_->ReqOrderInsert(&field, 0) != 0) {
    auto order_field = std::make_shared<OrderField>();
    // order_field->strategy_id = sub_account_id;
    // order_field->order_id = sub_order_id;
    order_field->status = OrderStatus::kInputRejected;
    // mail_box_->Send(std::move(order_field));
  }
}

void CTPTraderApi::Connect(const std::string& server,
                           std::string broker_id,
                           std::string user_id,
                           std::string password) {
  broker_id_ = std::move(broker_id);
  user_id_ = std::move(user_id);
  password_ = std::move(password);
  char fron_server[255] = {0};
  strcpy(fron_server, server.c_str());
  api_->RegisterFront(fron_server);
  api_->SubscribePublicTopic(THOST_TERT_RESUME);
  api_->SubscribePrivateTopic(THOST_TERT_RESUME);
  api_->Init();
}
