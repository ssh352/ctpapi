#ifndef LIVE_TRADE_LIVE_TRADE_DATA_FEED_HANDLER_H
#define LIVE_TRADE_LIVE_TRADE_DATA_FEED_HANDLER_H
#include <boost/date_time/posix_time/posix_time.hpp>
#include "caf/all.hpp"
#include "ctpapi/ThostFtdcMdApi.h"
#include "hpt_core/time_util.h"
#include "live_trade_mail_box.h"
#include "common/api_struct.h"
#include "caf_common/caf_atom_defines.h"
#include "live_trade_system.h"

class LiveTradeDataFeedHandler : public caf::event_based_actor,
                                 public CThostFtdcMdSpi {
 public:
  LiveTradeDataFeedHandler(caf::actor_config& cfg, LiveTradeSystem* live_trade_system);

  void HandleCTARtnOrderSignal(const std::shared_ptr<OrderField>& rtn_order,
                               const CTAPositionQty& position_qty);

  void Connect(const std::string& server,
               std::string broker_id,
               std::string user_id,
               std::string password);

  virtual void OnFrontConnected() override;

  virtual void OnFrontDisconnected(int nReason) override;

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override;

  virtual void OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                               CThostFtdcRspInfoField* pRspInfo,
                               int nRequestID,
                               bool bIsLast) override;

  virtual void OnRtnDepthMarketData(
      CThostFtdcDepthMarketDataField* pDepthMarketData) override;

  virtual void OnRspError(CThostFtdcRspInfoField* pRspInfo,
                          int nRequestID,
                          bool bIsLast) override;

  virtual caf::behavior make_behavior() override;

 private:
  LiveTradeMailBox* mail_box_;

  CThostFtdcMdApi* api_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  std::set<std::string> instruments_;
  LiveTradeSystem* live_trade_system_;
};

#endif  // LIVE_TRADE_LIVE_TRADE_DATA_FEED_HANDLER_H
