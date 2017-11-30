#include "ctp_order_serialization.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/variant.hpp>
#include "thost_field_fusion_adapt.h"

using IncreaseFlowCounterAtom = caf::atom_constant<caf::atom("inc")>;
using CheckFlowFeedIsDoneAtom = caf::atom_constant<caf::atom("check")>;

void CtpOrderSerialization::OnRtnOrder(CThostFtdcOrderField* pOrder) {
  boost::variant<CThostFtdcOrderField, CThostFtdcTradeField> v(*pOrder);
  boost::serialization::save(oa_, v, 0);
  send(this, IncreaseFlowCounterAtom::value);
}

void CtpOrderSerialization::OnRtnTrade(CThostFtdcTradeField* pTrade) {
  boost::variant<CThostFtdcOrderField, CThostFtdcTradeField> v(*pTrade);
  boost::serialization::save(oa_, v, 0);
  send(this, IncreaseFlowCounterAtom::value);
}

caf::behavior CtpOrderSerialization::make_behavior() {
  return {[=](RequestTodayFlowAtom) {
            req_flow_promise_ = make_response_promise<void>();
            delayed_send(this, std::chrono::milliseconds(500),
                         CheckFlowFeedIsDoneAtom::value, flow_counter_);
          },
          [=](IncreaseFlowCounterAtom) { ++flow_counter_; },
          [=](CheckFlowFeedIsDoneAtom, int last_counter) {
            if (last_counter == flow_counter_) {
              req_flow_promise_.deliver(caf::error());
            } else {
              delayed_send(this, std::chrono::milliseconds(500),
                           CheckFlowFeedIsDoneAtom::value, flow_counter_);
            }
          }};
}

void CtpOrderSerialization::OnRspUserLogin(
    CThostFtdcRspUserLoginField* pRspUserLogin,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  if (pRspInfo != NULL && pRspInfo->ErrorID == 0) {
    std::cout << "Logon:" << pRspInfo->ErrorMsg << "\n";
  } else {
    std::cout << "Logon:" << pRspInfo->ErrorMsg << "\n";
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
      file_(user_id + ".bin", std::ios_base::binary | std::ios_base::trunc),
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
