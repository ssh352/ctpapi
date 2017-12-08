#include "serialization_rtn_order.h"
#include "caf_common/caf_atom_defines.h"
#include "common/serialization_util.h"
#include "common_util.h"
SerializationCtaRtnOrder::SerializationCtaRtnOrder(caf::actor_config& cfg,
                                                   LiveTradeMailBox* mail_box)
    : event_based_actor(cfg),
      mail_box_(mail_box),
      file_(MakeFileNameWithDateTimeSubfix("daily_serialization", "cta", "bin"),
            std::ios_base::binary),
      oa_(file_) {
  mail_box_->Subscribe(
      typeid(std::tuple<std::shared_ptr<OrderField>, CTAPositionQty>), this);
  mail_box_->Subscribe(typeid(std::tuple<SerializationFlushAtom>), this);
}

caf::behavior SerializationCtaRtnOrder::make_behavior() {
  return {
      [=](const std::shared_ptr<OrderField>& rtn_order,
          const CTAPositionQty& position_qty) { oa_ << *rtn_order; },
      [=](SerializationFlushAtom) { file_.flush(); },
  };
}

SerializationStrategyRtnOrder::SerializationStrategyRtnOrder(
    caf::actor_config& cfg,
    LiveTradeMailBox* mail_box,
    std::string account_id)
    : event_based_actor(cfg),
      mail_box_(mail_box),
      account_id_(std::move(account_id)),
      file_(MakeFileNameWithDateTimeSubfix("daily_serialization",
                                           account_id_,
                                           "bin"),
            std::ios_base::binary),
      oa_(file_) {
  mail_box_->Subscribe(typeid(std::tuple<std::shared_ptr<OrderField> >), this);
  mail_box_->Subscribe(typeid(std::tuple<SerializationFlushAtom>), this);
}

caf::behavior SerializationStrategyRtnOrder::make_behavior() {
  return {
      [=](const std::shared_ptr<OrderField>& order) { oa_ << *order; },
      [=](SerializationFlushAtom) { file_.flush(); },
  };
}
