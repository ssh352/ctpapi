#include "unittest_helper.h"

std::shared_ptr<TickData> MakeTick(std::string instrument,
                                   double last_price,
                                   double bid_price,
                                   double ask_price,
                                   int qty,
                                   TimeStamp timestamp) {
  auto tick_data = std::make_shared<TickData>();
  tick_data->instrument = std::make_shared<std::string>(std::move(instrument));
  tick_data->tick = std::make_shared<Tick>();
  tick_data->tick->last_price = last_price;
  tick_data->tick->qty = qty;
  tick_data->tick->ask_price1 = ask_price;
  tick_data->tick->ask_qty1 = 1;
  tick_data->tick->bid_price1 = bid_price;
  tick_data->tick->bid_qty1 = 1;
  tick_data->tick->timestamp = timestamp;
  return std::move(tick_data);
}

std::shared_ptr<TickData> MakeTick(std::string instrument,
                                   double last_price,
                                   int qty,
                                   TimeStamp timestamp) {
  return MakeTick(std::move(instrument), last_price, last_price - 1,
                  last_price + 1, qty, timestamp);
}
