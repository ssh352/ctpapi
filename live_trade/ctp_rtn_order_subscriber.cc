#include "ctp_rtn_order_subscriber.h"

CAFCTAOrderSignalBroker::CAFCTAOrderSignalBroker(caf::actor_config& cfg,
                                                 LiveTradeMailBox* mail_box)
    : caf::event_based_actor(cfg),
      trade_api_(this, ".\\cta\\"),
      mail_box_(mail_box),
      signal_subscriber_(this) {
  // TODO:tempare hard code
  // signal_subscriber_.AddPosition("MA801", OrderDirection::kBuy, 9);
  // signal_subscriber_.AddPosition("c1801", OrderDirection::kBuy, 12);
  // signal_subscriber_.AddPosition("cs1801", OrderDirection::kBuy, 20);
  // signal_subscriber_.AddPosition("cu1801", OrderDirection::kSell, 1);
  // signal_subscriber_.AddPosition("l1801", OrderDirection::kBuy, 2);
  // signal_subscriber_.AddPosition("p1805", OrderDirection::kBuy, 2);
  // signal_subscriber_.AddPosition("p1801", OrderDirection::kSell, 4);
  // signal_subscriber_.AddPosition("pp1801", OrderDirection::kBuy, 6);
}

caf::behavior CAFCTAOrderSignalBroker::make_behavior() {
  set_default_handler(caf::skip);
  caf::behavior work_behavior = {
      [=](const std::shared_ptr<CTPOrderField>& ctp_order) {
        auto& it = ctp_orders_.find(ctp_order->order_id, HashCTPOrderField(),
                                    CompareCTPOrderField());
        if (it == ctp_orders_.end()) {
          signal_subscriber_.HandleSyncHistoryRtnOrder(
              CTASignalAtom::value, MakeOrderField(ctp_order, 0.0, 0));
          ctp_orders_.insert(ctp_order);
        } else if (ctp_order->status == OrderStatus::kCanceled) {
          signal_subscriber_.HandleSyncHistoryRtnOrder(
              CTASignalAtom::value, MakeOrderField(ctp_order, 0.0, 0));
          ctp_orders_.erase(it);
          ctp_orders_.insert(ctp_order);
        } else {
        }
      },
      [=](const std::string& instrument, const std::string& order_id,
          double trading_price, int trading_qty, TimeStamp timestamp) {
        BOOST_ASSERT(ctp_orders_.find(order_id, HashCTPOrderField(),
                                      CompareCTPOrderField()) !=
                     ctp_orders_.end());
        auto it = ctp_orders_.find(order_id, HashCTPOrderField(),
                                   CompareCTPOrderField());
        if (it != ctp_orders_.end()) {
          (*it)->leaves_qty -= trading_qty;
          signal_subscriber_.HandleSyncHistoryRtnOrder(
              CTASignalAtom::value,
              MakeOrderField(*it, trading_price, trading_qty, timestamp));
        }
      },

  };
  caf::behavior sync_order_behaivor = {
      [=](const std::shared_ptr<CTPOrderField>& ctp_order) {
        auto& it = ctp_orders_.find(ctp_order->order_id, HashCTPOrderField(),
                                    CompareCTPOrderField());
        if (it == ctp_orders_.end()) {
          signal_subscriber_.HandleSyncHistoryRtnOrder(
              CTASignalAtom::value, MakeOrderField(ctp_order, 0.0, 0));
          ctp_orders_.insert(ctp_order);
        } else if (ctp_order->status == OrderStatus::kCanceled) {
          signal_subscriber_.HandleSyncHistoryRtnOrder(
              CTASignalAtom::value, MakeOrderField(ctp_order, 0.0, 0));
          ctp_orders_.erase(it);
          ctp_orders_.insert(ctp_order);
        } else {
        }
        ++sync_rtn_order_count_;
      },
      [=](const std::string& instrument, const std::string& order_id,
          double trading_price, int trading_qty, TimeStamp timestamp) {
        ++sync_rtn_order_count_;
        BOOST_ASSERT(ctp_orders_.find(order_id, HashCTPOrderField(),
                                      CompareCTPOrderField()) !=
                     ctp_orders_.end());
        auto it = ctp_orders_.find(order_id, HashCTPOrderField(),
                                   CompareCTPOrderField());
        if (it != ctp_orders_.end()) {
          (*it)->leaves_qty -= trading_qty;
          signal_subscriber_.HandleSyncHistoryRtnOrder(
              CTASignalAtom::value,
              MakeOrderField(*it, trading_price, trading_qty, timestamp));
        }
      },
      [=](CheckHistoryRtnOrderIsDoneAtom, size_t last_check_size) {
        if (last_check_size != sync_rtn_order_count_) {
          // maybe still on receive
          delayed_send(this, std::chrono::milliseconds(500),
                       CheckHistoryRtnOrderIsDoneAtom::value,
                       sync_rtn_order_count_);
        } else {
          // auto& lg = g_logger::get();
          // BOOST_LOG(lg) << "CTAwRtnOrder:" << sequence_orders_.size();
          caf::aout(this) << "";
          become(work_behavior);
          set_default_handler(caf::print_and_drop);
        }
      },
  };

  return {
      [=](CtpConnectAtom, const std::string& server,
          const std::string& broker_id, const std::string& user_id,
          const std::string& password) {
        trade_api_.Connect(server, broker_id, user_id, password);
      },
      [=](const std::vector<OrderPosition>& quantitys) {
        signal_subscriber_.HandleSyncYesterdayPosition(CTASignalAtom::value,
                                                       quantitys);
        delayed_send(this, std::chrono::milliseconds(500),
                     CheckHistoryRtnOrderIsDoneAtom::value,
                     0);
        become(sync_order_behaivor);
      },
  };
}

