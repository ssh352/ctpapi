#ifndef CAF_DEFINES_H
#define CAF_DEFINES_H

#include "caf/all.hpp"

enum OrderRtnFrom {
  kORFInvalid,
  kORFSource,
  kORFDest,
};

enum OrderDirection {
  kODInvalid,
  kODBuy,
  kODSell,
};

enum EnterOrderAction {
  kEOAInvalid,
  kEOAOpen,
  kEOAClose,
  kEOAOpenConfirm,
  kEOACloseConfirm,
  kEOAOpenReverseOrder,
  kEOAOpenReverseOrderConfirm,
  kEOACancelForTest,
};

enum OrderStatus {
  kOSInvalid,
  kOSOpening,
  kOSCloseing,
  kOSOpened,
  kOSClosed,
  kOSCancel,
};

struct PositionData {
  // PositionData() = delete;
  // PositionData() {
  //   order_direction = kODInvalid;
  //   volume = 0;
  // }
  std::string instrument;
  OrderDirection order_direction;
  double volume;
};

struct OrderRtnData {
  OrderRtnData() {
    order_status = kOSInvalid;
    order_direction = kODInvalid;
    order_price = 0.0;
    volume = 0;
  }
  std::string order_no;
  std::string instrument;
  OrderDirection order_direction;
  OrderStatus order_status;
  double order_price;
  int volume;
};

struct EnterOrderData {
  EnterOrderData() {
    action = kEOAInvalid;
    order_direction = kODInvalid;
    order_price = 0.0;
    old_volume = 0.0;
    volume = 0;
  }
  std::string order_no;
  std::string instrument;
  EnterOrderAction action;
  OrderDirection order_direction;
  double order_price;
  int old_volume;
  int volume;
};

// using TALoginAtom = caf::atom_constant<caf::atom("login")>;
using TAPositionAtom = caf::atom_constant<caf::atom("pos")>;
using TAUnfillOrdersAtom = caf::atom_constant<caf::atom("ufo")>;
using TARtnOrderAtom = caf::atom_constant<caf::atom("ro")>;

using EnterOrderAtom = caf::atom_constant<caf::atom("eo")>;
using CancelOrderAtom = caf::atom_constant<caf::atom("co")>;


using AddStrategySubscriberAtom = caf::atom_constant<caf::atom("addsuber")>;

using TASubscriberActor = caf::typed_actor<
  caf::reacts_to<TAPositionAtom, std::vector<PositionData> >,
    caf::reacts_to<TAUnfillOrdersAtom, std::vector<OrderRtnData> >,
    caf::reacts_to<TARtnOrderAtom, OrderRtnData> >;

using OrderSubscriberActor =
    caf::typed_actor<caf::reacts_to<EnterOrderAtom, EnterOrderData>,
                     caf::reacts_to<CancelOrderAtom, std::string, std::string> >;

using FollowTAStrategyActor = TASubscriberActor::extend<
    caf::reacts_to<AddStrategySubscriberAtom, OrderSubscriberActor> >;

using OrderAgentActor = FollowTAStrategyActor::extend_with<OrderSubscriberActor>;

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, PositionData& x) {
  return f(caf::meta::type_name("OrderPositionData"), x.instrument,
           x.order_direction, x.volume);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, OrderRtnData& x) {
  return f(caf::meta::type_name("OrderRtnData"), x.instrument, x.order_no,
           x.order_status, x.order_direction, x.order_price, x.volume);
}

template <class Inspector>
typename Inspector::result_type inspect(Inspector& f, EnterOrderData& x) {
  return f(caf::meta::type_name("EnterOrderData"), x.instrument, x.order_no,
           x.action, x.order_direction, x.order_price, x.volume);
}

#endif /* CAF_DEFINES_H */
