#ifndef COMMON_SERIALIZATION_UTIL_H
#define COMMON_SERIALIZATION_UTIL_H
#include <boost/serialization/serialization.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include "ctpapi/ThostFtdcUserApiStruct.h"
#include "common/api_struct.h"

//BOOST_FUSION_ADAPT_STRUCT(CThostFtdcOrderField,
//                          BrokerID,
//                          InvestorID,
//                          InstrumentID,
//                          OrderRef,
//                          UserID,
//                          OrderPriceType,
//                          Direction,
//                          CombOffsetFlag,
//                          CombHedgeFlag,
//                          LimitPrice,
//                          VolumeTotalOriginal,
//                          TimeCondition,
//                          GTDDate,
//                          VolumeCondition,
//                          MinVolume,
//                          ContingentCondition,
//                          StopPrice,
//                          ForceCloseReason,
//                          IsAutoSuspend,
//                          BusinessUnit,
//                          RequestID,
//                          OrderLocalID,
//                          ExchangeID,
//                          ParticipantID,
//                          ClientID,
//                          ExchangeInstID,
//                          TraderID,
//                          InstallID,
//                          OrderSubmitStatus,
//                          NotifySequence,
//                          TradingDay,
//                          SettlementID,
//                          OrderSysID,
//                          OrderSource,
//                          OrderStatus,
//                          OrderType,
//                          VolumeTraded,
//                          VolumeTotal,
//                          InsertDate,
//                          InsertTime,
//                          ActiveTime,
//                          SuspendTime,
//                          UpdateTime,
//                          CancelTime,
//                          ActiveTraderID,
//                          ClearingPartID,
//                          SequenceNo,
//                          FrontID,
//                          SessionID,
//                          UserProductInfo,
//                          StatusMsg,
//                          UserForceClose,
//                          ActiveUserID,
//                          BrokerOrderSeq,
//                          RelativeOrderSysID,
//                          ZCETotalTradedVolume,
//                          IsSwapOrder,
//                          BranchID,
//                          InvestUnitID,
//                          AccountID,
//                          CurrencyID,
//                          IPAddress,
//                          MacAddress)
//
//BOOST_FUSION_ADAPT_STRUCT(CThostFtdcTradeField,
//                          BrokerID,
//                          InvestorID,
//                          InstrumentID,
//                          OrderRef,
//                          UserID,
//                          ExchangeID,
//                          TradeID,
//                          Direction,
//                          OrderSysID,
//                          ParticipantID,
//                          ClientID,
//                          TradingRole,
//                          ExchangeInstID,
//                          OffsetFlag,
//                          HedgeFlag,
//                          Price,
//                          Volume,
//                          TradeDate,
//                          TradeTime,
//                          TradeType,
//                          PriceSource,
//                          TraderID,
//                          OrderLocalID,
//                          ClearingPartID,
//                          BusinessUnit,
//                          SequenceNo,
//                          TradingDay,
//                          SettlementID,
//                          BrokerOrderSeq,
//                          TradeSource)

BOOST_FUSION_ADAPT_STRUCT(OrderField,
(OrderDirection, direction)
(OrderDirection, position_effect_direction)
(PositionEffect, position_effect)
(OrderStatus, status)
(int, qty)
(int, leaves_qty)
(int, trading_qty)
(int, error_id)
(int, raw_error_id)
(double, input_price)
(double, trading_price)
(double, avg_price)
(TimeStamp, input_timestamp)
(TimeStamp, update_timestamp)
(std::string, instrument_id)
(std::string, exchange_id)
(std::string, date)
(std::string, order_id)
(std::string, raw_error_message))
                          //direction,
                          //position_effect_direction,
                          //position_effect,
                          //status,
                          //qty,
                          //leaves_qty,
                          //trading_qty,
                          //error_id,
                          //raw_error_id,
                          //input_price,
                          //trading_price,
                          //avg_price,
                          //input_timestamp,
                          //update_timestamp,
                          //instrument_id,
                          //exchange_id,
                          //date,
                          //order_id,
                          //raw_error_message)


