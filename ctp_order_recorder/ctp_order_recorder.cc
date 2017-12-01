#include "ctp_order_recorder.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/variant.hpp>
#include "thost_field_fusion_adapt.h"
#include "Timer.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>

boost::filesystem::path CreateDirectoryIfNoExist(const std::string& sub_dir) {
  boost::filesystem::path dir{boost::filesystem::current_path()};
  dir /= sub_dir;
  if (!boost::filesystem::exists(dir) &&
      !boost::filesystem::create_directory(dir)) {
    std::cout << "create directory error\n";
  }
  return dir;
}

std::string MakeDateBinaryFileName(const std::string& user_id) {
  auto time = boost::posix_time::second_clock::local_time();
  struct ::tm tm_time = boost::posix_time::to_tm(time);
  std::ostringstream time_pid_stream;
  time_pid_stream.fill('0');
  time_pid_stream << 1900 + tm_time.tm_year << std::setw(2)
                  << 1 + tm_time.tm_mon << std::setw(2) << tm_time.tm_mday
                  << std::setw(2) << tm_time.tm_hour << std::setw(2)
                  << tm_time.tm_min << std::setw(2) << tm_time.tm_sec;
  const std::string& time_pid_string = time_pid_stream.str();

  boost::filesystem::path dir = CreateDirectoryIfNoExist(user_id);
  return dir.string() + "\\" + user_id + '_' + time_pid_string + ".bin";
}

void CtpOrderRecorder::OnRtnOrder(CThostFtdcOrderField* pOrder) {
  boost::variant<CThostFtdcOrderField, CThostFtdcTradeField> v(*pOrder);
  oa_ << NanoTimer::getInstance()->getNano();
  boost::serialization::save(oa_, v, 0);
}

void CtpOrderRecorder::OnRtnTrade(CThostFtdcTradeField* pTrade) {
  boost::variant<CThostFtdcOrderField, CThostFtdcTradeField> v(*pTrade);
  oa_ << NanoTimer::getInstance()->getNano();
  boost::serialization::save(oa_, v, 0);
}

caf::behavior CtpOrderRecorder::make_behavior() {
  return {[=](CtpDisconnectAtom) { api_->Release(); }};
}

void CtpOrderRecorder::OnRspUserLogin(
    CThostFtdcRspUserLoginField* pRspUserLogin,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  if (pRspInfo != NULL && pRspInfo->ErrorID == 0) {
    caf::aout(this) << parseNano(NanoTimer::getInstance()->getNano(),
                                 "%Y%m%d %H:%M:%S")
                    << "] Logon:" << pRspInfo->ErrorMsg << "\n";
  } else {
    caf::aout(this) << parseNano(NanoTimer::getInstance()->getNano(),
                                 "%Y%m%d %H:%M:%S")
                    << "] Logon:" << pRspInfo->ErrorMsg << "\n";
  }
}

void CtpOrderRecorder::OnFrontConnected() {
  caf::aout(this) << parseNano(NanoTimer::getInstance()->getNano(),
                               "%Y%m%d %H:%M:%S")
                  << "] OnFrontConnected\n";
  CThostFtdcReqUserLoginField field{0};
  strcpy(field.UserID, user_id_.c_str());
  strcpy(field.Password, password_.c_str());
  strcpy(field.BrokerID, broker_id_.c_str());
  api_->ReqUserLogin(&field, 0);
}

void CtpOrderRecorder::OnFrontDisconnected(int nReason) {
  caf::aout(this) << parseNano(NanoTimer::getInstance()->getNano(),
                               "%Y%m%d %H:%M:%S")
                  << "] OnFrontDisconnected:" << nReason << "\n";
}

CtpOrderRecorder::~CtpOrderRecorder() {}

CtpOrderRecorder::CtpOrderRecorder(caf::actor_config& cfg,
                                   std::string server,
                                   std::string broker_id,
                                   std::string user_id,
                                   std::string password)
    : event_based_actor(cfg),
      file_(MakeDateBinaryFileName(user_id),
            std::ios_base::binary | std::ios_base::trunc),
      oa_(file_),
      broker_id_(std::move(broker_id)),
      user_id_(std::move(user_id)),
      password_(std::move(password)) {
  boost::filesystem::path dir{boost::filesystem::current_path()};
  dir /= user_id_;
  api_ = CThostFtdcTraderApi::CreateFtdcTraderApi(
      (dir.string() + "\\").c_str());
  api_->RegisterSpi(this);

  char fron_server[255] = {0};
  strcpy(fron_server, server.c_str());
  api_->RegisterFront(fron_server);
  api_->SubscribePublicTopic(THOST_TERT_RESUME);
  api_->SubscribePrivateTopic(THOST_TERT_RESUME);
  api_->Init();
}

void CtpOrderRecorder::OnRtnInstrumentStatus(
    CThostFtdcInstrumentStatusField* pInstrumentStatus) {
  if (pInstrumentStatus->InstrumentStatus == THOST_FTDC_IS_Closed) {
    file_.flush();
  }
}
