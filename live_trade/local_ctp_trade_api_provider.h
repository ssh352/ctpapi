#ifndef LIVE_TRADE_LOCAL_CTP_TRADE_API_PROVIDER_H
#define LIVE_TRADE_LOCAL_CTP_TRADE_API_PROVIDER_H
#include "ctp_trader_api.h"
#include "ctp_trade_api_provider.h"

class LocalCtpTradeApiProvider : public CtpTradeApiProvider,
                                 public CTPTraderApi::Delegate {
 public:
  LocalCtpTradeApiProvider();

  void Connect(const std::string& server,
               const std::string& broker_id,
               const std::string& user_id,
               const std::string& password);

  virtual void Init(CtpTradeApiProvider::Delegate* delegate) override;

  virtual void HandleCtpLogon(int front_id, int session_id) override;

  virtual void HandleCTPRtnOrder(
      const std::shared_ptr<CTPOrderField>& order) override;

  virtual void HandleCTPTradeOrder(const std::string& instrument,
                                   const std::string& order_id,
                                   double trading_price,
                                   int trading_qty,
                                   TimeStamp timestamp) override;

  virtual void HandleRspYesterdayPosition(
      std::vector<OrderPosition> yesterday_positions) override;

  virtual void InputOrder(const CTPEnterOrder& input_order,
                          const std::string& order_id) override;

  virtual void CancelOrder(const CTPCancelOrder& cancel_order) override;

  virtual void RequestYesterdayPosition() override;

  virtual void HandleExchangeStatus(ExchangeStatus exchange_status) override;

 private:
  CtpTradeApiProvider::Delegate* delegate_ = NULL;
  std::unique_ptr<CTPTraderApi> trade_api_;
};

#endif  // LIVE_TRADE_LOCAL_CTP_TRADE_API_PROVIDER_H