BOOST_FUSION_ADAPT_STRUCT(CThostFtdcOrderField,
(TThostFtdcBrokerIDType, BrokerID)
(TThostFtdcInvestorIDType, InvestorID)
(TThostFtdcInstrumentIDType, InstrumentID)
(TThostFtdcOrderRefType, OrderRef)
(TThostFtdcUserIDType, UserID)
(TThostFtdcOrderPriceTypeType, OrderPriceType)
(TThostFtdcDirectionType, Direction)
(TThostFtdcCombOffsetFlagType, CombOffsetFlag)
(TThostFtdcCombHedgeFlagType, CombHedgeFlag)
(TThostFtdcPriceType, LimitPrice)
(TThostFtdcVolumeType, VolumeTotalOriginal)
(TThostFtdcTimeConditionType, TimeCondition)
(TThostFtdcDateType, GTDDate)
(TThostFtdcVolumeConditionType, VolumeCondition)
(TThostFtdcVolumeType, MinVolume)
(TThostFtdcContingentConditionType, ContingentCondition)
(TThostFtdcPriceType, StopPrice)
(TThostFtdcForceCloseReasonType, ForceCloseReason)
(TThostFtdcBoolType, IsAutoSuspend)
(TThostFtdcBusinessUnitType, BusinessUnit)
(TThostFtdcRequestIDType, RequestID)
(TThostFtdcOrderLocalIDType, OrderLocalID)
(TThostFtdcExchangeIDType, ExchangeID)
(TThostFtdcParticipantIDType, ParticipantID)
(TThostFtdcClientIDType, ClientID)
(TThostFtdcExchangeInstIDType, ExchangeInstID)
(TThostFtdcTraderIDType, TraderID)
(TThostFtdcInstallIDType, InstallID)
(TThostFtdcOrderSubmitStatusType, OrderSubmitStatus)
(TThostFtdcSequenceNoType, NotifySequence)
(TThostFtdcDateType, TradingDay)
(TThostFtdcSettlementIDType, SettlementID)
(TThostFtdcOrderSysIDType, OrderSysID)
(TThostFtdcOrderSourceType, OrderSource)
(TThostFtdcOrderStatusType, OrderStatus)
(TThostFtdcOrderTypeType, OrderType)
(TThostFtdcVolumeType, VolumeTraded)
(TThostFtdcVolumeType, VolumeTotal)
(TThostFtdcDateType, InsertDate)
(TThostFtdcTimeType, InsertTime)
(TThostFtdcTimeType, ActiveTime)
(TThostFtdcTimeType, SuspendTime)
(TThostFtdcTimeType, UpdateTime)
(TThostFtdcTimeType, CancelTime)
(TThostFtdcTraderIDType, ActiveTraderID)
(TThostFtdcParticipantIDType, ClearingPartID)
(TThostFtdcSequenceNoType, SequenceNo)
(TThostFtdcFrontIDType, FrontID)
(TThostFtdcSessionIDType, SessionID)
(TThostFtdcProductInfoType, UserProductInfo)
(TThostFtdcErrorMsgType, StatusMsg)
(TThostFtdcBoolType, UserForceClose)
(TThostFtdcUserIDType, ActiveUserID)
(TThostFtdcSequenceNoType, BrokerOrderSeq)
(TThostFtdcOrderSysIDType, RelativeOrderSysID)
(TThostFtdcVolumeType, ZCETotalTradedVolume)
(TThostFtdcBoolType, IsSwapOrder)
(TThostFtdcBranchIDType, BranchID)
(TThostFtdcInvestUnitIDType, InvestUnitID)
(TThostFtdcAccountIDType, AccountID)
(TThostFtdcCurrencyIDType, CurrencyID)
(TThostFtdcIPAddressType, IPAddress)
(TThostFtdcMacAddressType, MacAddress))


