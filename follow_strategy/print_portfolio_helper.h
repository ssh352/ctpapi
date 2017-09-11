#ifndef FOLLOW_TRADE_SERVER_PRINT_PORTFOLIO_HELPER_H
#define FOLLOW_TRADE_SERVER_PRINT_PORTFOLIO_HELPER_H
#include "common/api_struct.h"

std::string FormatPortfolio(std::string account_id,
                            std::vector<AccountPortfolio> master_portfolio,
                            std::vector<AccountPortfolio> slave_portfolio,
                            bool fully);

#endif  // FOLLOW_TRADE_SERVER_PRINT_PORTFOLIO_HELPER_H