void CAFCTAOrderSignalBroker::HandleCTPRtnOrder(
    const std::shared_ptr<CTPOrderField>& ctp_order) {
  send(this, ctp_order);
}

void CAFCTAOrderSignalBroker::Connect(const std::string& server,
                                      const std::string& broker_id,
                                      const std::string& user_id,
                                      const std::string& password) {
  trade_api_.Connect(server, broker_id, user_id, password);
}

std::shared_ptr<OrderField> CAFCTAOrderSignalBroker::MakeOrderField(
    const std::shared_ptr<CTPOrderField>& ctp_order,
    double trading_price,
    int trading_qty,
    TimeStamp timestamp) const {
  auto order = std::make_shared<OrderField>();
  order->direction = ctp_order->direction;
  order->position_effect_direction = ctp_order->position_effect_direction;
  order->position_effect =
      ctp_order->position_effect == CTPPositionEffect::kOpen
          ? PositionEffect::kOpen
          : PositionEffect::kClose;
  order->status = ctp_order->status;
  order->qty = ctp_order->qty;
  order->leaves_qty = ctp_order->leaves_qty;
  order->error_id = ctp_order->error_id;
  order->raw_error_id = ctp_order->raw_error_id;
  order->input_price = ctp_order->input_price;
  order->avg_price = ctp_order->avg_price;
  order->input_timestamp = ctp_order->input_timestamp;
  if (timestamp != 0) {
    order->update_timestamp = timestamp;
  } else {
    order->update_timestamp = ctp_order->update_timestamp;
  }
  order->instrument_id = ctp_order->instrument;
  order->exchange_id = ctp_order->exchange_id;
  order->date = ctp_order->date;
  order->order_id = ctp_order->order_id;
  order->raw_error_message = ctp_order->raw_error_message;
  order->trading_qty = trading_qty;
  order->trading_price = trading_price;
  return order;
}

void CAFCTAOrderSignalBroker::HandleCTPTradeOrder(const std::string& instrument,
                                                  const std::string& order_id,
                                                  double trading_price,
                                                  int trading_qty,
                                                  TimeStamp timestamp) {
  send(this, instrument, order_id, trading_price, trading_qty, timestamp);
}

void CAFCTAOrderSignalBroker::HandleLogon() {
  trade_api_.RequestYesterdayPosition();
}

void CAFCTAOrderSignalBroker::HandleRspYesterdayPosition(
    std::vector<OrderPosition> yesterday_positions) {
  send(this, std::move(yesterday_positions));
}
