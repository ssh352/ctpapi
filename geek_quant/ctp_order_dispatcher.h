#ifndef FOLLOW_TRADE_CTP_ORDER_DISPATCHER_H
#define FOLLOW_TRADE_CTP_ORDER_DISPATCHER_H
#include <boost/optional.hpp>
#include "geek_quant/caf_defines.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"

class CtpOrderDispatcher {
 public:
  CtpOrderDispatcher(bool is_cta);
  boost::optional<RtnOrderData> HandleRtnOrder(CThostFtdcOrderField pOrder);

 private:
  bool IsSameOrderStatus(const CThostFtdcOrderField& left,
                         const CThostFtdcOrderField& right);
  OldOrderStatus ParseThostForOrderStatus(const CThostFtdcOrderField& order);
  RequestBy ParseRequestBy(const std::string& user_product_info) const;

  std::vector<CThostFtdcOrderField> orders_;
  bool is_cta_;
};

#endif  // FOLLOW_TRADE_CTP_ORDER_DISPATCHER_H
