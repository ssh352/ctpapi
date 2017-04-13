#ifndef CAF_DEFINES_H
#define CAF_DEFINES_H

#include "caf/all.hpp"

using CTPReqLogin = caf::atom_constant<caf::atom("lg")>;
using CTPRspLogin = caf::atom_constant<caf::atom("rsplg")>;
using CTPReqQryInvestorPositionsAtom = caf::atom_constant<caf::atom("qryipa")>;
using CTPRspQryInvestorPositionsAtom = caf::atom_constant<caf::atom("rspqip")>;
using CTPReqHistoryRtnOrdersAtom = caf::atom_constant<caf::atom("reqrto")>;
using CTPSubscribeRtnOrderAtom = caf::atom_constant<caf::atom("subro")>;
using CTPRtnOrderAtom = caf::atom_constant<caf::atom("rtnord")>;
using CTPReqSettlementInfoConfirm = caf::atom_constant<caf::atom("reqsttcnf")>;
using CTPRspSettlementInfoConfirm = caf::atom_constant<caf::atom("rspsttcnf")>;
using CTPReqOpenOrderAtom = caf::atom_constant<caf::atom("reqoo")>;
using CTPReqCloseOrderAtom = caf::atom_constant<caf::atom("reqco")>;
using CTPCancelOrderAtom = caf::atom_constant<caf::atom("co")>;

// using TALoginAtom = caf::atom_constant<caf::atom("login")>;

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, OrderData& x) {
  return f(caf::meta::type_name("OrderData"), x.account_id_, x.order_id_,
           x.instrument_, x.datetime_, x.user_product_info_, x.order_sys_id_,
           x.exchange_id_, x.quanitty_, x.filled_quantity_, x.session_id_,
           x.price_, x.direction_, x.type_, x.status_, x.position_effect_);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, OrderPosition& x) {
  return f(caf::meta::type_name("OrderPosition"), x.instrument,
           x.order_direction, x.quantity);
}

/*
template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, EnterOrderData& x) {
return f(caf::meta::type_name("EnterOrderData"), x.instrument, x.order_no,
       x.action, x.order_direction, x.order_price, x.volume);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, OrderIdent& x) {
return f(caf::meta::type_name("OrderIdent"), x.order_id, x.front_id,
       x.session_id, x.exchange_id, x.order_sys_id);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, OrderPosition& x) {
return f(caf::meta::type_name("OrderPosition"), x.instrument,
       x.order_direction, x.volume);
}
*/

#endif /* CAF_DEFINES_H */
