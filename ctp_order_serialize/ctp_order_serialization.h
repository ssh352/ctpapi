#ifndef CTP_ORDER_SERIALIZE_CTP_ORDER_SERIALIZATION_H
#define CTP_ORDER_SERIALIZE_CTP_ORDER_SERIALIZATION_H
#include <fstream>
#include <boost/archive/binary_oarchive.hpp>
#include "caf/all.hpp"
#include "ctpapi/ThostFtdcTraderApi.h"

using RequestTodayFlowAtom = caf::atom_constant<caf::atom("todayflow")>;

class CtpOrderSerialization : public caf::event_based_actor,
                              public CThostFtdcTraderSpi {
 public:
  CtpOrderSerialization(caf::actor_config& cfg,
                        std::string server,
                        std::string broker_id,
                        std::string user_id,
                        std::string password);

  ~CtpOrderSerialization();

  virtual void OnFrontConnected() override;

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override;

  virtual caf::behavior make_behavior() override;

  virtual void OnRtnOrder(CThostFtdcOrderField* pOrder) override;

  virtual void OnRtnTrade(CThostFtdcTradeField* pTrade) override;

 private:
  std::ofstream file_;
  CThostFtdcTraderApi* api_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  boost::archive::binary_oarchive oa_;
  int flow_counter_ = 0;
  caf::response_promise req_flow_promise_;
};

#endif  // CTP_ORDER_SERIALIZE_CTP_ORDER_SERIALIZATION_H
