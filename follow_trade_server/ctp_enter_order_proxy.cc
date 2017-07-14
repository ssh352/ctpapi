#include "ctp_enter_order_proxy.h"

void CTPEnterOrderProxy::OpenOrder(const std::string& instrument,
                                   const std::string& order_no,
                                   OrderDirection direction,
                                   OrderPriceType price_type,
                                   double price,
                                   int quantity) {
  throw std::logic_error("The method or operation is not implemented.");
}

void CTPEnterOrderProxy::CloseOrder(const std::string& instrument,
                                    const std::string& order_no,
                                    OrderDirection direction,
                                    PositionEffect position_effect,
                                    OrderPriceType price_type,
                                    double price,
                                    int quantity) {
  throw std::logic_error("The method or operation is not implemented.");
}

void CTPEnterOrderProxy::CancelOrder(const std::string& order_no) {
  throw std::logic_error("The method or operation is not implemented.");
}
