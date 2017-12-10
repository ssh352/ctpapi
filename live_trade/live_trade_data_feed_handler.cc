#include "live_trade_data_feed_handler.h"
#include "bft_core/make_message.h"

void LiveTradeDataFeedHandler::OnRspError(CThostFtdcRspInfoField* pRspInfo,
                                          int nRequestID,
                                          bool bIsLast) {}

void LiveTradeDataFeedHandler::OnRtnDepthMarketData(
    CThostFtdcDepthMarketDataField* pDepthMarketData) {
  auto tick = std::make_shared<TickData>();
  tick->instrument =
      std::make_shared<std::string>(pDepthMarketData->InstrumentID);
  tick->tick = std::make_shared<Tick>();
  tick->tick->ask_price1 = pDepthMarketData->AskPrice1;
  tick->tick->bid_price1 = pDepthMarketData->BidPrice1;
  tick->tick->ask_qty1 = pDepthMarketData->AskVolume1;
  tick->tick->bid_qty1 = pDepthMarketData->BidVolume1;
  tick->tick->last_price = pDepthMarketData->LastPrice;
  tick->tick->timestamp =
      ptime_to_timestamp(boost::posix_time::microsec_clock::local_time());
  live_trade_system_->SendToGlobal(bft::MakeMessage(std::move(tick)));
}

void LiveTradeDataFeedHandler::OnRspUserLogout(
    CThostFtdcUserLogoutField* pUserLogout,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {}

void LiveTradeDataFeedHandler::OnRspUserLogin(
    CThostFtdcRspUserLoginField* pRspUserLogin,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  if (pRspInfo != NULL && pRspInfo->ErrorID == 0) {
  } else {
  }
}

void LiveTradeDataFeedHandler::OnFrontDisconnected(int nReason) {}

void LiveTradeDataFeedHandler::OnFrontConnected() {
  CThostFtdcReqUserLoginField reqUserLogin;
  strcpy(reqUserLogin.BrokerID, broker_id_.c_str());
  strcpy(reqUserLogin.UserID, user_id_.c_str());
  strcpy(reqUserLogin.Password, password_.c_str());
  api_->ReqUserLogin(&reqUserLogin, 0);
}

void LiveTradeDataFeedHandler::Connect(const std::string& server,
                                       std::string broker_id,
                                       std::string user_id,
                                       std::string password) {}

LiveTradeDataFeedHandler::LiveTradeDataFeedHandler(
    caf::actor_config& cfg,
    LiveTradeSystem* live_trade_system)
    : caf::event_based_actor(cfg), live_trade_system_(live_trade_system) {
  api_ = CThostFtdcMdApi::CreateFtdcMdApi();
  api_->RegisterSpi(this);
  live_trade_system_->Subscribe(
      typeid(std::tuple<std::shared_ptr<OrderField>, CTAPositionQty>), this);
}

caf::behavior LiveTradeDataFeedHandler::make_behavior() {
  return {
      [=](CtpConnectAtom, const std::string& server,
          const std::string& broker_id, const std::string& user_id,
          const std::string& password) {
        broker_id_ = broker_id;
        user_id_ = user_id;
        password_ = password;
        char fron_server[255] = {0};
        strcpy(fron_server, server.c_str());
        api_->RegisterFront(fron_server);
        api_->Init();
      },
      [=](const std::shared_ptr<OrderField>& rtn_order,
          const CTAPositionQty& /*position_qty*/) {
        if (instruments_.find(rtn_order->instrument_id) == instruments_.end()) {
          TThostFtdcInstrumentIDType ftdc_instrument;
          strcpy(ftdc_instrument, rtn_order->instrument_id.c_str());
          char* instruments[] = {ftdc_instrument};
          api_->SubscribeMarketData(instruments, 1);
          instruments_.insert(rtn_order->instrument_id);
        }
      }};
}

void LiveTradeDataFeedHandler::HandleCTARtnOrderSignal(
    const std::shared_ptr<OrderField>& rtn_order,
    const CTAPositionQty& position_qty) {}
