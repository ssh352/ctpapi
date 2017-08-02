#ifndef CAF_DEFINES_H
#define CAF_DEFINES_H

#include <tuple>
#include "caf/all.hpp"
#include "ctpapi/ThostFtdcUserApiStruct.h"

// CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::tuple<std::string, std::string, std::string>)

// using TALoginAtom = caf::atom_constant<caf::atom("login")>;

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, OrderPosition& x) {
  return f(caf::meta::type_name("OrderPosition"), x.instrument,
           x.order_direction, x.quantity);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, CThostFtdcOrderField& x) {
  return f(
      caf::meta::type_name("CThostFtdcOrderField"), x.BrokerID, x.InvestorID,
      x.InstrumentID, x.OrderRef, x.UserID, x.OrderPriceType, x.Direction,
      x.CombOffsetFlag, x.CombHedgeFlag, x.LimitPrice, x.VolumeTotalOriginal,
      x.TimeCondition, x.GTDDate, x.VolumeCondition, x.MinVolume,
      x.ContingentCondition, x.StopPrice, x.ForceCloseReason, x.IsAutoSuspend,
      x.BusinessUnit, x.RequestID, x.OrderLocalID, x.ExchangeID,
      x.ParticipantID, x.ClientID, x.ExchangeInstID, x.TraderID, x.InstallID,
      x.OrderSubmitStatus, x.NotifySequence, x.TradingDay, x.SettlementID,
      x.OrderSysID, x.OrderSource, x.OrderStatus, x.OrderType, x.VolumeTraded,
      x.VolumeTotal, x.InsertDate, x.InsertTime, x.ActiveTime, x.SuspendTime,
      x.UpdateTime, x.CancelTime, x.ActiveTraderID, x.ClearingPartID,
      x.SequenceNo, x.FrontID, x.SessionID, x.UserProductInfo, x.StatusMsg,
      x.UserForceClose, x.ActiveUserID, x.BrokerOrderSeq, x.RelativeOrderSysID,
      x.ZCETotalTradedVolume, x.IsSwapOrder, x.BranchID, x.InvestUnitID,
      x.AccountID, x.CurrencyID, x.IPAddress, x.MacAddress);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, AccountPortfolio& x) {
  return f(caf::meta::type_name("AccountPortfolio"), x.instrument, x.direction,
           x.closeable, x.open, x.close);
}

/*
template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, EnterOrderData& x) {
return f(caf::meta::type_name("EnterOrderData"), x.instrument, x.order_id,
       x.action, x.order_direction, x.order_price, x.volume);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, OrderIdent& x) {
return f(caf::meta::type_name("OrderIdent"), x.order_id, x.front_id,
       x.session_id, x.exchange_id, x.order_sys_id);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, OrderPosition& x) {
return f(caf::meta::type_name("OrderPosition"), x.instrument,
       x.order_direction, x.volume);
}
*/

#endif /* CAF_DEFINES_H */
