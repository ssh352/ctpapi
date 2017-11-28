#include "serialization_rtn_order.h"

namespace boost {
namespace serialization {

template <class Archive>
void serialize(Archive& ar, CTPOrderField& order, const unsigned int version) {
  ar& order.direction;
  ar& order.position_effect_direction;
  ar& order.position_effect;
  ar& order.status;
  ar& order.qty;
  ar& order.leaves_qty;
  ar& order.trading_qty;
  ar& order.error_id;
  ar& order.raw_error_id;
  ar& order.front_id;
  ar& order.session_id;
  ar& order.input_price;
  ar& order.trading_price;
  ar& order.avg_price;
  ar& order.input_timestamp;
  ar& order.update_timestamp;
  ar& order.instrument;
  ar& order.exchange_id;
  ar& order.date;
  ar& order.order_id;
  ar& order.order_ref;
  ar& order.order_sys_id;
  ar& order.raw_error_message;
}

template <class Archive>
void serialize(Archive& ar, OrderField& order, const unsigned int version) {
  ar& order.direction;
  ar& order.position_effect_direction;
  ar& order.position_effect;
  ar& order.status;
  ar& order.qty;
  ar& order.leaves_qty;
  ar& order.trading_qty;
  ar& order.error_id;
  ar& order.raw_error_id;
  ar& order.input_price;
  ar& order.trading_price;
  ar& order.avg_price;
  ar& order.input_timestamp;
  ar& order.update_timestamp;
  ar& order.instrument_id;
  ar& order.exchange_id;
  ar& order.date;
  ar& order.order_id;
  ar& order.raw_error_message;
}

}  // namespace serialization
}  // namespace boost

SerializationCtaRtnOrder::SerializationCtaRtnOrder(caf::actor_config& cfg,
                                                   LiveTradeMailBox* mail_box)
    : event_based_actor(cfg),
      mail_box_(mail_box),
      file_("cta.bin", std::ios_base::binary),
      oa_(file_) {
  mail_box_->Subscribe(
      typeid(std::tuple<std::shared_ptr<OrderField>, CTAPositionQty>), this);
}

caf::behavior SerializationCtaRtnOrder::make_behavior() {
  return {[=](const std::shared_ptr<OrderField>& rtn_order,
              const CTAPositionQty& position_qty) { oa_ << *rtn_order; }

  };
}

SerializationStrategyRtnOrder::SerializationStrategyRtnOrder(
    caf::actor_config& cfg,
    LiveTradeMailBox* mail_box,
    std::string account_id)
    : event_based_actor(cfg),
      mail_box_(mail_box),
      account_id_(std::move(account_id)),
      file_(account_id_ + ".bin", std::ios_base::binary),
      oa_(file_) {
  mail_box_->Subscribe(typeid(std::tuple<std::shared_ptr<CTPOrderField> >),
                       this);
}

caf::behavior SerializationStrategyRtnOrder::make_behavior() {
  return {[=](const std::shared_ptr<CTPOrderField>& order) { oa_ << *order; }};
}
