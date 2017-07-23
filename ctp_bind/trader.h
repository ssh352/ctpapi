#ifndef CTP_BIND_DEMO_TRADER_H
#define CTP_BIND_DEMO_TRADER_H
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/variant.hpp>
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
  // virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
  // CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};
  typedef boost::variant<
      std::function<
          void(CThostFtdcInputOrderField*, CThostFtdcRspInfoField*, bool)>,
      std::function<void(CThostFtdcInputOrderActionField*,
                         CThostFtdcRspInfoField*,
                         bool)>>
      CTPCallback;

 public:
  void Run() { io_service_.run(); }
  template <typename Fun, typename Field, typename Callback>
  void Request(Fun f, Field* field, Callback cb) {
    // CTPCallback c = cb;
    int request_id = request_id_;
    io_service_.post(
        [=](void) { 
      ctp_callbacks_.insert(std::make_pair(request_id, cb)); 
    });
    // boost::bind(f, api_, field, request_id_)();
    ++request_id_;
  }

  template <typename Field>
  auto MakeHandleResponse(Field field,
                          CThostFtdcRspInfoField rsp_info,
                          int request_id,
                          bool is_last) {
    CTPCallbackVisitor<Field*> visitor(&field, &rsp_info, is_last);
    return [=, field{std::move(field)}, rsp_info{std::move(rsp_info)}](void) {
      if (ctp_callbacks_.find(request_id) != ctp_callbacks_.end()) {
        boost::apply_visitor(visitor, ctp_callbacks_[request_id]);
      }
    };
  }

  virtual void OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                CThostFtdcRspInfoField* pRspInfo,
                                int nRequestID,
                                bool bIsLast) override {
    io_service_.post(MakeHandleResponse(CThostFtdcInputOrderField(*pInputOrder),
                                        CThostFtdcRspInfoField(*pRspInfo),
                                        nRequestID, bIsLast));
    /*
    io_service_.post([=](void) {
    });
    */
  }

  virtual void OnRspOrderAction(
      CThostFtdcInputOrderActionField* pInputOrderAction,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override {
    io_service_.post(MakeHandleResponse(
        CThostFtdcInputOrderActionField(*pInputOrderAction),
        CThostFtdcRspInfoField(*pRspInfo), nRequestID, bIsLast));
  }

 private:
  int request_id_ = 0;
  std::map<int, CTPCallback> ctp_callbacks_;
  CThostFtdcTraderApi* api_;
  boost::asio::io_service io_service_;
};
}  // namespace ctp_bind

#endif  // CTP_BIND_DEMO_TRADER_H
