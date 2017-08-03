#ifndef FOLLOW_TRADE_SERVER_CTA_SIGNAL_TRADER_H
#define FOLLOW_TRADE_SERVER_CTA_SIGNAL_TRADER_H

#include <atomic>
#include <boost/any.hpp>
#include <boost/asio.hpp>
#include <boost/bimap.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/variant.hpp>
#include <unordered_set>
#include "caf/all.hpp"
#include "common/api_struct.h"
#include "ctpapi/ThostFtdcTraderApi.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"

class CTASignalTrader : public caf::event_based_actor,
                        public CThostFtdcTraderSpi {
 public:
  CTASignalTrader(caf::actor_config& cfg,
                  std::string server,
                  std::string broker_id,
                  std::string user_id,
                  std::string password);
  ~CTASignalTrader();

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override;

  virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                               CThostFtdcRspInfoField* pRspInfo,
                               int nRequestID,
                               bool bIsLast) override;

  virtual void OnRtnOrder(CThostFtdcOrderField* pOrder) override;

  virtual void OnFrontConnected() override;

  virtual void OnFrontDisconnected(int nReason) override;

  virtual void OnRspQryInvestorPosition(
      CThostFtdcInvestorPositionField* pInvestorPosition,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo,
                          int nRequestID,
                          bool bIsLast) override;

 protected:
  virtual caf::behavior make_behavior() override;

 private:
  void CallOnActor(std::function<void(void)> func);

  void OnRtnOrderOnIOThread(boost::shared_ptr<CThostFtdcOrderField> order);

  std::string MakeOrderId(TThostFtdcFrontIDType front_id,
                          TThostFtdcSessionIDType session_id,
                          const std::string& order_ref) const;

  OrderStatus ParseTThostFtdcOrderStatus(
      boost::shared_ptr<CThostFtdcOrderField> order) const;

  PositionEffect ParseTThostFtdcPositionEffect(TThostFtdcOffsetFlagType flag);

  bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) const;
  TThostFtdcDirectionType OrderDirectionToTThostOrderDireciton(
      OrderDirection direction);

  TThostFtdcOffsetFlagType PositionEffectToTThostOffsetFlag(
      PositionEffect position_effect);

  CThostFtdcTraderApi* api_;

  std::list<boost::shared_ptr<OrderField>> sequence_orders_;
  std::set<caf::strong_actor_ptr> rtn_order_observers_;

  caf::strong_actor_ptr db_;
  std::atomic<int> request_id_ = 0;
  std::atomic<int> order_ref_index_ = 0;
  std::string server_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  TThostFtdcSessionIDType session_id_ = 0;
  TThostFtdcFrontIDType front_id_ = 0;
  std::vector<OrderPosition> yesterday_positions_;
};

#endif  // FOLLOW_TRADE_SERVER_CTA_SIGNAL_TRADER_H
