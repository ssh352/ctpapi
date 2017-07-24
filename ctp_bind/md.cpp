#include "md.h"
#include "md_observer.h"

ctp_bind::Md::Md(std::string server,
                 std::string broker_id,
                 std::string user_id,
                 std::string password)
    : server_(std::move(server)),
      broker_id_(std::move(broker_id)),
      user_id_(std::move(user_id)),
      password_(std::move(password)) {}

ctp_bind::Md::~Md() {
  delete io_service_;
}

void ctp_bind::Md::InitAsio(boost::asio::io_service* io_service) {
  if (io_service == NULL) {
    io_service_ = new boost::asio::io_service();
  } else {
    io_service_ = io_service;
  }
  io_worker_ = std::make_shared<boost::asio::io_service::work>(*io_service_);
}

void ctp_bind::Md::Connect(
    std::function<void(CThostFtdcRspUserLoginField*, CThostFtdcRspInfoField*)>
        callback) {
  api_ = CThostFtdcMdApi::CreateFtdcMdApi();
  api_->RegisterSpi(this);
  api_->RegisterFront(const_cast<char*>(server_.c_str()));
  api_->Init();
  on_connect_ = callback;
}

void ctp_bind::Md::Run() {
  io_service_->run();
}

void ctp_bind::Md::Subscribe(
    std::vector<std::string> instruments,
    std::function<void(const CThostFtdcDepthMarketDataField*)> callback,
    boost::shared_ptr<MdObserver> tracker) {
  MdSlotType::slot_type slot(callback);
  slot.track(tracker);
  io_service_->post([ =, instruments{std::move(instruments)} ](void) {
    std::vector<char*> ctp_instruments;
    for (auto& instrument : instruments) {
      if (callbacks_.find(instrument) != callbacks_.end()) {
        callbacks_[instrument].connect(slot);
      } else {
        ctp_instruments.push_back(const_cast<char*>(instrument.c_str()));
        boost::signals2::signal<void(const CThostFtdcDepthMarketDataField*)>
            signal;
        signal.connect(slot);
        callbacks_.insert(std::make_pair(instrument, std::move(signal)));
      }
    }
    if (!ctp_instruments.empty()) {
      api_->SubscribeMarketData(&ctp_instruments[0],
                                static_cast<int>(ctp_instruments.size()));
    }
  });
}

void ctp_bind::Md::Unsbscribe(std::vector<std::string> instruments) {}

void ctp_bind::Md::OnFrontConnected() {
  CThostFtdcReqUserLoginField field{0};
  strcpy(field.UserID, user_id_.c_str());
  strcpy(field.Password, password_.c_str());
  strcpy(field.BrokerID, broker_id_.c_str());
  api_->ReqUserLogin(&field, 0);
}

void ctp_bind::Md::OnFrontDisconnected(int nReason) {}

void ctp_bind::Md::OnHeartBeatWarning(int nTimeLapse) {}

void ctp_bind::Md::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                                  CThostFtdcRspInfoField* pRspInfo,
                                  int nRequestID,
                                  bool bIsLast) {
  on_connect_(pRspUserLogin, pRspInfo);
}

void ctp_bind::Md::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                                   CThostFtdcRspInfoField* pRspInfo,
                                   int nRequestID,
                                   bool bIsLast) {}

void ctp_bind::Md::OnRspError(CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) {}

void ctp_bind::Md::OnRspSubMarketData(
    CThostFtdcSpecificInstrumentField* pSpecificInstrument,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {}

void ctp_bind::Md::OnRspUnSubMarketData(
    CThostFtdcSpecificInstrumentField* pSpecificInstrument,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {}

void ctp_bind::Md::OnRspSubForQuoteRsp(
    CThostFtdcSpecificInstrumentField* pSpecificInstrument,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {}

void ctp_bind::Md::OnRspUnSubForQuoteRsp(
    CThostFtdcSpecificInstrumentField* pSpecificInstrument,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {}

void ctp_bind::Md::OnRtnDepthMarketData(
    CThostFtdcDepthMarketDataField* pDepthMarketData) {
  CThostFtdcDepthMarketDataField data(*pDepthMarketData);
  io_service_->post([ =, data{std::move(data)} ](void) {
    if (callbacks_.find(data.InstrumentID) != callbacks_.end()) {
      callbacks_[data.InstrumentID](&data);
    }
  });
}

void ctp_bind::Md::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField* pForQuoteRsp) {}
