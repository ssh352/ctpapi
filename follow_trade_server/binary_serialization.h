#ifndef FOLLOW_TRADE_SERVER_BINARY_SERIALIZATION_H
#define FOLLOW_TRADE_SERVER_BINARY_SERIALIZATION_H
#include "common/api_struct.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"

namespace boost {
namespace serialization {

template <class Archive>
void serialize(Archive& ar, OrderPosition& order, const unsigned int version) {
  ar& order.instrument;
  ar& order.order_direction;
  ar& order.quantity;
}

template <class Archive>
void serialize(Archive& ar, OrderField& order, const unsigned int version) {

}

template <class Archive>
void serialize(Archive& ar,
               CThostFtdcOrderField& order,
               const unsigned int version) {
  ar& order.BrokerID;
  ar& order.InvestorID;
  ar& order.InstrumentID;
  ar& order.OrderRef;
  ar& order.UserID;
  ar& order.OrderPriceType;
  ar& order.Direction;
  ar& order.CombOffsetFlag;
  ar& order.CombHedgeFlag;
  ar& order.LimitPrice;
  ar& order.VolumeTotalOriginal;
  ar& order.TimeCondition;
  ar& order.GTDDate;
  ar& order.VolumeCondition;
  ar& order.MinVolume;
  ar& order.ContingentCondition;
  ar& order.StopPrice;
  ar& order.ForceCloseReason;
  ar& order.IsAutoSuspend;
  ar& order.BusinessUnit;
  ar& order.RequestID;
  ar& order.OrderLocalID;
  ar& order.ExchangeID;
  ar& order.ParticipantID;
  ar& order.ClientID;
  ar& order.ExchangeInstID;
  ar& order.TraderID;
  ar& order.InstallID;
  ar& order.OrderSubmitStatus;
  ar& order.NotifySequence;
  ar& order.TradingDay;
  ar& order.SettlementID;
  ar& order.OrderSysID;
  ar& order.OrderSource;
  ar& order.OrderStatus;
  ar& order.OrderType;
  ar& order.VolumeTraded;
  ar& order.VolumeTotal;
  ar& order.InsertDate;
  ar& order.InsertTime;
  ar& order.ActiveTime;
  ar& order.SuspendTime;
  ar& order.UpdateTime;
  ar& order.CancelTime;
  ar& order.ActiveTraderID;
  ar& order.ClearingPartID;
  ar& order.SequenceNo;
  ar& order.FrontID;
  ar& order.SessionID;
  ar& order.UserProductInfo;
  ar& order.StatusMsg;
  ar& order.UserForceClose;
  ar& order.ActiveUserID;
  ar& order.BrokerOrderSeq;
  ar& order.RelativeOrderSysID;
  ar& order.ZCETotalTradedVolume;
  ar& order.IsSwapOrder;
  ar& order.BranchID;
  ar& order.InvestUnitID;
  ar& order.AccountID;
  ar& order.CurrencyID;
  ar& order.IPAddress;
  ar& order.MacAddress;
}

}  // namespace serialization
}  // namespace boost

#endif  // FOLLOW_TRADE_SERVER_BINARY_SERIALIZATION_H
