#include "ctp_rtn_order_subscriber.h"

CtpRtnOrderSubscriber::CtpRtnOrderSubscriber(caf::actor_config& cfg,
                                             LiveTradeMailBox* mail_box)
    : caf::event_based_actor(cfg),
      trade_api_(this),
      mail_box_(mail_box),
      signal_subscriber_(this, "MA801") {}

caf::behavior CtpRtnOrderSubscriber::make_behavior() {
  return {
      [=](CtpConnectAtom, const std::string& server,
          const std::string& broker_id, const std::string& user_id,
          const std::string& password) {
        trade_api_.Connect(server, broker_id, user_id, password);
      },
      [=](const CTASignalAtom& value,
          const std::vector<OrderPosition>& quantitys) {
        signal_subscriber_.HandleCTASignalInitPosition(value, quantitys);
      },
      [=](const CTASignalAtom& value,
          const std::vector<std::shared_ptr<const OrderField>>& orders) {
        signal_subscriber_.HandleCTASignalHistoryOrder(value, orders);
      },

      [=](CTASignalAtom value,
          const std::shared_ptr<OrderField>& order) {
        signal_subscriber_.HandleCTASignalOrder(value, order);
      },
  };
}

void CtpRtnOrderSubscriber::HandleCTPRtnOrder(
    const std::shared_ptr<CTPOrderField>& ctp_order) {
  auto order = std::make_shared<OrderField>();
  order->direction = ctp_order->direction;
  order->position_effect =
      ctp_order->position_effect == CTPPositionEffect::kOpen
          ? PositionEffect::kOpen
          : PositionEffect::kClose;
  order->status = ctp_order->status;
  order->qty = ctp_order->qty;
  order->leaves_qty = ctp_order->leaves_qty;
  order->trading_qty = ctp_order->trading_qty;
  order->error_id = ctp_order->error_id;
  order->raw_error_id = ctp_order->raw_error_id;
  order->input_price = ctp_order->input_price;
  order->trading_price = ctp_order->trading_price;
  order->avg_price = ctp_order->avg_price;
  order->input_timestamp = ctp_order->input_timestamp;
  order->update_timestamp = ctp_order->update_timestamp;
  order->instrument_id = ctp_order->instrument;
  order->exchange_id = ctp_order->exchange_id;
  order->date = ctp_order->date;
  order->order_id = ctp_order->order_id;
  order->raw_error_message = ctp_order->raw_error_message;
  send(this, CTASignalAtom::value, std::move(order));
}

void CtpRtnOrderSubscriber::Connect(const std::string& server,
                                    const std::string& broker_id,
                                    const std::string& user_id,
                                    const std::string& password) {
  trade_api_.Connect(server, broker_id, user_id, password);
}
