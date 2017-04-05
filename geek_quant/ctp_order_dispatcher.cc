#include "ctp_order_dispatcher.h"

CtpOrderDispatcher::CtpOrderDispatcher(bool is_cta) : is_cta_(is_cta) {}

boost::optional<OrderRtnData> CtpOrderDispatcher::HandleRtnOrder(
    CThostFtdcOrderField raw_order) {
  auto it = std::find_if(orders_.begin(), orders_.end(), [&](auto order) {
    return std::string(order.OrderRef) == raw_order.OrderRef &&
           order.SessionID == raw_order.SessionID;
  });
  if (it == orders_.end()) {
    // new order
    OrderRtnData order;
    order.order_direction =
        raw_order.Direction == THOST_FTDC_D_Buy ? kODBuy : kODSell;
    order.instrument = raw_order.InstrumentID;
    order.order_price = raw_order.LimitPrice;
    order.order_status = raw_order.CombOffsetFlag[0] == THOST_FTDC_OF_Open
                             ? kOSOpening
                             : kOSCloseing;
    order.volume = raw_order.VolumeTotal;
    order.order_no = raw_order.OrderRef;
    order.request_by = ParseRequestBy(raw_order.UserProductInfo);
    order.session_id = raw_order.SessionID;
    order.today =
        raw_order.CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday ? true : false;
    orders_.push_back(raw_order);
    return boost::optional<OrderRtnData>(order);
  } else {
    if (!IsSameOrderStatus(*it, raw_order)) {
      OrderRtnData order;
      order.order_direction =
          raw_order.Direction == THOST_FTDC_D_Buy ? kODBuy : kODSell;
      order.instrument = raw_order.InstrumentID;
      order.order_price = raw_order.LimitPrice;
      order.volume = it->VolumeTotal - raw_order.VolumeTotal;
      order.order_status = ParseThostForOrderStatus(raw_order);
      order.request_by = ParseRequestBy(raw_order.UserProductInfo);
      order.today = raw_order.CombOffsetFlag[0] == THOST_FTDC_OF_CloseToday
                        ? true
                        : false;
      order.order_no = raw_order.OrderRef;
      order.session_id = raw_order.SessionID;
      *it = raw_order;
      return boost::optional<OrderRtnData>(order);
    }
  }

  return boost::optional<OrderRtnData>{};
}

bool CtpOrderDispatcher::IsSameOrderStatus(const CThostFtdcOrderField& prev,
                                           const CThostFtdcOrderField& last) {
  OrderStatus prev_order_status = ParseThostForOrderStatus(prev);
  OrderStatus last_order_status = ParseThostForOrderStatus(last);
  if (prev_order_status != last_order_status) {
    return false;
  }
  return prev.VolumeTraded == last.VolumeTraded;
}

OrderStatus CtpOrderDispatcher::ParseThostForOrderStatus(
    const CThostFtdcOrderField& order) {
  OrderStatus order_status = kOSInvalid;
  switch (order.OrderStatus) {
    case THOST_FTDC_OST_AllTraded:
    case THOST_FTDC_OST_PartTradedQueueing:
    case THOST_FTDC_OST_PartTradedNotQueueing:
    case THOST_FTDC_OST_NoTradeQueueing:
    case THOST_FTDC_OST_NoTradeNotQueueing:
    case THOST_FTDC_OST_Unknown:
      if (order.VolumeTraded != 0) {
        order_status = order.CombOffsetFlag[0] == THOST_FTDC_OF_Open
                           ? kOSOpened
                           : kOSClosed;
      } else {
        order_status = order.CombOffsetFlag[0] == THOST_FTDC_OF_Open
                           ? kOSOpening
                           : kOSCloseing;
      }
      break;
    case THOST_FTDC_OST_Canceled: {
      order_status = order.CombOffsetFlag[0] == THOST_FTDC_OF_Open
                         ? kOSOpenCanceled
                         : kOSCloseCanceled;
    }
    case THOST_FTDC_OST_NotTouched:
    case THOST_FTDC_OST_Touched:
    default:
      break;
  }
  return order_status;
}

RequestBy CtpOrderDispatcher::ParseRequestBy(
    const std::string& user_product_info) const {
  RequestBy request_by;
  if (is_cta_) {
    request_by = RequestBy::kCTA;
  } else {
    request_by = user_product_info == kStrategyUserProductInfo
                     ? RequestBy::kStrategy
                     : RequestBy::kApp;
  }
  return request_by;
}