BOOST_FUSION_ADAPT_STRUCT(CThostFtdcTradeField,
(TThostFtdcBrokerIDType, BrokerID)
(TThostFtdcInvestorIDType, InvestorID)
(TThostFtdcInstrumentIDType, InstrumentID)
(TThostFtdcOrderRefType, OrderRef)
(TThostFtdcUserIDType, UserID)
(TThostFtdcExchangeIDType, ExchangeID)
(TThostFtdcTradeIDType, TradeID)
(TThostFtdcDirectionType, Direction)
(TThostFtdcOrderSysIDType, OrderSysID)
(TThostFtdcParticipantIDType, ParticipantID)
(TThostFtdcClientIDType, ClientID)
(TThostFtdcTradingRoleType, TradingRole)
(TThostFtdcExchangeInstIDType, ExchangeInstID)
(TThostFtdcOffsetFlagType, OffsetFlag)
(TThostFtdcHedgeFlagType, HedgeFlag)
(TThostFtdcPriceType, Price)
(TThostFtdcVolumeType, Volume)
(TThostFtdcDateType, TradeDate)
(TThostFtdcTimeType, TradeTime)
(TThostFtdcTradeTypeType, TradeType)
(TThostFtdcPriceSourceType, PriceSource)
(TThostFtdcTraderIDType, TraderID)
(TThostFtdcOrderLocalIDType, OrderLocalID)
(TThostFtdcParticipantIDType, ClearingPartID)
(TThostFtdcBusinessUnitType, BusinessUnit)
(TThostFtdcSequenceNoType, SequenceNo)
(TThostFtdcDateType, TradingDay)
(TThostFtdcSettlementIDType, SettlementID)
(TThostFtdcSequenceNoType, BrokerOrderSeq)
(TThostFtdcTradeSourceType, TradeSource)
  )


//template <class Archive>
//void serialize(Archive& ar, OrderField& order, const unsigned int version) {
//  ar& order.direction;
//  ar& order.position_effect_direction;
//  ar& order.position_effect;
//  ar& order.status;
//  ar& order.qty;
//  ar& order.leaves_qty;
//  ar& order.trading_qty;
//  ar& order.error_id;
//  ar& order.raw_error_id;
//  ar& order.input_price;
//  ar& order.trading_price;
//  ar& order.avg_price;
//  ar& order.input_timestamp;
//  ar& order.update_timestamp;
//  ar& order.instrument_id;
//  ar& order.exchange_id;
//  ar& order.date;
//  ar& order.order_id;
//  ar& order.raw_error_message;
//}

namespace boost {
namespace serialization {

template <typename Archive>
struct HelpSerializeCtpField {
  HelpSerializeCtpField(Archive& ar) : ar_(ar) {}
  template <typename T>
  void operator()(T& m) const {
    ar_& m;
  }

 private:
  Archive& ar_;
};

template <class Archive>
void serialize(Archive& ar,
               CThostFtdcOrderField& order,
               const unsigned int version) {
  fusion::for_each(order, HelpSerializeCtpField<Archive>(ar));
}

template <class Archive>
void serialize(Archive& ar,
               CThostFtdcTradeField& order,
               const unsigned int version) {
  fusion::for_each(order, HelpSerializeCtpField<Archive>(ar));
}

template <class Archive>
void serialize(Archive& ar, OrderField& order, const unsigned int version) {
  fusion::for_each(order, HelpSerializeCtpField<Archive>(ar));
}

template <class Archive>
void serialize(Archive& ar, CTPOrderField& order, const unsigned int version) {
  ar& order.direction;
  ar& order.position_effect_direction;
  ar& order.position_effect;
  ar& order.status;
  ar& order.qty;
  ar& order.leaves_qty;
  ar& order.trading_qty;
  ar& order.error_id;
  ar& order.raw_error_id;
  ar& order.front_id;
  ar& order.session_id;
  ar& order.input_price;
  ar& order.trading_price;
  ar& order.avg_price;
  ar& order.input_timestamp;
  ar& order.update_timestamp;
  ar& order.instrument;
  ar& order.exchange_id;
  ar& order.date;
  ar& order.order_id;
  ar& order.order_ref;
  ar& order.order_sys_id;
  ar& order.raw_error_message;
}

}  // namespace serialization
}  // namespace boost

inline std::ostream& operator<<(std::ostream& out,
                                const OrderDirection& direction) {
  out << static_cast<int>(direction);
  return out;
}

inline std::ostream& operator<<(std::ostream& out,
                                const PositionEffect& position_effect) {
  out << static_cast<int>(position_effect);
  return out;
}

inline std::ostream& operator<<(std::ostream& out, const OrderStatus& status) {
  out << static_cast<int>(status);
  return out;
}

#endif  // COMMON_SERIALIZATION_UTIL_H
