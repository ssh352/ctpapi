#ifndef CTP_BIND_DEMO_MD_H
#define CTP_BIND_DEMO_MD_H
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2/signal.hpp>
#include "ctpapi/ThostFtdcMdApi.h"

namespace ctp_bind {
class MdObserver;
typedef boost::signals2::signal<void(const CThostFtdcDepthMarketDataField*)>
    MdSingnal;
class Md : public CThostFtdcMdSpi {
 public:
  Md(std::string server,
     std::string broker_id,
     std::string user_id,
     std::string password);
  ~Md();

  void InitAsio(boost::asio::io_service* io_service = nullptr);

  void Connect(std::function<void(CThostFtdcRspUserLoginField*,
                                  CThostFtdcRspInfoField*)> callback);

  void Run();

  void Subscribe(
      std::vector<std::pair<std::string, MdSingnal::slot_type> > instruments_slots);

  void Unsbscribe(std::vector<std::string> instruments);

  virtual void OnFrontConnected() override;

  virtual void OnFrontDisconnected(int nReason) override;

  virtual void OnHeartBeatWarning(int nTimeLapse) override;

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override;

  virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                               CThostFtdcRspInfoField* pRspInfo,
                               int nRequestID,
                               bool bIsLast) override;

  virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo,
                          int nRequestID,
                          bool bIsLast) override;

  virtual void OnRspSubMarketData(
      CThostFtdcSpecificInstrumentField* pSpecificInstrument,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspUnSubMarketData(
      CThostFtdcSpecificInstrumentField* pSpecificInstrument,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspSubForQuoteRsp(
      CThostFtdcSpecificInstrumentField* pSpecificInstrument,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRspUnSubForQuoteRsp(
      CThostFtdcSpecificInstrumentField* pSpecificInstrument,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) override;

  virtual void OnRtnDepthMarketData(
      CThostFtdcDepthMarketDataField* pDepthMarketData) override;

  virtual void OnRtnForQuoteRsp(
      CThostFtdcForQuoteRspField* pForQuoteRsp) override;

 private:
  CThostFtdcMdApi* api_;
  boost::asio::io_service* io_service_;
  std::shared_ptr<boost::asio::io_service::work> io_worker_;
  std::map<std::string, MdSingnal> callbacks_;

  std::function<void(CThostFtdcRspUserLoginField* field,
                     CThostFtdcRspInfoField* rsp_info)>
      on_connect_;

  std::string server_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
};
}  // namespace ctp_bind

#endif  // CTP_BIND_DEMO_MD_H
