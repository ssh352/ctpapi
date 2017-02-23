#ifndef CAF_DEFINES_H
#define CAF_DEFINES_H

#include "caf/all.hpp"
#include "tradeapi/ThostFtdcTraderApi.h"

enum OrderDirection {
  kBuy,
  kSell,
};

enum OrderAction {
  kOpen,
  kClose,
  kCancel,
};

using CtpLoginAtom = caf::atom_constant<caf::atom("CtpLogin")>;
using CtpRtnOrderAtom = caf::atom_constant<caf::atom("CtpRO")>;

using AddListenerAtom = caf::atom_constant<caf::atom("AddListen")>;
using OpenOrderAtom = caf::atom_constant<caf::atom("OpenOrd")>;
using CloseOrderAtom = caf::atom_constant<caf::atom("CloseOrd")>;
using CancelOrderAtom = caf::atom_constant<caf::atom("CancelOrd")>;

using CtpObserver =
    caf::typed_actor<caf::reacts_to<CtpLoginAtom>,
                     caf::reacts_to<CtpRtnOrderAtom, CThostFtdcOrderField>,
                     caf::reacts_to<AddListenerAtom, caf::strong_actor_ptr> >;

using StrategyOrderAction =
    caf::typed_actor<caf::reacts_to<OpenOrderAtom,
                                    std::string,
                                    std::string,
                                    OrderDirection,
                                    double,
                                    int>,
                     caf::reacts_to<CloseOrderAtom,
                                    std::string,
                                    std::string,
                                    OrderDirection,
                                    double,
                                    int>,
                     caf::reacts_to<CancelOrderAtom, std::string> >;

// foo needs to be serializable
template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, CThostFtdcOrderField& x) {
  return f(
      meta::type_name("CThostFtdcOrderField"), x.BrokerID, x.InvestorID,
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
#endif /* CAF_DEFINES_H */
