#include "ctp_order_dispatcher.h"

boost::optional<OrderRtnData> CtpOrderDispatcher::HandleRtnOrder(
    CThostFtdcOrderField raw_order) {
  auto it = std::find_if(orders_.begin(), orders_.end(), [&](auto order) {
    return std::string(order.OrderRef) == raw_order.OrderRef;
  });
  if (it == orders_.end()) {
    // new order
    OrderRtnData order;
    order.order_direction =
        raw_order.Direction == THOST_FTDC_D_Buy ? kODBuy : kODSell;
    order.order_price = raw_order.LimitPrice;
    order.volume = raw_order.VolumeTotal;
    order.order_status = raw_order.CombOffsetFlag[0] == THOST_FTDC_OF_Open
                             ? kOSOpening
                             : kOSCloseing;
    order.order_no = raw_order.OrderRef;
    orders_.push_back(raw_order);
    return boost::optional<OrderRtnData>(order);
  } else {
    if (!IsSameOrderStatus(*it, raw_order)) {
      OrderRtnData order;
      order.order_direction =
          raw_order.Direction == THOST_FTDC_D_Buy ? kODBuy : kODSell;
      order.order_price = raw_order.LimitPrice;
      order.volume = raw_order.VolumeTotal;
      order.order_status = ParseThostForOrderStatus(raw_order);
      order.order_no = raw_order.OrderRef;
      orders_.push_back(raw_order);

      return boost::optional<OrderRtnData>(order);
    }
  }

  return boost::optional<OrderRtnData>{};
}

bool CtpOrderDispatcher::IsSameOrderStatus(const CThostFtdcOrderField& left,
                                           const CThostFtdcOrderField& right) {
  return true;
}

OrderStatus CtpOrderDispatcher::ParseThostForOrderStatus(
    const CThostFtdcOrderField& order) {
  return  order.CombOffsetFlag[0] == THOST_FTDC_OF_Open
                            ? kOSOpened
                            : kOSClosed;
}
