#include "ctp_util.h"

CThostFtdcInputOrderField MakeCtpOpenOrder(const std::string& instrument,
                                           const std::string& order_no,
                                           OrderDirection direction,
                                           OrderPriceType price_type,
                                           double price,
                                           int quantity) {
  CThostFtdcInputOrderField field = {0};
  // strcpy(filed.BrokerID, "");
  // strcpy(filed.InvestorID, "");
  strcpy(field.InstrumentID, instrument.c_str());
  strcpy(field.OrderRef, order_no.c_str());
  // strcpy(filed.UserID, );
  field.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
  field.Direction =
      direction == OrderDirection::kBuy ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;
  field.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
  strcpy(field.CombHedgeFlag, "1");
  field.LimitPrice = price;
  field.VolumeTotalOriginal = quantity;
  field.TimeCondition = THOST_FTDC_TC_GFD;
  strcpy(field.GTDDate, "");
  field.VolumeCondition = THOST_FTDC_VC_AV;
  field.MinVolume = 1;
  field.ContingentCondition = THOST_FTDC_CC_Immediately;
  field.StopPrice = 0;
  field.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
  field.IsAutoSuspend = 0;
  field.UserForceClose = 0;
  return field;
}

CThostFtdcInputOrderField MakeCtpCloseOrder(const std::string& instrument,
                                            const std::string& order_no,
                                            OrderDirection direction,
                                            PositionEffect position_effect,
                                            OrderPriceType price_type,
                                            double price,
                                            int quantity) {
  CThostFtdcInputOrderField field = {0};
  // strcpy(filed.BrokerID, "");
  // strcpy(filed.InvestorID, "");
  strcpy(field.InstrumentID, instrument.c_str());
  strcpy(field.OrderRef, order_no.c_str());
  // strcpy(filed.UserID, );
  field.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
  field.Direction =
      direction == OrderDirection::kBuy ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;
  field.CombOffsetFlag[0] = position_effect == PositionEffect::kCloseToday
                                ? THOST_FTDC_OF_CloseToday
                                : THOST_FTDC_OF_Close;
  strcpy(field.CombHedgeFlag, "1");
  field.LimitPrice = price;
  field.VolumeTotalOriginal = quantity;
  field.TimeCondition = THOST_FTDC_TC_GFD;
  strcpy(field.GTDDate, "");
  field.VolumeCondition = THOST_FTDC_VC_AV;
  field.MinVolume = 1;
  field.ContingentCondition = THOST_FTDC_CC_Immediately;
  field.StopPrice = 0;
  field.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
  field.IsAutoSuspend = 0;
  field.UserForceClose = 0;
  return field;
}

CThostFtdcInputOrderActionField MakeCtpCancelOrderAction(
    int front_id,
    int session_id,
    const std::string& order_id,
    const std::string& exchange_id,
    const std::string& order_sys_id){
  CThostFtdcInputOrderActionField order = {0};
  order.ActionFlag = THOST_FTDC_AF_Delete;
  order.FrontID = front_id;
  order.SessionID = session_id;
  strcpy(order.OrderRef, order_id.c_str());
  strcpy(order.ExchangeID, exchange_id.c_str());
  strcpy(order.OrderSysID, order_sys_id.c_str());
  return order;
}

OrderDirection ParseOrderDirection(TThostFtdcDirectionType direction) {
  return direction == THOST_FTDC_D_Buy ? OrderDirection::kBuy
                                       : OrderDirection::kSell;
}

OrderPriceType ParseOrderPriceType(
    TThostFtdcOrderPriceTypeType order_price_type) {
  return order_price_type == THOST_FTDC_OPT_AnyPrice ? OrderPriceType::kMarket
                                                     : OrderPriceType::kLimit;
}

OrderStatus ParseOrderStatus(TThostFtdcOffsetFlagType flag) {
  OrderStatus status = OrderStatus::kActive;
  switch (flag) {
    case THOST_FTDC_OST_AllTraded:
      status = OrderStatus::kAllFilled;
      break;
    case THOST_FTDC_OST_Canceled:
      status = OrderStatus::kCancel;
    default:
      break;
  }
  return status;
}

PositionEffect ParsePositionEffect(TThostFtdcOffsetFlagType flag) {
  PositionEffect position_effect = PositionEffect::kClose;
  switch (flag) {
    case THOST_FTDC_OF_Open:
      position_effect = PositionEffect::kOpen;
      break;
    case THOST_FTDC_OF_Close:
      position_effect = PositionEffect::kClose;
      break;
    case THOST_FTDC_OF_CloseToday:
      position_effect = PositionEffect::kCloseToday;
      break;
    default:
      break;
  }
  return position_effect;
}

OrderData MakeOrderData(CThostFtdcOrderField* order) {
  // std::string account_id_;
  // std::string order_id_;
  // std::string instrument_;
  // std::string datetime_;
  // std::string user_product_info_;
  // std::string order_sys_id_;
  // std::string exchange_id_;
  // int quanitty_;
  // int filled_quantity_;
  // int session_id_;
  // double price_;
  // OrderDirection direction_;
  // OrderPriceType type_;
  // OrderStatus status_;
  // PositionEffect position_effect_;
  return OrderData{
      order->InvestorID,
      order->OrderRef,
      order->InstrumentID,
      order->InsertTime,
      order->UserProductInfo,
      order->OrderSysID,
      order->ExchangeID,
      order->VolumeTotalOriginal,
      order->VolumeTraded,
      order->SessionID,
      order->LimitPrice,
      ParseOrderDirection(order->Direction),
      ParseOrderPriceType(order->OrderPriceType),
      ParseOrderStatus(order->OrderStatus),
      ParsePositionEffect(order->CombOffsetFlag[0]),
  };
}
