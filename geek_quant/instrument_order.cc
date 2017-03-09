#include "instrument_order.h"

InstrumentOrder::InstrumentOrder(OrderDelegate* delegate,
                                 const OrderRtnData& order)
    : delegate_(delegate) {
  memset(&source_order_, 0, sizeof(InnerOrderData));
  memset(&dest_order_, 0, sizeof(InnerOrderData));
}


InstrumentOrder::~InstrumentOrder() {}

void InstrumentOrder::HandleOrderRtnData(OrderRtnFrom from,
                                         const OrderRtnData& order) {}
