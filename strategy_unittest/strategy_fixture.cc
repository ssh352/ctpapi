#include "strategy_fixture.h"
#include <boost/assert.hpp>

#include "unittest_helper.h"

std::shared_ptr<OrderField> StrategyFixture::MakeCanceledOrder(
    const std::string& account_id,
    const std::string& order_id) {
  BOOST_ASSERT(order_containter_.find(account_id + ":" + order_id) !=
               order_containter_.end());
  auto order = std::make_shared<OrderField>(*order_containter_.at(order_id));
  order->status = OrderStatus::kCanceled;
  return std::move(order);
}

std::shared_ptr<OrderField> StrategyFixture::MakeNewCloseOrder(
    const std::string& account_id,
    const std::string& order_id,
    const std::string& instrument,
    OrderDirection direction,
    double price,
    double qty,
    TimeStamp timestamp,
    PositionEffect position_effect /*= PositionEffect::kClose*/) {
  return MakeNewOrder(account_id, order_id, instrument, position_effect,
                      direction, price, qty, timestamp);
}

std::shared_ptr<OrderField> StrategyFixture::MakeNewOpenOrder(
    const std::string& account_id,
    const std::string& order_id,
    const std::string& instrument,
    OrderDirection direction,
    double price,
    double qty,
    TimeStamp timestamp) {
  return MakeNewOrder(account_id, order_id, instrument, PositionEffect::kOpen,
                      direction, price, qty, timestamp);
}

std::shared_ptr<OrderField> StrategyFixture::MakeTradedOrder(
    const std::string& account_id,
    const std::string& order_id,
    double traded_qty,
    TimeStamp timestamp) {
  auto key = account_id + ":" + order_id;
  BOOST_ASSERT(order_containter_.find(key) != order_containter_.end());
  auto order = std::make_shared<OrderField>(*order_containter_.at(key));
  order->status =
      traded_qty == order->qty ? OrderStatus::kAllFilled : OrderStatus::kActive;
  order->trading_price = order->input_price;
  order->trading_qty = traded_qty;
  order->leaves_qty = order->qty - traded_qty;
  order->update_timestamp = timestamp;
  order_containter_[key] = order;
  return std::move(order);
}

std::shared_ptr<OrderField> StrategyFixture::MakeTradedOrder(
    const std::string& account_id,
    const std::string& order_id,
    double traded_qty,
    double trading_price,
    TimeStamp timestamp) {
  auto key = account_id + ":" + order_id;
  BOOST_ASSERT(order_containter_.find(key) != order_containter_.end());
  auto order = std::make_shared<OrderField>(*order_containter_.at(key));
  order->status =
      traded_qty == order->qty ? OrderStatus::kAllFilled : OrderStatus::kActive;
  order->trading_price = trading_price;
  order->trading_qty = traded_qty;
  order->leaves_qty = order->qty - traded_qty;
  order->update_timestamp = timestamp;
  order_containter_[key] = order;
  return std::move(order);
}

std::shared_ptr<OrderField> StrategyFixture::MakeNewOrder(
    const std::string& account_id,
    const std::string& order_id,
    const std::string& instrument,
    PositionEffect position_effect,
    OrderDirection direction,
    double price,
    double qty,
    TimeStamp timestamp) {
  auto order = MakeOrderField(account_id, order_id, instrument, position_effect,
                              direction, OrderStatus::kActive, price, qty, 0,
                              qty, timestamp, timestamp);
  order_containter_.insert({account_id + ":" + order_id, order});
  return std::move(order);
}

std::shared_ptr<OrderField> StrategyFixture::MakeOrderField(
    const std::string& account_id,
    const std::string& order_id,
    const std::string& instrument,
    PositionEffect position_effect,
    OrderDirection direction,
    OrderStatus status,
    double price,
    double leaves_qty,
    double traded_qty,
    double qty,
    TimeStamp input_timestamp,
    TimeStamp update_timestamp) {
  auto order = std::make_shared<OrderField>();
  order->strategy_id = account_id;
  order->order_id = order_id;
  order->position_effect = position_effect;
  order->direction = direction;
  order->status = status;
  order->instrument_id = instrument;
  order->input_price = price;
  order->leaves_qty = leaves_qty;
  order->trading_qty = traded_qty;
  order->qty = qty;
  order->input_timestamp = input_timestamp;
  order->update_timestamp = update_timestamp;
  return std::move(order);
}

void StrategyFixture::OpenAndFillOrder(const std::string& master_account_id,
                                       const std::string& slave_account_id,
                                       OrderDirection order_direction,
                                       int qty,
                                       int trading_qty,
                                       int slave_trading_qty,
                                       int delayed_open_order_after_seconds) {
  Send(CTASignalAtom::value, MakeNewOpenOrder(master_account_id, "0", "I1",
                                              order_direction, 88.8, qty, 0));
  Send(CTASignalAtom::value,
       MakeTradedOrder(master_account_id, "0", trading_qty, 0));

  Send(MakeTick("I1", 88.8, 10, delayed_open_order_after_seconds * 1000));

  Send(MakeNewOpenOrder(slave_account_id, "0", "I1", order_direction, 88.8, qty,
                        0));
  Send(MakeTradedOrder(slave_account_id, "0", slave_trading_qty, 0));

  Clear();
}
