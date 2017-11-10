#include "strategy_fixture.h"
#include <boost/assert.hpp>

#include "unittest_helper.h"
#include "hpt_core/order_util.h"

std::shared_ptr<OrderField> StrategyFixture::MakeCanceledOrder(
    const std::string& account_id,
    const std::string& order_id) {
  auto key = account_id + ":" + order_id;
  BOOST_ASSERT(order_containter_.find(key) != order_containter_.end());
  auto order = std::make_shared<OrderField>(*order_containter_.at(key));
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
    PositionEffect position_effect /*= PositionEffect::kClose*/) {
  return MakeNewOrder(account_id, order_id, instrument, position_effect,
                      direction, price, qty);
}

std::shared_ptr<OrderField> StrategyFixture::MakeNewOpenOrder(
    const std::string& account_id,
    const std::string& order_id,
    const std::string& instrument,
    OrderDirection direction,
    double price,
    double qty) {
  return MakeNewOrder(account_id, order_id, instrument, PositionEffect::kOpen,
                      direction, price, qty);
}

std::shared_ptr<OrderField> StrategyFixture::MakeTradedOrder(
    const std::string& account_id,
    const std::string& order_id,
    double traded_qty) {
  auto key = account_id + ":" + order_id;
  BOOST_ASSERT(order_containter_.find(key) != order_containter_.end());
  auto order = std::make_shared<OrderField>(*order_containter_.at(key));
  order->status = traded_qty == order->leaves_qty ? OrderStatus::kAllFilled
                                                  : OrderStatus::kActive;
  order->trading_price = order->input_price;
  order->trading_qty = traded_qty;
  order->leaves_qty -= traded_qty;
  BOOST_ASSERT(order->leaves_qty >= 0);
  order->update_timestamp = now_timestamp_;
  order_containter_[key] = order;
  return std::move(order);
}

std::shared_ptr<OrderField> StrategyFixture::MakeTradedOrder(
    const std::string& account_id,
    const std::string& order_id,
    double trading_price,
    double traded_qty) {
  auto key = account_id + ":" + order_id;
  BOOST_ASSERT(order_containter_.find(key) != order_containter_.end());
  auto order = std::make_shared<OrderField>(*order_containter_.at(key));
  order->status =
      traded_qty == order->qty ? OrderStatus::kAllFilled : OrderStatus::kActive;
  order->trading_price = trading_price;
  order->trading_qty = traded_qty;
  order->leaves_qty = order->qty - traded_qty;
  order->update_timestamp = now_timestamp_;
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
    double qty) {
  auto order = MakeOrderField(account_id, order_id, instrument, position_effect,
                              direction, OrderStatus::kActive, price, qty, 0,
                              qty, now_timestamp_, now_timestamp_);
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
  order->position_effect_direction =
      AdjustDirectionByPositionEffect(position_effect, direction);
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

void StrategyFixture::HandleInputOrder(const InputOrder& input_order) {
  event_queues_.push_back(input_order);
  auto order =
      MakeNewOrder(account_id_, input_order.order_id, input_order.instrument,
                   input_order.position_effect,
                   AdjustDirectionByPositionEffect(input_order.position_effect,
                                                   input_order.direction),
                   input_order.price, input_order.qty);
  if (auto_reply_new_rtn_order) {
    Send(std::move(order));
  } else {
    pending_reply_new_rtn_orders_.push_back(std::move(order));
  }
}

void StrategyFixture::HandleCancelOrder(const CancelOrderSignal& signal) {
  event_queues_.push_back(signal);
  Send(MakeCanceledOrder(account_id_, signal.order_id));
}

void StrategyFixture::SendAndClearPendingReplyRtnOrder() {
  std::for_each(pending_reply_new_rtn_orders_.begin(),
                pending_reply_new_rtn_orders_.end(),
                [=](const auto& order) { Send(order); });

  pending_reply_new_rtn_orders_.clear();
}

const CTASignalAtom CTASignalAtom::value;

const BeforeTradingAtom BeforeTradingAtom::value;

const BeforeCloseMarketAtom BeforeCloseMarketAtom::value;

const CloseMarketNearAtom CloseMarketNearAtom::value;
