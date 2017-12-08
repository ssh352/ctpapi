#include "local_ctp_trade_api_provider.h"
#include "common_util.h"

LocalCtpTradeApiProvider::LocalCtpTradeApiProvider() {
  ClearUpCTPFolwDirectory(".\\follow_account\\");
  trade_api_ = std::make_unique<CTPTraderApi>(this, ".\\follow_account\\");
}

void LocalCtpTradeApiProvider::Init(CtpTradeApiProvider::Delegate* delegate) {
  delegate_ = delegate;
}

void LocalCtpTradeApiProvider::HandleRspYesterdayPosition(
    std::vector<OrderPosition> yesterday_positions) {
  delegate_->HandleRspYesterdayPosition(std::move(yesterday_positions));
}

void LocalCtpTradeApiProvider::HandleCTPTradeOrder(
    const std::string& instrument,
    const std::string& order_id,
    double trading_price,
    int trading_qty,
    TimeStamp timestamp) {
  delegate_->HandleCTPTradeOrder(instrument, order_id, trading_price,
                                 trading_qty, timestamp);
}

void LocalCtpTradeApiProvider::HandleCTPRtnOrder(
    const std::shared_ptr<CTPOrderField>& order) {
  delegate_->HandleCTPRtnOrder(order);
}

void LocalCtpTradeApiProvider::HandleCtpLogon(int front_id, int session_id) {
  delegate_->HandleCtpLogon(front_id, session_id);
}

void LocalCtpTradeApiProvider::Connect(const std::string& server,
                                       const std::string& broker_id,
                                       const std::string& user_id,
                                       const std::string& password) {
  trade_api_->Connect(server, broker_id, user_id, password);
}

void LocalCtpTradeApiProvider::InputOrder(const CTPEnterOrder& input_order,
                                          const std::string& ctp_order_ref) {
  trade_api_->InputOrder(input_order, ctp_order_ref);
}

void LocalCtpTradeApiProvider::CancelOrder(const CTPCancelOrder& cancel_order) {
  trade_api_->CancelOrder(cancel_order);
}

void LocalCtpTradeApiProvider::RequestYesterdayPosition() {
  trade_api_->RequestYesterdayPosition();
}

void LocalCtpTradeApiProvider::HandleExchangeStatus(
    ExchangeStatus exchange_status) {
}
