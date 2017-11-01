#ifndef LIVE_TRADE_CTP_TRADER_API_H
#define LIVE_TRADE_CTP_TRADER_API_H

#include <unordered_map>
#include <boost/format.hpp>
#include "ctpapi/ThostFtdcTraderApi.h"
#include "common/api_struct.h"

class CTPTraderApi : public CThostFtdcTraderSpi {
 public:
   class Delegate {
   public:
     virtual void HandleCTPRtnOrder(const std::shared_ptr<CTPOrderField>& order) = 0;
   };
  CTPTraderApi(Delegate* delegate);

  void Connect(const std::string& server,
               std::string broker_id,
               std::string user_id,
               std::string password);

  void HandleInputOrder(const CTPEnterOrder& order, const std::string& order_id);

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


  std::string MakeOrderId(const std::string& order_ref) const;

  std::string MakeOrderId(TThostFtdcFrontIDType front_id,
                          TThostFtdcSessionIDType session_id,
                          const std::string& order_ref) const;


  CTPPositionEffect ParseTThostFtdcPositionEffect(TThostFtdcOffsetFlagType flag);

  OrderStatus ParseTThostFtdcOrderStatus(CThostFtdcOrderField* order) const;

  TThostFtdcDirectionType OrderDirectionToTThostOrderDireciton(
      OrderDirection direction);

  TThostFtdcOffsetFlagType PositionEffectToTThostOffsetFlag(
      CTPPositionEffect position_effect);

  Delegate* delegate_;
  CThostFtdcTraderApi* api_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  TThostFtdcSessionIDType session_id_ = 0;
  TThostFtdcFrontIDType front_id_ = 0;

  std::unordered_map<std::string, int> order_traded_qty_set_;
};

#endif  // LIVE_TRADE_CTP_TRADER_API_H
