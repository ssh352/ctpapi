#ifndef CAF_DEFINES_H
#define CAF_DEFINES_H

#include "caf/all.hpp"

extern const char kStrategyUserProductInfo[];

enum class RequestBy {
  kInvalid,
  kCTA,
  kStrategy,
  kApp,
};
enum OrderRtnFrom {
  kInvalid,
  kORFInvalid,
  kORFSource,
  kORFDest,
};

enum OrderDirection {
  kODUnkown,
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
  kOSOpenCanceled,
  kOSCloseCanceled,
};

enum OpenClose {
  kOCInvalid,
  kOCOpen,
  kOCClose,
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

struct OpenOrderData {
  std::string instrument;
  OrderDirection direction;
  OrderStatus order_status;
};

struct OrderRtnData {
  OrderRtnData() {
    order_status = kOSInvalid;
    order_direction = kODUnkown;
    order_price = 0.0;
    request_by = RequestBy::kInvalid;
    volume = 0;
  }
  std::string order_no;
  std::string instrument;
  OrderDirection order_direction;
  OrderStatus order_status;
  RequestBy request_by;
  double order_price;
  int volume;
};

struct EnterOrderData {
  EnterOrderData() {
    action = kEOAInvalid;
    order_direction = kODUnkown;
    order_price = 0.0;
    volume = 0;
  }
  std::string order_no;
  std::string instrument;
  EnterOrderAction action;
  OrderDirection order_direction;
  double order_price;
  int volume;
};

struct OrderVolume {
  std::string order_no;
  OrderDirection order_direction;
  int opening;
  int position;
  int closeing;
  int closed;
  int canceling;
  int canceled;
};

struct OrderIdent {
  int front_id;
  int session_id;
  std::string order_id;
  std::string exchange_id;
  std::string order_sys_id;
};

struct CloseingActionItem {
  int close_volume;
  int position_volume;
};
struct CloseingActionInfo {
  std::string order_no;
  std::map<std::string, CloseingActionItem> items;
};

struct OpenReverseOrderItem {
  std::string order_no;
  int volume;
  int reverse_volume;
};

struct OpenReverseOrderActionInfo {
  std::string order_no;
  std::vector<OpenReverseOrderItem> items;
};

struct OrderPosition {
  std::string instrument;
  OrderDirection order_direction;
  int volume;
};

// using TALoginAtom = caf::atom_constant<caf::atom("login")>;
using TAPositionAtom = caf::atom_constant<caf::atom("pos")>;
using TAUnfillOrdersAtom = caf::atom_constant<caf::atom("ufo")>;
using TARtnOrderAtom = caf::atom_constant<caf::atom("ro")>;
using TAOrderIdentAtom = caf::atom_constant<caf::atom("ordident")>;
using QryInvestorPositionsAtom = caf::atom_constant<caf::atom("qryposs")>;
using SettlementInfoConfirmAtom = caf::atom_constant<caf::atom("sttlcfm")>;

using TrySyncHistoryOrderAtom = caf::atom_constant<caf::atom("syncord")>;
using OrderRtnForTrader = caf::atom_constant<caf::atom("rotrader")>;
using OrderRtnForFollow = caf::atom_constant<caf::atom("rofollow")>;
using YesterdayPositionForTraderAtom = caf::atom_constant<caf::atom("tyerpos")>;
using YesterdayPositionForFollowerAtom = caf::atom_constant<caf::atom("fyerpos")>;

using TraderRtnOrderAtom = caf::atom_constant<caf::atom("tro")>;
using FollowerRtnOrderAtom = caf::atom_constant<caf::atom("fro")>;

using EnterOrderAtom = caf::atom_constant<caf::atom("eo")>;
using CancelOrderAtom = caf::atom_constant<caf::atom("co")>;

using AddStrategySubscriberAtom = caf::atom_constant<caf::atom("addsuber")>;

using TASubscriberActor = caf::typed_actor<
    caf::reacts_to<TAPositionAtom, std::vector<PositionData> >,
    caf::reacts_to<TAUnfillOrdersAtom, std::vector<OrderRtnData> >,
    caf::reacts_to<TARtnOrderAtom, OrderRtnData> >;

using OrderSubscriberActor = caf::typed_actor<
    caf::reacts_to<EnterOrderAtom, EnterOrderData>,
    caf::reacts_to<CancelOrderAtom, std::string, std::string> >;

using FollowTAStrategyActor = TASubscriberActor::extend<
    caf::reacts_to<AddStrategySubscriberAtom, OrderSubscriberActor> >;

using OrderAgentActor =
    FollowTAStrategyActor::extend_with<OrderSubscriberActor>;

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

#endif /* CAF_DEFINES_H */
