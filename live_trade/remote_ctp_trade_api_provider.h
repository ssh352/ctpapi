#ifndef LIVE_TRADE_REMOTE_CTP_TRADE_API_PROVIDER_H
#define LIVE_TRADE_REMOTE_CTP_TRADE_API_PROVIDER_H
#include "caf/all.hpp"
#include "ctp_trade_api_provider.h"

class RemoteCtpApiTradeApiProvider : public CtpTradeApiProvider {
 public:
  void SetRemoteHandler(caf::actor actor);

  void SetRemoteTradeApi(caf::actor actor);

  virtual void Init(Delegate* delegate) override;

  virtual void InputOrder(const CTPEnterOrder& input_order,
                          const std::string& order_id) override;

  virtual void CancelOrder(const CTPCancelOrder& cancel_order) override;

  virtual void RequestYesterdayPosition() override;

  void HandleCTPRtnOrder(const std::shared_ptr<CTPOrderField>& order);

  void HandleCTPTradeOrder(const std::string& instrument,
                           const std::string& order_id,
                           double trading_price,
                           int trading_qty,
                           TimeStamp timestamp);

  void HandleRspYesterdayPosition(
      std::vector<OrderPosition> yesterday_positions);

 private:
  Delegate* delegate_;
  caf::actor handler_;
  caf::actor remote_trade_api_;
};

#endif  // LIVE_TRADE_REMOTE_CTP_TRADE_API_PROVIDER_H
