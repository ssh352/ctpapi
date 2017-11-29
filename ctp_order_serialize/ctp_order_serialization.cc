#include "ctp_order_serialization.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/variant.hpp>
#include "thost_field_fusion_adapt.h"


void CtpOrderSerialization::OnRtnOrder(CThostFtdcOrderField* pOrder) {
  boost::variant<CThostFtdcOrderField, CThostFtdcTradeField> v(*pOrder);
  boost::serialization::save(oa_, v, 0);
  std::cout << "fuck\n";
}

void CtpOrderSerialization::OnRtnTrade(CThostFtdcTradeField* pTrade) {
  boost::variant<CThostFtdcOrderField, CThostFtdcTradeField> v(*pTrade);
  boost::serialization::save(oa_, v, 0);
  std::cout << "fuck\n";
}

caf::behavior CtpOrderSerialization::make_behavior() {
  return {};
}

void CtpOrderSerialization::OnRspUserLogin(
    CThostFtdcRspUserLoginField* pRspUserLogin,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  if (pRspInfo != NULL && pRspInfo->ErrorID == 0) {
  }
}

void CtpOrderSerialization::OnFrontConnected() {
  CThostFtdcReqUserLoginField field{0};
  strcpy(field.UserID, user_id_.c_str());
  strcpy(field.Password, password_.c_str());
  strcpy(field.BrokerID, broker_id_.c_str());
  api_->ReqUserLogin(&field, 0);
}

CtpOrderSerialization::~CtpOrderSerialization() {}

CtpOrderSerialization::CtpOrderSerialization(caf::actor_config& cfg,
                                             std::string server,
                                             std::string broker_id,
                                             std::string user_id,
                                             std::string password)
    : event_based_actor(cfg),
      file_("test.bin", std::ios_base::binary),
      oa_(file_),
      broker_id_(std::move(broker_id)),
      user_id_(std::move(user_id)),
      password_(std::move(password)) {
  api_ = CThostFtdcTraderApi::CreateFtdcTraderApi();
  api_->RegisterSpi(this);

  char fron_server[255] = {0};
  strcpy(fron_server, server.c_str());
  api_->RegisterFront(fron_server);
  api_->SubscribePublicTopic(THOST_TERT_RESTART);
  api_->SubscribePrivateTopic(THOST_TERT_RESTART);
  api_->Init();
  // sqlite3_close(db_);
}
