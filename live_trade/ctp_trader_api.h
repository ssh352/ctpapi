#ifndef LIVE_TRADE_CTP_TRADER_API_H
#define LIVE_TRADE_CTP_TRADER_API_H

#include <unordered_map>
#include <boost/format.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/functional/hash.hpp>
#include "ctpapi/ThostFtdcTraderApi.h"
#include "common/api_struct.h"

class CTPTraderApi : public CThostFtdcTraderSpi {
 public:
  class Delegate {
   public:
    virtual void HandleCtpLogon(int front_id, int session_id) = 0;
    virtual void HandleCTPRtnOrder(
        const std::shared_ptr<CTPOrderField>& order) = 0;

    virtual void HandleCTPTradeOrder(const std::string& instrument,
                                     const std::string& order_id,
                                     double trading_price,
                                     int trading_qty,
                                     TimeStamp timestamp) = 0;

    virtual void HandleRspYesterdayPosition(
        std::vector<OrderPosition> yesterday_positions) = 0;
  };
  CTPTraderApi(Delegate* delegate, const std::string& ctp_flow_path);

  void Connect(const std::string& server,
               std::string broker_id,
               std::string user_id,
               std::string password);

  void InputOrder(const CTPEnterOrder& order, const std::string& order_id);

  void CancelOrder(const CTPCancelOrder& ctp_cancel);

  void RequestYesterdayPosition();

  virtual void OnRspQryInvestorPosition(
      CThostFtdcInvestorPositionField* pInvestorPosition,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast);

  virtual void OnFrontConnected() override;

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override;

  void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                       CThostFtdcRspInfoField* pRspInfo,
                       int nRequestID,
                       bool bIsLast);

  virtual void OnRtnOrder(CThostFtdcOrderField* pOrder) override;

  virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo,
                          int nRequestID,
                          bool bIsLast) override;

  virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                CThostFtdcRspInfoField* pRspInfo,
                                int nRequestID,
                                bool bIsLast) override;

  virtual void OnRspOrderAction(
      CThostFtdcInputOrderActionField* pInputOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  std::string MakeCtpUniqueOrderId(int front_id,
                                   int session_id,
                                   const std::string& order_ref) const;

  std::string MakeCtpUniqueOrderId(const std::string& order_ref) const;

  CTPPositionEffect ParseTThostFtdcPositionEffect(
      TThostFtdcOffsetFlagType flag);

  OrderStatus ParseTThostFtdcOrderStatus(CThostFtdcOrderField* order) const;

  TThostFtdcDirectionType OrderDirectionToTThostOrderDireciton(
      OrderDirection direction);

  TThostFtdcOffsetFlagType PositionEffectToTThostOffsetFlag(
      CTPPositionEffect position_effect);

  virtual void OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                   CThostFtdcRspInfoField* pRspInfo) override;

  virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField* pOrderAction,
                                   CThostFtdcRspInfoField* pRspInfo) override;

  virtual void OnRtnTrade(CThostFtdcTradeField* pTrade) override;

  void OnRspQrySettlementInfoConfirm(
      CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast);

  void OnRspSettlementInfoConfirm(
      CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast);
 private:
  void SettlementInfoConfirm();

  struct ExchangeIdOrderSysId {
    std::string exchange_id;
    std::string order_sys_id;
  };

  struct HashExchagneIdOrderSysId {
    size_t operator()(const ExchangeIdOrderSysId& id) const {
      size_t seed = 0;
      boost::hash_combine(seed, id.exchange_id);
      boost::hash_combine(seed, id.order_sys_id);
      return seed;
    }
  };

  struct CompareExchangeIdOrderSysId {
    bool operator()(const ExchangeIdOrderSysId& l,
                    const ExchangeIdOrderSysId& r) const {
      return l.exchange_id == r.exchange_id && l.order_sys_id == r.order_sys_id;
    }
  };

  Delegate* delegate_;
  CThostFtdcTraderApi* api_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  TThostFtdcSessionIDType session_id_ = 0;
  TThostFtdcFrontIDType front_id_ = 0;

  std::unordered_map<ExchangeIdOrderSysId,
                     std::string,
                     HashExchagneIdOrderSysId,
                     CompareExchangeIdOrderSysId>
      order_sys_id_to_order_id_;
  std::vector<OrderPosition> rsp_yesterday_position_cache_;
};

#endif  // LIVE_TRADE_CTP_TRADER_API_H
