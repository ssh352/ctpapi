#ifndef FOLLOW_TRADE_SERVER_CTP_PORTFOLIO_H
#define FOLLOW_TRADE_SERVER_CTP_PORTFOLIO_H
#include "ctp_instrument_position.h"
#include "ctpapi/ThostFtdcUserApiStruct.h"
#include "common/api_struct.h"

class CTPPortfolio {
 public:
  void InitYesterdayPosition(std::vector<OrderPosition> positions);

  void OnRtnOrder(CThostFtdcOrderField order);

 private:
  std::map<std::pair<TThostFtdcSessionIDType, std::string>,
           CThostFtdcOrderField>
      active_orders_;
  std::map<std::pair<std::string, TThostFtdcDirectionType>,
           CTPInstrumentPosition>
      instrument_positions_;
};

#endif  // FOLLOW_TRADE_SERVER_CTP_PORTFOLIO_H
