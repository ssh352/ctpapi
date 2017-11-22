#include "remote_ctp_trade_api_provider.h"
#include "caf_common/caf_atom_defines.h"

void RemoteCtpApiTradeApiProvider::RequestYesterdayPosition() {
  caf::send_as(handler_, remote_trade_api_,
               ReqYesterdayPositionAtom::value);
}

void RemoteCtpApiTradeApiProvider::CancelOrder(
    const CTPCancelOrder& cancel_order) {
  caf::send_as(handler_, remote_trade_api_, cancel_order);
}

void RemoteCtpApiTradeApiProvider::InputOrder(const CTPEnterOrder& input_order,
                                              const std::string& order_id) {
  caf::send_as(handler_, remote_trade_api_, input_order, order_id);
}

void RemoteCtpApiTradeApiProvider::Init(Delegate* delegate) {
  delegate_ = delegate;
}

void RemoteCtpApiTradeApiProvider::SetRemoteTradeApi(caf::actor actor) {
  remote_trade_api_ = actor;
}

void RemoteCtpApiTradeApiProvider::HandleCTPRtnOrder(
    const std::shared_ptr<CTPOrderField>& order) {
  delegate_->HandleCTPRtnOrder(order);
}

void RemoteCtpApiTradeApiProvider::HandleCTPTradeOrder(
    const std::string& instrument,
    const std::string& order_id,
    double trading_price,
    int trading_qty,
    TimeStamp timestamp) {
  delegate_->HandleCTPTradeOrder(instrument, order_id, trading_price,
                                 trading_qty, timestamp);
}

void RemoteCtpApiTradeApiProvider::HandleRspYesterdayPosition(
    std::vector<OrderPosition> yesterday_positions) {
  delegate_->HandleRspYesterdayPosition(std::move(yesterday_positions));
}

void RemoteCtpApiTradeApiProvider::SetRemoteHandler(caf::actor actor) {
  handler_ = actor;
}

void RemoteCtpApiTradeApiProvider::HandleCTPLogon(int front_id, int session_id) {
  delegate_->HandleCtpLogon(front_id, session_id);
}
