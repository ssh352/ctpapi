#ifndef CTP_BIND_DEMO_TRADER_H
#define CTP_BIND_DEMO_TRADER_H
#include <atomic>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/variant.hpp>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include "api_struct.h"
#include "ctpapi/ThostFtdcTraderApi.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"

namespace ctp_bind {
class Trader : public CThostFtdcTraderSpi {
  public:
  Trader(std::string server,
         std::string broker_id,
         std::string user_id,
         std::string password);

  void InitAsio(boost::asio::io_service* io_service = nullptr);

  void Connect(std::function<void(CThostFtdcRspUserLoginField*,
                                  CThostFtdcRspInfoField*)> callback);

  void Run();

  void OpenOrder(std::string instrument,
                 TThostFtdcDirectionType direction,
                 double price,
                 int volume,
                 std::string order_ref = "");

  void CloseOrder(std::string instrument,
                  TThostFtdcDirectionType direction,
                  double price,
                  int volume,
                  std::string order_ref = "");

  void CancelOrder(std::string order_id);

  void SubscribeRtnOrder(
      std::function<void(boost::shared_ptr<OrderField>)> callback);

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

 private:
  void ReqCancelOrder(boost::shared_ptr<CThostFtdcOrderField> order);

  void CancelOrderOnIOThread(std::string);

  void OnRtnOrderOnIOThread(boost::shared_ptr<CThostFtdcOrderField> order);

  std::string MakeOrderId(TThostFtdcFrontIDType front_id, TThostFtdcSessionIDType session_id, const std::string& order_ref) const;

  OrderStatus ParseTThostFtdcOrderStatus(
      boost::shared_ptr<CThostFtdcOrderField> order) const;

  PositionEffect ParseTThostFtdcPositionEffect(TThostFtdcOffsetFlagType flag);

  CThostFtdcTraderApi* api_;
  boost::asio::io_service* io_service_;
  std::shared_ptr<boost::asio::io_service::work> io_worker_;

  std::unordered_map<std::string, boost::shared_ptr<CThostFtdcOrderField> >
      orders_;

  std::function<void(CThostFtdcRspUserLoginField*, CThostFtdcRspInfoField*)>
      on_connect_;

  std::function<void(boost::shared_ptr<OrderField>)> on_rtn_order_;

  std::atomic<int> request_id_ = 0;
  std::string server_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  TThostFtdcSessionIDType session_id_ = 0;
  TThostFtdcFrontIDType front_id_ = 0;
  bool IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) const;
};
}  // namespace ctp_bind

#endif  // CTP_BIND_DEMO_TRADER_H
