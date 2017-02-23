#ifndef CTP_TRADER_H
#define CTP_TRADER_H
#include "geek_quant/caf_defines.h"
#include <fstream>
#include <boost/lexical_cast.hpp>

class CtpTrader : public CThostFtdcTraderSpi {
 public:
  CtpTrader(caf::strong_actor_ptr observer)
      : observer_(observer),
        fstream_("rtn_order.csv"),
        position_order_fstream_("position_order.csv"),
        position_detail_fstream_("position_detail_orders.csv") {
    cta_api_ = CThostFtdcTraderApi::CreateFtdcTraderApi();
    fstream_ << "BrokerID,"
             << "InvestorID,"
             << "InstrumentID,"
             << "OrderRef,"
             << "UserID,"
             << "OrderPriceType,"
             << "Direction,"
             << "CombOffsetFlag,"
             << "CombHedgeFlag,"
             << "LimitPrice,"
             << "VolumeTotalOriginal,"
             << "TimeCondition,"
             << "GTDDate,"
             << "VolumeCondition,"
             << "MinVolume,"
             << "ContingentCondition,"
             << "StopPrice,"
             << "ForceCloseReason,"
             << "IsAutoSuspend,"
             << "BusinessUnit,"
             << "RequestID,"
             << "OrderLocalID,"
             << "ExchangeID,"
             << "ParticipantID,"
             << "ClientID,"
             << "ExchangeInstID,"
             << "TraderID,"
             << "InstallID,"
             << "OrderSubmitStatus,"
             << "NotifySequence,"
             << "TradingDay,"
             << "SettlementID,"
             << "OrderSysID,"
             << "OrderSource,"
             << "OrderStatus,"
             << "OrderType,"
             << "VolumeTraded,"
             << "VolumeTotal,"
             << "InsertDate,"
             << "InsertTime,"
             << "ActiveTime,"
             << "SuspendTime,"
             << "UpdateTime,"
             << "CancelTime,"
             << "ActiveTraderID,"
             << "ClearingPartID,"
             << "SequenceNo,"
             << "FrontID,"
             << "SessionID,"
             << "UserProductInfo,"
             << "StatusMsg,"
             << "UserForceClose,"
             << "ActiveUserID,"
             << "BrokerOrderSeq,"
             << "RelativeOrderSysID,"
             << "ZCETotalTradedVolume,"
             << "IsSwapOrder,"
             << "BranchID,"
             << "InvestUnitID,"
             << "AccountID,"
             << "CurrencyID,"
             << "IPAddress,"
             << "MacAddress,"
             << "\n";

    position_detail_fstream_ << "InstrumentID,"
                             << "BrokerID,"
                             << "InvestorID,"
                             << "HedgeFlag,"
                             << "Direction,"
                             << "OpenDate,"
                             << "TradeID,"
                             << "Volume,"
                             << "OpenPrice,"
                             << "TradingDay,"
                             << "SettlementID,"
                             << "TradeType,"
                             << "CombInstrumentID,"
                             << "ExchangeID,"
                             << "CloseProfitByDate,"
                             << "CloseProfitByTrade,"
                             << "PositionProfitByDate,"
                             << "PositionProfitByTrade,"
                             << "Margin,"
                             << "ExchMargin,"
                             << "MarginRateByMoney,"
                             << "MarginRateByVolume,"
                             << "LastSettlementPrice,"
                             << "SettlementPrice,"
                             << "CloseVolume,"
                             << "CloseAmount\n";

    position_order_fstream_ << "InstrumentID,"
                            << "BrokerID,"
                            << "InvestorID,"
                            << "PosiDirection,"
                            << "HedgeFlag,"
                            << "PositionDate,"
                            << "YdPosition,"
                            << "Position,"
                            << "LongFrozen,"
                            << "ShortFrozen,"
                            << "LongFrozenAmount,"
                            << "ShortFrozenAmount,"
                            << "OpenVolume,"
                            << "CloseVolume,"
                            << "OpenAmount,"
                            << "CloseAmount,"
                            << "PositionCost,"
                            << "PreMargin,"
                            << "UseMargin,"
                            << "FrozenMargin,"
                            << "FrozenCash,"
                            << "FrozenCommission,"
                            << "CashIn,"
                            << "Commission,"
                            << "CloseProfit,"
                            << "PositionProfit,"
                            << "PreSettlementPrice,"
                            << "SettlementPrice,"
                            << "TradingDay,"
                            << "SettlementID,"
                            << "OpenCost,"
                            << "ExchangeMargin,"
                            << "CombPosition,"
                            << "CombLongFrozen,"
                            << "CombShortFrozen,"
                            << "CloseProfitByDate,"
                            << "CloseProfitByTrade,"
                            << "TodayPosition,"
                            << "MarginRateByMoney,"
                            << "MarginRateByVolume,"
                            << "StrikeFrozen,"
                            << "StrikeFrozenAmount,"
                            << "AbandonFrozen\n";
  }

