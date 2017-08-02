#ifndef FOLLOW_TRADE_SERVER_STRATEGY_TRADER_H
#define FOLLOW_TRADE_SERVER_STRATEGY_TRADER_H

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

using SubscribeRtnOrderAtom = caf::atom_constant<caf::atom("subro")>;
class StrategyTrader : public caf::event_based_actor,
                       public CThostFtdcTraderSpi {
 public:
  StrategyTrader(caf::actor_config& cfg,
                 caf::group rtn_order_grp,
                 std::string server,
                 std::string broker_id,
                 std::string user_id,
                 std::string password);

  void LimitOrder(std::string sub_account_id,
                  std::string sub_order_id,
                  std::string instrument,
                  PositionEffect position_effect,
                  OrderDirection direction,
                  double price,
                  int volume);

  void CancelOrder(std::string sub_accont_id, std::string sub_order_id);

  // void CancelOrder(std::string order_id);

  //   void SubscribeRtnOrder(
  //       std::string sub_account_id,
  //       std::function<void(boost::shared_ptr<OrderField>)> callback);
  //
  //   void SubscribeRtnOrder(
  //       std::function<void(boost::shared_ptr<OrderField>)> callback);

  void ReqInvestorPosition(
      std::function<void(InvestorPositionField, bool)> callback);

  virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                CThostFtdcRspInfoField* pRspInfo,
                                int nRequestID,
                                bool bIsLast) override;

  virtual void OnRspOrderAction(
      CThostFtdcInputOrderActionField* pInputOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo,
                          int nRequestID,
                          bool bIsLast) override;

  virtual void OnRspAuthenticate(
      CThostFtdcRspAuthenticateField* pRspAuthenticateField,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

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

 private:
  void CallOnActor(std::function<void(void)> func);

  void CancelOrderOnIOThread(std::string sub_accont_id, std::string order_id);

  void OnRtnOrderOnIOThread(boost::shared_ptr<CThostFtdcOrderField> order);

  std::string MakeOrderId(TThostFtdcFrontIDType front_id,
                          TThostFtdcSessionIDType session_id,
                          const std::string& order_ref) const;

  OrderStatus ParseTThostFtdcOrderStatus(
      boost::shared_ptr<CThostFtdcOrderField> order) const;

  PositionEffect ParseTThostFtdcPositionEffect(TThostFtdcOffsetFlagType flag);

  CThostFtdcTraderApi* api_;

  std::unordered_map<std::string, boost::shared_ptr<CThostFtdcOrderField>>
      orders_;

  std::list<boost::shared_ptr<OrderField>> sequence_orders_;

  typedef boost::bimap<std::pair<std::string, std::string>, std::string>
      SubOrderIDBiomap;

  SubOrderIDBiomap sub_order_ids_;

  std::unordered_map<std::string, std::set<caf::strong_actor_ptr>>
      sub_account_on_rtn_order_callbacks_;

  std::unordered_map<int, boost::any> response_;

  std::function<void(CThostFtdcRspUserLoginField*, CThostFtdcRspInfoField*)>
      on_connect_;

  caf::strong_actor_ptr db_;
  std::atomic<int> request_id_ = 0;
  std::atomic<int> order_ref_index_ = 0;
  std::string server_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  TThostFtdcSessionIDType session_id_ = 0;
  TThostFtdcFrontIDType front_id_ = 0;
  bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) const;
  TThostFtdcDirectionType OrderDirectionToTThostOrderDireciton(
      OrderDirection direction);
  TThostFtdcOffsetFlagType PositionEffectToTThostOffsetFlag(
      PositionEffect position_effect);
  caf::group rtn_order_grp_;

 protected:
  virtual caf::behavior make_behavior() override;
};


#endif // FOLLOW_TRADE_SERVER_STRATEGY_TRADER_H



