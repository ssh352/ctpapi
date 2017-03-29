#ifndef SERIALIZE_RTN_ORDER_SERIALIZE_CTP_TRADER_H
#define SERIALIZE_RTN_ORDER_SERIALIZE_CTP_TRADER_H
#include "ctpapi/ThostFtdcTraderApi.h"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace boost {
namespace serialization {

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

template <class Archive>
void serialize(Archive& ar,
               CThostFtdcInputOrderField& order,
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
  ar& order.UserForceClose;
  ar& order.IsSwapOrder;
  ar& order.ExchangeID;
  ar& order.InvestUnitID;
  ar& order.AccountID;
  ar& order.CurrencyID;
  ar& order.ClientID;
  ar& order.IPAddress;
  ar& order.MacAddress;
}

template <class Archive>
void serialize(Archive& ar,
               CThostFtdcRspInfoField& order,
               const unsigned int version) {
  ar& order.ErrorID;
  ar& order.ErrorMsg;
}

template <class Archive>
void serialize(Archive& ar,
               CThostFtdcOrderActionField& order,
               const unsigned int version) {
  ar& order.BrokerID;
  ar& order.InvestorID;
  ar& order.OrderActionRef;
  ar& order.OrderRef;
  ar& order.RequestID;
  ar& order.FrontID;
  ar& order.SessionID;
  ar& order.ExchangeID;
  ar& order.OrderSysID;
  ar& order.ActionFlag;
  ar& order.LimitPrice;
  ar& order.VolumeChange;
  ar& order.ActionDate;
  ar& order.ActionTime;
  ar& order.TraderID;
  ar& order.InstallID;
  ar& order.OrderLocalID;
  ar& order.ActionLocalID;
  ar& order.ParticipantID;
  ar& order.ClientID;
  ar& order.BusinessUnit;
  ar& order.OrderActionStatus;
  ar& order.UserID;
  ar& order.StatusMsg;
  ar& order.InstrumentID;
  ar& order.BranchID;
  ar& order.InvestUnitID;
  ar& order.IPAddress;
  ar& order.MacAddress;
}

template <class Archive>
void serialize(Archive& ar,
               CThostFtdcInvestorPositionField& position,
               const unsigned int version) {
  ar& position.InstrumentID;
  ar& position.BrokerID;
  ar& position.InvestorID;
  ar& position.PosiDirection;
  ar& position.HedgeFlag;
  ar& position.PositionDate;
  ar& position.YdPosition;
  ar& position.Position;
  ar& position.LongFrozen;
  ar& position.ShortFrozen;
  ar& position.LongFrozenAmount;
  ar& position.ShortFrozenAmount;
  ar& position.OpenVolume;
  ar& position.CloseVolume;
  ar& position.OpenAmount;
  ar& position.CloseAmount;
  ar& position.PositionCost;
  ar& position.PreMargin;
  ar& position.UseMargin;
  ar& position.FrozenMargin;
  ar& position.FrozenCash;
  ar& position.FrozenCommission;
  ar& position.CashIn;
  ar& position.Commission;
  ar& position.CloseProfit;
  ar& position.PositionProfit;
  ar& position.PreSettlementPrice;
  ar& position.SettlementPrice;
  ar& position.TradingDay;
  ar& position.SettlementID;
  ar& position.OpenCost;
  ar& position.ExchangeMargin;
  ar& position.CombPosition;
  ar& position.CombLongFrozen;
  ar& position.CombShortFrozen;
  ar& position.CloseProfitByDate;
  ar& position.CloseProfitByTrade;
  ar& position.TodayPosition;
  ar& position.MarginRateByMoney;
  ar& position.MarginRateByVolume;
  ar& position.StrikeFrozen;
  ar& position.StrikeFrozenAmount;
  ar& position.AbandonFrozen;
}

}  // namespace serialization
}  // namespace boost

class SerializeCtpTrader : public CThostFtdcTraderSpi {
 public:
  SerializeCtpTrader(const std::string& file_prefix);

  void LoginServer(const std::string& front_server,
                   const std::string& broker_id,
                   const std::string& user_id,
                   const std::string& password);

  virtual void OnFrontConnected() override;

  virtual void OnFrontDisconnected(int nReason) override;

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override;

  virtual void OnRtnOrder(CThostFtdcOrderField* pOrder) override;

  virtual void OnRtnTrade(CThostFtdcTradeField* pTrade) override;

  virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                   CThostFtdcRspInfoField* pRspInfo) override;

  virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction,
                                   CThostFtdcRspInfoField* pRspInfo) override;

  virtual void OnRspQryInvestorPosition(
      CThostFtdcInvestorPositionField* pInvestorPosition,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspQryOrder(CThostFtdcOrderField* pOrder,
                             CThostFtdcRspInfoField* pRspInfo,
                             int nRequestID,
                             bool bIsLast) override;

 private:
  CThostFtdcTraderApi* cta_api_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  std::ofstream rtn_order_file_;
  std::ofstream err_rtn_order_insert_file_;
  std::ofstream err_rtn_order_action_file_;
  std::ofstream inverstor_position_file_;
  std::ofstream qry_order_file_;
  // boost::archive::text_oarchive oa(file);
  boost::shared_ptr<boost::archive::text_oarchive> rtn_order_oa_;
  boost::shared_ptr<boost::archive::text_oarchive> err_rtn_order_insert_oa_;
  boost::shared_ptr<boost::archive::text_oarchive> err_rtn_order_action_oa_;
  boost::shared_ptr<boost::archive::text_oarchive> inverstor_position_oa_;
  boost::shared_ptr<boost::archive::text_oarchive> qry_order_oa_;
};

#endif  // SERIALIZE_RTN_ORDER_SERIALIZE_CTP_TRADER_H
