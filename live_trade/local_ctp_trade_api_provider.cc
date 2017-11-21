#include "local_ctp_trade_api_provider.h"

LocalCtpTradeApiProvider::LocalCtpTradeApiProvider() {
  trade_api_ = std::make_unique<CTPTraderApi>(this, ".\\follow_account\\");
}

void LocalCtpTradeApiProvider::Init(CtpTradeApiProvider::Delegate* delegate) {
  delegate_ = delegate;
}

void LocalCtpTradeApiProvider::HandleRspYesterdayPosition(
    std::vector<OrderPosition> yesterday_positions) {
  throw std::logic_error("The method or operation is not implemented.");
}

void LocalCtpTradeApiProvider::HandleCTPTradeOrder(
    const std::string& instrument,
    const std::string& order_id,
    double trading_price,
    int trading_qty,
    TimeStamp timestamp) {
  throw std::logic_error("The method or operation is not implemented.");
}

void LocalCtpTradeApiProvider::HandleCTPRtnOrder(
    const std::shared_ptr<CTPOrderField>& order) {
  throw std::logic_error("The method or operation is not implemented.");
}

void LocalCtpTradeApiProvider::HandleLogon() {
  throw std::logic_error("The method or operation is not implemented.");
}

void LocalCtpTradeApiProvider::Connect(const std::string& server,
                                       const std::string& broker_id,
                                       const std::string& user_id,
                                       const std::string& password) {
  trade_api_->Connect(server, broker_id, user_id, password);
}

void LocalCtpTradeApiProvider::InputOrder(const CTPEnterOrder& input_order,
                                          const std::string& ctp_order_ref) {
  trade_api_->InputOrder;
}

void LocalCtpTradeApiProvider::CancelOrder(const CTPCancelOrder& cancel_order) {
  throw std::logic_error("The method or operation is not implemented.");
}

void LocalCtpTradeApiProvider::RequestYesterdayPosition() {
  throw std::logic_error("The method or operation is not implemented.");
}
