#ifndef FOLLOW_TRADE_SERVER_CTP_UTIL_H
#define FOLLOW_TRADE_SERVER_CTP_UTIL_H
#include "follow_strategy_mode/defines.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"

CThostFtdcInputOrderField MakeCtpOpenOrder(const std::string& instrument,
                                           const std::string& order_no,
                                           OrderDirection direction,
                                           OrderPriceType price_type,
                                           double price,
                                           int quantity);

CThostFtdcInputOrderField MakeCtpCloseOrder(const std::string& instrument,
                                            const std::string& order_no,
                                            OrderDirection direction,
                                            PositionEffect position_effect,
                                            OrderPriceType price_type,
                                            double price,
                                            int quantity);

CThostFtdcInputOrderActionField MakeCtpCancelOrderAction(
    int front_id,
    int session_id,
    const std::string& order_id,
    const std::string& exchange_id,
    const std::string& order_sys_id);
OrderData MakeOrderData(const CThostFtdcOrderField& order);

#endif  // FOLLOW_TRADE_SERVER_CTP_UTIL_H
