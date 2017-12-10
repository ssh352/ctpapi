#ifndef LIVE_TRADE_CAF_ATOM_DEFINES_H
#define LIVE_TRADE_CAF_ATOM_DEFINES_H
#include "caf/all.hpp"
#include "common/api_struct.h"
#include "bft_core/message.h"

using IdleAtom = caf::atom_constant<caf::atom("idle")>;
using CtpConnectAtom = caf::atom_constant<caf::atom("ctpconnect")>;
using CloseMarketNearAtom = caf::atom_constant<caf::atom("nearclose")>;

using TickContainer = std::vector<std::pair<std::shared_ptr<Tick>, int64_t>>;
using CTASignalContainer =
    std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>;

using CheckHistoryRtnOrderIsDoneAtom =
    caf::atom_constant<caf::atom("checksync")>;

using ReqYesterdayPositionAtom = caf::atom_constant<caf::atom("reqyespos")>;

using RemoteCTPConnectAtom = caf::atom_constant<caf::atom("rectpcon")>;

using SerializeCtaAtom = caf::atom_constant<caf::atom("ser_cta")>;
using SerializeStrategyAtom = caf::atom_constant<caf::atom("ser_stg")>;
using SerializationFlushAtom = caf::atom_constant<caf::atom("seflush")>;


CAF_ALLOW_UNSAFE_MESSAGE_TYPE(TickContainer)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTASignalContainer)
// CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CancelOrder)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(InputOrderSignal)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(InputOrder)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<TickData>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<OrderField>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTAPositionQty)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CancelOrder)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(OrderAction)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<CTPPositionField>)

// CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<OrderPosition>)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<CTPOrderField>)
// CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTPEnterOrder)
// CAF_ALLOW_UNSAFE_MESSAGE_TYPE(CTPCancelOrder)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(bft::Message)


template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, OrderPosition& x) {
  return f(caf::meta::type_name("order_position"), x.instrument,
           x.order_direction, x.quantity);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, CTPEnterOrder& x) {
  return f(caf::meta::type_name("ctp_enter_order"), x.instrument, x.order_id,
           x.position_effect, x.direction, x.price, x.qty, x.timestamp);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, CTPCancelOrder& x) {
  return f(caf::meta::type_name("ctp_cancel_order"), x.front_id, x.session_id,
           x.order_id, x.order_ref, x.order_sys_id, x.exchange_id);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, CTPOrderField& x) {
  return f(caf::meta::type_name("ctp_order"), x.direction,
           x.position_effect_direction, x.position_effect, x.status, x.qty,
           x.leaves_qty, x.trading_qty, x.error_id, x.raw_error_id, x.front_id,
           x.session_id, x.input_price, x.trading_price, x.avg_price,
           x.input_timestamp, x.update_timestamp, x.instrument, x.exchange_id,
           x.date, x.order_id, x.order_ref, x.order_sys_id,
           x.raw_error_message);
}

#endif  // LIVE_TRADE_CAF_ATOM_DEFINES_H
