#ifndef FOLLOW_TRADE_SERVER_CTP_UTIL_H
#define FOLLOW_TRADE_SERVER_CTP_UTIL_H
#include "common/api_struct.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"

CThostFtdcInputOrderField MakeCtpOpenOrder(const std::string& instrument,
                                           const std::string& order_id,
                                           OrderDirection direction,
                                           double price,
                                           int quantity);

CThostFtdcInputOrderField MakeCtpCloseOrder(const std::string& instrument,
                                            const std::string& order_id,
                                            OrderDirection direction,
                                            PositionEffect position_effect,
                                            double price,
                                            int quantity);

CThostFtdcInputOrderActionField MakeCtpCancelOrderAction(
    int front_id,
    int session_id,
    const std::string& order_id );
OrderData MakeOrderData(const CThostFtdcOrderField& order);

#endif  // FOLLOW_TRADE_SERVER_CTP_UTIL_H