  void LoginServer(const std::string& front_server,
                   const std::string& broker_id,
                   const std::string& user_id,
                   const std::string& password) {
    broker_id_ = broker_id;
    user_id_ = user_id;
    password_ = password;

    cta_api_->RegisterSpi(this);
    char front_server_buffer[256] = {0};
    strcpy(front_server_buffer, front_server.c_str());

    cta_api_->RegisterFront(front_server_buffer);
    // api_->SubscribePublicTopic(THOST_TERT_RESTART);
    cta_api_->SubscribePublicTopic(THOST_TERT_RESUME);
    cta_api_->SubscribePrivateTopic(THOST_TERT_RESTART);
    cta_api_->Init();
  }

  void Join() { cta_api_->Join(); }

  virtual void OnFrontConnected() override {
    std::cout << "OnFrontConnected\n";
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, broker_id_.c_str());
    strcpy(req.UserID, user_id_.c_str());
    strcpy(req.Password, password_.c_str());
    int iResult = cta_api_->ReqUserLogin(&req, 0);
  };

  virtual void OnFrontDisconnected(int nReason) override {}

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override {
    anon_send(caf::actor_cast<CtpObserver::pointer>(observer_),
              CtpLoginAtom::value);
    if (pRspInfo->ErrorID == 0) {
      std::cout << "OnRspUserLogin\n";
      /*
      CThostFtdcQryOrderField qry_field = {0};
      strcpy(qry_field.BrokerID, pRspUserLogin->BrokerID);
      // strcpy(qry_field.InvestorID, pRspUserLogin->InvestorID);
      cta_api_->ReqQryOrder(&qry_field, 0);
      */

      /*
      {
        CThostFtdcQryInvestorPositionDetailField position_field = {0};
        strcpy(position_field.BrokerID, pRspUserLogin->BrokerID);
        strcpy(position_field.InvestorID, pRspUserLogin->UserID);
        cta_api_->ReqQryInvestorPositionDetail(&position_field, 0);
      }
      */

      /*
      {
        CThostFtdcQryInvestorPositionField position_field = {0};
        strcpy(position_field.BrokerID, pRspUserLogin->BrokerID);
        strcpy(position_field.InvestorID, pRspUserLogin->UserID);
        cta_api_->ReqQryInvestorPosition(&position_field, 0);
      }
      */


      // CThostFtdcQrySettlementInfoField req = { 0 };
      // strcpy(req.BrokerID, broker_id_.c_str());
      // strcpy(req.TradingDay, "20170217");
      // api_->ReqQrySettlementInfo(&req, 0);
    } else {
      std::cout << "User Login Error:" << pRspInfo->ErrorMsg << "\n";
    }
  }

  virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                CThostFtdcRspInfoField* pRspInfo,
                                int nRequestID,
                                bool bIsLast) {
    std::cout << __FUNCTION__ << "\n";
  };

  virtual void OnRspOrderAction(
      CThostFtdcInputOrderActionField* pInputOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) {
    std::cout << __FUNCTION__ << "\n";
  };

  virtual void OnRspBatchOrderAction(
      CThostFtdcInputBatchOrderActionField* pInputBatchOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) {
    std::cout << __FUNCTION__ << "\n";
  };

  virtual void OnRspQryOrder(CThostFtdcOrderField* pOrder,
                             CThostFtdcRspInfoField* pRspInfo,
                             int nRequestID,
                             bool bIsLast) {
    std::cout << __FUNCTION__ << "\n";
  };

  virtual void OnRtnOrder(CThostFtdcOrderField* pOrder) {
    fstream_ << boost::lexical_cast<std::string>(pOrder->BrokerID) << ","
             << boost::lexical_cast<std::string>(pOrder->InvestorID) << ","
             << boost::lexical_cast<std::string>(pOrder->InstrumentID) << ","
             << boost::lexical_cast<std::string>(pOrder->OrderRef) << ","
             << boost::lexical_cast<std::string>(pOrder->UserID) << ","
             << boost::lexical_cast<std::string>(pOrder->OrderPriceType) << ","
             << boost::lexical_cast<std::string>(pOrder->Direction) << ","
             << boost::lexical_cast<std::string>(pOrder->CombOffsetFlag) << ","
             << boost::lexical_cast<std::string>(pOrder->CombHedgeFlag) << ","
             << boost::lexical_cast<std::string>(pOrder->LimitPrice) << ","
             << boost::lexical_cast<std::string>(pOrder->VolumeTotalOriginal)
             << "," << boost::lexical_cast<std::string>(pOrder->TimeCondition)
             << "," << boost::lexical_cast<std::string>(pOrder->GTDDate) << ","
             << boost::lexical_cast<std::string>(pOrder->VolumeCondition) << ","
             << boost::lexical_cast<std::string>(pOrder->MinVolume) << ","
             << boost::lexical_cast<std::string>(pOrder->ContingentCondition)
             << "," << boost::lexical_cast<std::string>(pOrder->StopPrice)
             << ","
             << boost::lexical_cast<std::string>(pOrder->ForceCloseReason)
             << "," << boost::lexical_cast<std::string>(pOrder->IsAutoSuspend)
             << "," << boost::lexical_cast<std::string>(pOrder->BusinessUnit)
             << "," << boost::lexical_cast<std::string>(pOrder->RequestID)
             << "," << boost::lexical_cast<std::string>(pOrder->OrderLocalID)
             << "," << boost::lexical_cast<std::string>(pOrder->ExchangeID)
             << "," << boost::lexical_cast<std::string>(pOrder->ParticipantID)
             << "," << boost::lexical_cast<std::string>(pOrder->ClientID) << ","
             << boost::lexical_cast<std::string>(pOrder->ExchangeInstID) << ","
             << boost::lexical_cast<std::string>(pOrder->TraderID) << ","
             << boost::lexical_cast<std::string>(pOrder->InstallID) << ","
             << boost::lexical_cast<std::string>(pOrder->OrderSubmitStatus)
             << "," << boost::lexical_cast<std::string>(pOrder->NotifySequence)
             << "," << boost::lexical_cast<std::string>(pOrder->TradingDay)
             << "," << boost::lexical_cast<std::string>(pOrder->SettlementID)
             << "," << boost::lexical_cast<std::string>(pOrder->OrderSysID)
             << "," << boost::lexical_cast<std::string>(pOrder->OrderSource)
             << "," << boost::lexical_cast<std::string>(pOrder->OrderStatus)
             << "," << boost::lexical_cast<std::string>(pOrder->OrderType)
             << "," << boost::lexical_cast<std::string>(pOrder->VolumeTraded)
             << "," << boost::lexical_cast<std::string>(pOrder->VolumeTotal)
             << "," << boost::lexical_cast<std::string>(pOrder->InsertDate)
             << "," << boost::lexical_cast<std::string>(pOrder->InsertTime)
             << "," << boost::lexical_cast<std::string>(pOrder->ActiveTime)
             << "," << boost::lexical_cast<std::string>(pOrder->SuspendTime)
             << "," << boost::lexical_cast<std::string>(pOrder->UpdateTime)
             << "," << boost::lexical_cast<std::string>(pOrder->CancelTime)
             << "," << boost::lexical_cast<std::string>(pOrder->ActiveTraderID)
             << "," << boost::lexical_cast<std::string>(pOrder->ClearingPartID)
             << "," << boost::lexical_cast<std::string>(pOrder->SequenceNo)
             << "," << boost::lexical_cast<std::string>(pOrder->FrontID) << ","
             << boost::lexical_cast<std::string>(pOrder->SessionID) << ","
             << boost::lexical_cast<std::string>(pOrder->UserProductInfo) << ","
             << boost::lexical_cast<std::string>(pOrder->StatusMsg) << ","
             << boost::lexical_cast<std::string>(pOrder->UserForceClose) << ","
             << boost::lexical_cast<std::string>(pOrder->ActiveUserID) << ","
             << boost::lexical_cast<std::string>(pOrder->BrokerOrderSeq) << ","
             << boost::lexical_cast<std::string>(pOrder->RelativeOrderSysID)
             << ","
             << boost::lexical_cast<std::string>(pOrder->ZCETotalTradedVolume)
             << "," << boost::lexical_cast<std::string>(pOrder->IsSwapOrder)
             << "," << boost::lexical_cast<std::string>(pOrder->BranchID) << ","
             << boost::lexical_cast<std::string>(pOrder->InvestUnitID) << ","
             << boost::lexical_cast<std::string>(pOrder->AccountID) << ","
             << boost::lexical_cast<std::string>(pOrder->CurrencyID) << ","
             << boost::lexical_cast<std::string>(pOrder->IPAddress) << ","
             << boost::lexical_cast<std::string>(pOrder->MacAddress) << "\n";
    anon_send(caf::actor_cast<CtpObserver::pointer>(observer_),
              CtpRtnOrderAtom::value, CThostFtdcOrderField(*pOrder));
  };

  virtual void OnRtnTrade(CThostFtdcTradeField* pTrade) {
    std::cout << __FUNCTION__ << "\n";
  };

  virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                   CThostFtdcRspInfoField* pRspInfo) {
    std::cout << __FUNCTION__ << "ErrorID:" << pRspInfo->ErrorID
              << ","
                 "ErrorMsg:"
              << pRspInfo->ErrorMsg << "\n";
  };

  virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction,
                                   CThostFtdcRspInfoField* pRspInfo) {
    std::cout << __FUNCTION__ << "\n";
  };

  virtual void OnErrRtnBatchOrderAction(
      CThostFtdcBatchOrderActionField* pBatchOrderAction,
      CThostFtdcRspInfoField* pRspInfo) {
    std::cout << __FUNCTION__ << "\n";
  };

  virtual void OnRspQrySettlementInfo(
      CThostFtdcSettlementInfoField* pSettlementInfo,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) {
    std::cout << pSettlementInfo->Content << "\n";
  };

  virtual void OnRspQryInvestorPosition(
      CThostFtdcInvestorPositionField* pInvestorPosition,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) {
    position_order_fstream_
        << pInvestorPosition->InstrumentID << "," << pInvestorPosition->BrokerID
        << "," << pInvestorPosition->InvestorID << ","
        << pInvestorPosition->PosiDirection << ","
        << pInvestorPosition->HedgeFlag << ","
        << pInvestorPosition->PositionDate << ","
        << pInvestorPosition->YdPosition << "," << pInvestorPosition->Position
        << "," << pInvestorPosition->LongFrozen << ","
        << pInvestorPosition->ShortFrozen << ","
        << pInvestorPosition->LongFrozenAmount << ","
        << pInvestorPosition->ShortFrozenAmount << ","
        << pInvestorPosition->OpenVolume << ","
        << pInvestorPosition->CloseVolume << ","
        << pInvestorPosition->OpenAmount << ","
        << pInvestorPosition->CloseAmount << ","
        << pInvestorPosition->PositionCost << ","
        << pInvestorPosition->PreMargin << "," << pInvestorPosition->UseMargin
        << "," << pInvestorPosition->FrozenMargin << ","
        << pInvestorPosition->FrozenCash << ","
        << pInvestorPosition->FrozenCommission << ","
        << pInvestorPosition->CashIn << "," << pInvestorPosition->Commission
        << "," << pInvestorPosition->CloseProfit << ","
        << pInvestorPosition->PositionProfit << ","
        << pInvestorPosition->PreSettlementPrice << ","
        << pInvestorPosition->SettlementPrice << ","
        << pInvestorPosition->TradingDay << ","
        << pInvestorPosition->SettlementID << "," << pInvestorPosition->OpenCost
        << "," << pInvestorPosition->ExchangeMargin << ","
        << pInvestorPosition->CombPosition << ","
        << pInvestorPosition->CombLongFrozen << ","
        << pInvestorPosition->CombShortFrozen << ","
        << pInvestorPosition->CloseProfitByDate << ","
        << pInvestorPosition->CloseProfitByTrade << ","
        << pInvestorPosition->TodayPosition << ","
        << pInvestorPosition->MarginRateByMoney << ","
        << pInvestorPosition->MarginRateByVolume << ","
        << pInvestorPosition->StrikeFrozen << ","
        << pInvestorPosition->StrikeFrozenAmount << ","
        << pInvestorPosition->AbandonFrozen << "\n";
  };

  virtual void OnRspQryInvestorPositionDetail(
      CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) {
    position_detail_fstream_
        << pInvestorPositionDetail->InstrumentID << ","
        << pInvestorPositionDetail->BrokerID << ","
        << pInvestorPositionDetail->InvestorID << ","
        << pInvestorPositionDetail->HedgeFlag << ","
        << pInvestorPositionDetail->Direction << ","
        << pInvestorPositionDetail->OpenDate << ","
        << pInvestorPositionDetail->TradeID << ","
        << pInvestorPositionDetail->Volume << ","
        << pInvestorPositionDetail->OpenPrice << ","
        << pInvestorPositionDetail->TradingDay << ","
        << pInvestorPositionDetail->SettlementID << ","
        << pInvestorPositionDetail->TradeType << ","
        << pInvestorPositionDetail->CombInstrumentID << ","
        << pInvestorPositionDetail->ExchangeID << ","
        << pInvestorPositionDetail->CloseProfitByDate << ","
        << pInvestorPositionDetail->CloseProfitByTrade << ","
        << pInvestorPositionDetail->PositionProfitByDate << ","
        << pInvestorPositionDetail->PositionProfitByTrade << ","
        << pInvestorPositionDetail->Margin << ","
        << pInvestorPositionDetail->ExchMargin << ","
        << pInvestorPositionDetail->MarginRateByMoney << ","
        << pInvestorPositionDetail->MarginRateByVolume << ","
        << pInvestorPositionDetail->LastSettlementPrice << ","
        << pInvestorPositionDetail->SettlementPrice << ","
        << pInvestorPositionDetail->CloseVolume << ","
        << pInvestorPositionDetail->CloseAmount << "\n";
  };

 private:
  CThostFtdcTraderApi* cta_api_;
  CThostFtdcTraderApi* follower_api_;
  caf::strong_actor_ptr observer_;
  // OrderNo -> ASDF
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  std::ofstream fstream_;
  std::ofstream position_detail_fstream_;
  std::ofstream position_order_fstream_;
};

#endif /* CTP_TRADER_H */
