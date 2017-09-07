#ifndef LIVE_TRADE_LIVE_TRADE_DATA_FEED_HANDLER_H
#define LIVE_TRADE_LIVE_TRADE_DATA_FEED_HANDLER_H
#include <boost/date_time/posix_time/posix_time.hpp>
#include "caf/all.hpp"
#include "ctpapi/ThostFtdcMdApi.h"
#include "../hpt_core/time_util.h"

template <typename MailBox>
class LiveTradeDataFeedHandler : public CThostFtdcMdSpi {
 public:
  LiveTradeDataFeedHandler(MailBox* mail_box) : mail_box_(mail_box) {
    api_ = CThostFtdcMdApi::CreateFtdcMdApi();
    api_->RegisterSpi(this);
  }

  void SubscribeInstrument(std::string instrument) {
    instrument_ = std::make_shared<std::string>(std::move(instrument));
  }

  void Connect(const std::string& server,
               std::string broker_id,
               std::string user_id,
               std::string password) {
    broker_id_ = std::move(broker_id);
    user_id_ = std::move(user_id);
    password_ = std::move(password);
    char fron_server[255] = {0};
    strcpy(fron_server, server.c_str());
    api_->RegisterFront(fron_server);
    api_->Init();
  }

  virtual void OnFrontConnected() override {
    CThostFtdcReqUserLoginField reqUserLogin;
    strcpy(reqUserLogin.BrokerID, broker_id_.c_str());
    strcpy(reqUserLogin.UserID, user_id_.c_str());
    strcpy(reqUserLogin.Password, password_.c_str());
    api_->ReqUserLogin(&reqUserLogin, 0);
  }

  virtual void OnFrontDisconnected(int nReason) override {}

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override {
    if (pRspInfo != NULL && pRspInfo->ErrorID == 0) {
      TThostFtdcInstrumentIDType ftdc_instrument;
      strcpy(ftdc_instrument, instrument_->c_str());
      char* instruments[] = {ftdc_instrument};
      api_->SubscribeMarketData(instruments, 1);
    } else {
    }
  }

  virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                               CThostFtdcRspInfoField* pRspInfo,
                               int nRequestID,
                               bool bIsLast) override {}

  virtual void OnRtnDepthMarketData(
      CThostFtdcDepthMarketDataField* pDepthMarketData) override {
    auto tick = std::make_shared<TickData>();
    tick->instrument = instrument_;
    tick->tick = std::make_shared<Tick>();
    tick->tick->ask_price1 = pDepthMarketData->AskPrice1;
    tick->tick->bid_price1 = pDepthMarketData->BidPrice1;
    tick->tick->ask_qty1 = pDepthMarketData->AskVolume1;
    tick->tick->bid_qty1 = pDepthMarketData->BidVolume1;
    tick->tick->last_price = pDepthMarketData->LastPrice;
    tick->tick->timestamp =
        ptime_to_timestamp(boost::posix_time::microsec_clock::local_time());
    mail_box_->Send(std::move(tick));
  }

  virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo,
                          int nRequestID,
                          bool bIsLast) override {}

 private:
  MailBox* mail_box_;

  CThostFtdcMdApi* api_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  std::shared_ptr<std::string> instrument_;
};

#endif  // LIVE_TRADE_LIVE_TRADE_DATA_FEED_HANDLER_H
