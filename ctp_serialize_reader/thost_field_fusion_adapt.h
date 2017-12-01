#ifndef _THOST_FIELD_FUSION_ADAPT_H
#define _THOST_FIELD_FUSION_ADAPT_H
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/for_each.hpp>
#include "ctpapi/ThostFtdcUserApiStruct.h"

BOOST_FUSION_ADAPT_STRUCT(CThostFtdcOrderField,
                          BrokerID,
                          InvestorID,
                          InstrumentID,
                          OrderRef,
                          UserID,
                          OrderPriceType,
                          Direction,
                          CombOffsetFlag,
                          CombHedgeFlag,
                          LimitPrice,
                          VolumeTotalOriginal,
                          TimeCondition,
                          GTDDate,
                          VolumeCondition,
                          MinVolume,
                          ContingentCondition,
                          StopPrice,
                          ForceCloseReason,
                          IsAutoSuspend,
                          BusinessUnit,
                          RequestID,
                          OrderLocalID,
                          ExchangeID,
                          ParticipantID,
                          ClientID,
                          ExchangeInstID,
                          TraderID,
                          InstallID,
                          OrderSubmitStatus,
                          NotifySequence,
                          TradingDay,
                          SettlementID,
                          OrderSysID,
                          OrderSource,
                          OrderStatus,
                          OrderType,
                          VolumeTraded,
                          VolumeTotal,
                          InsertDate,
                          InsertTime,
                          ActiveTime,
                          SuspendTime,
                          UpdateTime,
                          CancelTime,
                          ActiveTraderID,
                          ClearingPartID,
                          SequenceNo,
                          FrontID,
                          SessionID,
                          UserProductInfo,
                          StatusMsg,
                          UserForceClose,
                          ActiveUserID,
                          BrokerOrderSeq,
                          RelativeOrderSysID,
                          ZCETotalTradedVolume,
                          IsSwapOrder,
                          BranchID,
                          InvestUnitID,
                          AccountID,
                          CurrencyID,
                          IPAddress,
                          MacAddress)



BOOST_FUSION_ADAPT_STRUCT(CThostFtdcTradeField,
                          BrokerID,
                          InvestorID,
                          InstrumentID,
                          OrderRef,
                          UserID,
                          ExchangeID,
                          TradeID,
                          Direction,
                          OrderSysID,
                          ParticipantID,
                          ClientID,
                          TradingRole,
                          ExchangeInstID,
                          OffsetFlag,
                          HedgeFlag,
                          Price,
                          Volume,
                          TradeDate,
                          TradeTime,
                          TradeType,
                          PriceSource,
                          TraderID,
                          OrderLocalID,
                          ClearingPartID,
                          BusinessUnit,
                          SequenceNo,
                          TradingDay,
                          SettlementID,
                          BrokerOrderSeq,
                          TradeSource)
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

}  // namespace serialization
}  // namespace boost

#endif // _THOST_FIELD_FUSION_ADAPT_H
