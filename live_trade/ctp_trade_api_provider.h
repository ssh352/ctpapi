#ifndef LIVE_TRADE_CTP_TRADE_API_PROVIDER_H
#define LIVE_TRADE_CTP_TRADE_API_PROVIDER_H
#include <vector>
#include "common/api_struct.h"

class CtpTradeApiProvider {
 public:
  class Delegate {
   public:
   public:
    virtual void HandleLogon() = 0;
    virtual void HandleCTPRtnOrder(
        const std::shared_ptr<CTPOrderField>& order) = 0;

    virtual void HandleCTPTradeOrder(const std::string& instrument,
                                     const std::string& order_id,
                                     double trading_price,
                                     int trading_qty,
                                     TimeStamp timestamp) = 0;

    virtual void HandleRspYesterdayPosition(
        std::vector<OrderPosition> yesterday_positions) = 0;
  };

  virtual void Init(Delegate* delegate) = 0;

  virtual void Connect(const std::string& server,
                       const std::string& broker_id,
                       const std::string& user_id,
                       const std::string& password) = 0;

  virtual void InputOrder(const CTPEnterOrder& input_order,
                          const std::string& order_id) = 0;

  virtual void CancelOrder(const CTPCancelOrder& cancel_order) = 0;

  virtual void RequestYesterdayPosition() = 0;
};

#endif  // LIVE_TRADE_CTP_TRADE_API_PROVIDER_H
