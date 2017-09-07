#include "unittest_helper.h"

std::shared_ptr<TickData> MakeTick(std::string instrument,
                                   double last_price,
                                   int qty) {
  auto tick_data = std::make_shared<TickData>();
  tick_data->instrument = std::make_shared<std::string>(std::move(instrument));
  tick_data->tick = std::make_shared<Tick>();
  tick_data->tick->last_price = last_price;
  tick_data->tick->qty = qty;
  tick_data->tick->ask_price1 = last_price + 1;
  tick_data->tick->qty = 1;
  tick_data->tick->timestamp = 10000;

  tick_data->tick->bid_price1 = last_price - 1;
  tick_data->tick->bid_qty1 = 1;
  return std::move(tick_data);
}
