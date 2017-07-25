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
#include "ctpapi/ThostFtdcTraderApi.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"

namespace ctp_bind {
class Trader : public CThostFtdcTraderSpi {
  template <typename Field>
  struct CTPCallbackVisitor : boost::static_visitor<> {
    CTPCallbackVisitor(Field field,
                       CThostFtdcRspInfoField* rsp_info,
                       bool is_last)
        : field_(field), rsp_info_(rsp_info), is_last_(is_last) {}

    template <typename T>
    void operator()(T callback) const {
      std::cout << "";
    }

    void operator()(std::function<void(Field, CThostFtdcRspInfoField*, bool)>
                        callback) const {
      callback(field_, rsp_info_, is_last_);
    }

    Field field_;
    CThostFtdcRspInfoField* rsp_info_;
    bool is_last_;
  };

  struct CTPRspErrorCallbackVisitor : boost::static_visitor<> {
    CTPRspErrorCallbackVisitor(CThostFtdcRspInfoField* rsp_info, bool is_last)
        : rsp_info_(rsp_info), is_last_(is_last) {}

    template <typename Field>
    void operator()(std::function<void(Field, CThostFtdcRspInfoField*, bool)>
                        callback) const {
      callback(NULL, rsp_info_, is_last_);
    }

    CThostFtdcRspInfoField* rsp_info_;
    bool is_last_;
  };

  // virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
  // CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
  typedef boost::variant<
      std::function<
          void(CThostFtdcInputOrderField*, CThostFtdcRspInfoField*, bool)>,
      std::function<void(CThostFtdcInputOrderActionField*,
                         CThostFtdcRspInfoField*,
                         bool)> >
      CTPCallback;

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
      std::function<void(boost::shared_ptr<CThostFtdcOrderField>)> callback);

  template <typename Fun, typename Field, typename Callback>
  void Request(Fun f, Field* field, Callback cb) {
    // CTPCallback c = cb;
    int request_id = request_id_.fetch_add(1);
    io_service_->post(
        [=](void) { ctp_callbacks_.insert(std::make_pair(request_id, cb)); });
    boost::bind(f, api_, field, request_id)();
  }

  template <typename Field>
  auto MakeHandleResponse(Field field,
                          CThostFtdcRspInfoField rsp_info,
                          int request_id,
                          bool is_last) {
    return [ =, field{std::move(field)},
             rsp_info{std::move(rsp_info)} ](void) mutable {
      if (ctp_callbacks_.find(request_id) != ctp_callbacks_.end()) {
        boost::apply_visitor(
            CTPCallbackVisitor<Field*>{&field, &rsp_info, is_last},
            ctp_callbacks_[request_id]);
      }
    };
  }

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

  std::string MakeOrderId(boost::shared_ptr<CThostFtdcOrderField> order);

  std::map<int, CTPCallback> ctp_callbacks_;
  CThostFtdcTraderApi* api_;
  boost::asio::io_service* io_service_;
  std::shared_ptr<boost::asio::io_service::work> io_worker_;

  std::unordered_map<std::string, boost::shared_ptr<CThostFtdcOrderField> >
      orders_;

  std::function<void(CThostFtdcRspUserLoginField*, CThostFtdcRspInfoField*)>
      on_connect_;

  std::function<void(boost::shared_ptr<CThostFtdcOrderField>)> on_rtn_order_;
  
  std::atomic<int> request_id_ = 0;
  std::string server_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  TThostFtdcSessionIDType session_id_ = 0;
  TThostFtdcFrontIDType front_id_ = 0;
};
}  // namespace ctp_bind

#endif  // CTP_BIND_DEMO_TRADER_H
