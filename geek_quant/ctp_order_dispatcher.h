#ifndef FOLLOW_TRADE_CTP_ORDER_DISPATCHER_H
#define FOLLOW_TRADE_CTP_ORDER_DISPATCHER_H
#include <boost/optional.hpp>
#include "geek_quant/caf_defines.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"

class CtpOrderDispatcher {
 public:
  boost::optional<OrderRtnData> HandleRtnOrder(CThostFtdcOrderField pOrder);

 private:
  bool IsSameOrderStatus(const CThostFtdcOrderField& left,
                         const CThostFtdcOrderField& right);
  std::vector<CThostFtdcOrderField> orders_;
  OrderStatus ParseThostForOrderStatus(const CThostFtdcOrderField& order);
};

#endif  // FOLLOW_TRADE_CTP_ORDER_DISPATCHER_H
