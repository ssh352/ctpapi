#include "ctp_instrument_broker_test.h"
#include <boost/lexical_cast.hpp>
#include "common/api_data_type.h"
#include "hpt_core/order_util.h"

const static std::string default_instrument = "I1";
const static std::string account_id = "A1";

std::string CTPInstrumentBrokerTest::GenerateOrderId() {
  return boost::lexical_cast<std::string>(order_seq_++);
}

void CTPInstrumentBrokerTest::SimulateCTPTradedOrderFieldWithPrice(
    const std::string& order_id,
    double trading_price,
    int qty) {
  auto order =
      orders_.find(order_id, HashCTPOrderField(), CompareCTPOrderField());
  //(*order)->trading_qty = qty;
  //(*order)->leaves_qty -= qty;
  //(*order)->status = (*order)->leaves_qty == 0 ? OrderStatus::kAllFilled
  //                                             : OrderStatus::kActive;
  //(*order)->trading_price = trading_price;
  // BOOST_ASSERT((*order)->leaves_qty >= 0);
  // broker_.HandleRtnOrder(*order);
  broker_.HandleTraded(order_id, trading_price, qty, 0);
}

void CTPInstrumentBrokerTest::SimulateCTPTradedOrderField(
    const std::string& order_id,
    int qty) {
  auto order =
      orders_.find(order_id, HashCTPOrderField(), CompareCTPOrderField());
  broker_.HandleTraded(order_id, (*order)->input_price, qty, 0);
}

void CTPInstrumentBrokerTest::SimulateCTPNewOpenOrderField(
    const std::string& order_id,
    OrderDirection direction,
    double price,
    int qty) {
  auto order = MakeCTPOrderField(
      order_id, default_instrument, CTPPositionEffect::kOpen, direction,
      OrderStatus::kActive, price, qty, 0, qty, 0, 0);
  broker_.HandleRtnOrder(order);
  orders_.emplace(std::move(order));
}

void CTPInstrumentBrokerTest::MakeCloseOrderRequest(const std::string& order_id,
                                                    OrderDirection direction,
                                                    double price,
                                                    int qty) {
  broker_.HandleInputOrder(InputOrder{default_instrument, order_id,
                                      PositionEffect::kClose, direction, price,
                                      10, 0});
}

void CTPInstrumentBrokerTest::MakeOpenOrderRequest(const std::string& order_id,
                                                   OrderDirection direction,
                                                   double price,
                                                   int qty) {
  broker_.HandleInputOrder(InputOrder{default_instrument, order_id,
                                      PositionEffect::kOpen, direction, price,
                                      10, 0});
}

std::shared_ptr<CTPOrderField> CTPInstrumentBrokerTest::MakeCTPOrderField(
    const std::string& order_id,
    const std::string& instrument,
    CTPPositionEffect position_effect,
    OrderDirection direction,
    OrderStatus status,
    double price,
    double leaves_qty,
    double traded_qty,
    double qty,
    TimeStamp input_timestamp,
    TimeStamp update_timestamp) {
  auto order = std::make_shared<CTPOrderField>();
  order->order_id = order_id;
  order->position_effect = position_effect;
  order->direction = direction;
  order->position_effect_direction = AdjustDirectionByPositionEffect(
      position_effect == CTPPositionEffect::kOpen ? PositionEffect::kOpen
                                                  : PositionEffect::kClose,
      direction);
  order->status = status;
  order->instrument = instrument;
  order->input_price = price;
  order->leaves_qty = leaves_qty;
  order->trading_qty = traded_qty;
  order->qty = qty;
  order->input_timestamp = input_timestamp;
  order->update_timestamp = update_timestamp;
  return std::move(order);
}

void CTPInstrumentBrokerTest::Clear() {
  event_queues_.clear();
}

void CTPInstrumentBrokerTest::ReturnOrderField(
    const std::shared_ptr<OrderField>& order) {
  event_queues_.push_back(order);
}

void CTPInstrumentBrokerTest::CancelOrder(const CTPCancelOrder& cancel_order) {
  event_queues_.push_back(cancel_order);
}

void CTPInstrumentBrokerTest::EnterOrder(const CTPEnterOrder& enter_order) {
  event_queues_.push_back(enter_order);
}

CTPInstrumentBrokerTest::CTPInstrumentBrokerTest()
    : broker_(this,
              default_instrument,
              false,
              std::bind(&CTPInstrumentBrokerTest::GenerateOrderId, this)) {}

void CTPInstrumentBrokerTest::SimulateCTPNewCloseOrderField(
    const std::string& order_id,
    OrderDirection direction,
    double price,
    int qty,
    CTPPositionEffect position_effect /*= CTPPositionEffect::kClose*/) {
  auto order = MakeCTPOrderField(order_id, default_instrument, position_effect,
                                 direction, OrderStatus::kActive, price, qty, 0,
                                 qty, 0, 0);
  broker_.HandleRtnOrder(order);
  orders_.emplace(std::move(order));
}

void CTPInstrumentBrokerTest::MakeCancelOrderRequest(
    const std::string& order_id,
    int qty) {
  broker_.HandleCancel(CancelOrderSignal{order_id, default_instrument, qty});
}

void CTPInstrumentBrokerTest::MakeOrderAction(const std::string& order_id,
                                              int old_qty,
                                              int new_qty) {
  broker_.HandleOrderAction(
      OrderAction{order_id, default_instrument, old_qty, new_qty, 0.0, 0.0});
}

void CTPInstrumentBrokerTest::MakeOrderAction(const std::string& order_id,
                                              double old_price,
                                              double new_price) {
  broker_.HandleOrderAction(
      OrderAction{order_id, default_instrument, 0, 0, old_price, new_price});
}

void CTPInstrumentBrokerTest::MakeOrderAction(const std::string& order_id,
                                              double old_price,
                                              double new_price,
                                              int old_qty,
                                              int new_qty) {
  broker_.HandleOrderAction(OrderAction{order_id, default_instrument, old_qty,
                                        new_qty, old_price, new_price});
}

void CTPInstrumentBrokerTest::SimulateCTPCancelOrderField(
    const std::string& order_id) {
  auto order =
      orders_.find(order_id, HashCTPOrderField(), CompareCTPOrderField());
  auto order_clone = std::make_shared<CTPOrderField>(*(*order));
  order_clone->trading_price = 0;
  order_clone->trading_qty = 0;
  order_clone->status = OrderStatus::kCanceled;
  broker_.HandleRtnOrder(std::move(order_clone));
}

bool CTPInstrumentBrokerTest::CompareCTPOrderField::operator()(
    const std::string& l,
    const std::shared_ptr<CTPOrderField>& r) const {
  return l == r->order_id;
}

bool CTPInstrumentBrokerTest::CompareCTPOrderField::operator()(
    const std::shared_ptr<CTPOrderField>& l,
    const std::shared_ptr<CTPOrderField>& r) const {
  return l->order_id == r->order_id;
}

size_t CTPInstrumentBrokerTest::HashCTPOrderField::operator()(
    const std::string& order_id) const {
  return std::hash<std::string>()(order_id);
}

size_t CTPInstrumentBrokerTest::HashCTPOrderField::operator()(
    const std::shared_ptr<CTPOrderField>& order) const {
  return std::hash<std::string>()(order->order_id);
}
