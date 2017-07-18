#include "ctp_instrument_position.h"

void CTPInstrumentPosition::InitYesterdayPosition(int volume) {
  yesterday_volume_ = volume;
  total_volume_ = volume;
}

void CTPInstrumentPosition::OnRtnOrder(const CThostFtdcOrderField& pre_order,
                                       const CThostFtdcOrderField& order) {
  if (order.CombOffsetFlag[0] == THOST_FTDC_OF_Open) {
    total_volume_ += order.VolumeTraded - pre_order.VolumeTraded;
  } else {
    if (order.CombOffsetFlag[0] != THOST_FTDC_OF_CloseToday) {
      yesterday_volume_ -= std::min(
          yesterday_volume_, order.VolumeTraded - pre_order.VolumeTraded);
    }
    total_volume_ -= order.VolumeTraded - pre_order.VolumeTraded;
  }
}
