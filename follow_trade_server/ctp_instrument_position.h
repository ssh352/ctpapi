#ifndef FOLLOW_TRADE_SERVER_CTP_INSTRUMENT_POSITION_H
#define FOLLOW_TRADE_SERVER_CTP_INSTRUMENT_POSITION_H
#include "ctpapi/ThostFtdcUserApiStruct.h"

class CTPInstrumentPosition {
 public:
  void InitYesterdayPosition(int volume);

  void OnRtnOrder(const CThostFtdcOrderField& pre_order,
                  const CThostFtdcOrderField& order);

 private:
  int yesterday_volume_ = 0;
  int total_volume_ = 0;
};

#endif  // FOLLOW_TRADE_SERVER_CTP_INSTRUMENT_POSITION_H
