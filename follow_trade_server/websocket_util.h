#ifndef FOLLOW_TRADE_SERVER_WEBSOCKET_UTIL_H
#define FOLLOW_TRADE_SERVER_WEBSOCKET_UTIL_H
#include "common/api_struct.h"
std::string MakePortfoilioJson(const std::string& master_account_id,
                               std::vector<AccountPortfolio> master_portfolios,
                               const std::string& slave_account_id,
                               std::vector<AccountPortfolio> slave_portfolios);

#endif  // FOLLOW_TRADE_SERVER_WEBSOCKET_UTIL_H
