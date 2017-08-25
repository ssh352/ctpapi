#include <memory>
#include <boost/assert.hpp>
#include "gtest/gtest.h"
#include "portfolio.h"

std::unordered_map<std::string, std::shared_ptr<OrderField> >
    g_order_containter;

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

  tick_data->tick->bid_price1 = last_price - 1;
  tick_data->tick->bid_qty1 = 1;
  return std::move(tick_data);
}

std::shared_ptr<OrderField> MakeOrderField(const std::string& order_id,
                                           const std::string& instrument,
                                           PositionEffect position_effect,
                                           OrderDirection direction,
                                           OrderStatus status,
                                           double price,
                                           double leaves_qty,
                                           double traded_qty,
                                           double qty) {
  auto order = std::make_shared<OrderField>();
  order->order_id = order_id;
  order->position_effect = position_effect;
  order->direction = direction;
  order->status = status;
  order->instrument_id = instrument;
  order->price = price;
  order->leaves_qty = leaves_qty;
  order->traded_qty = traded_qty;
  order->qty = qty;
  return std::move(order);
}

auto MakeNewOrder(const std::string& order_id,
                  const std::string& instrument,
                  PositionEffect position_effect,
                  OrderDirection direction,
                  double price,
                  double qty) {
  auto order = MakeOrderField(order_id, instrument, position_effect, direction,
                              OrderStatus::kActive, price, qty, 0, qty);
  g_order_containter.insert({order_id, order});
  return std::move(order);
}

auto MakeTradedOrder(const std::string& order_id, double traded_qty) {
  BOOST_ASSERT(g_order_containter.find(order_id) != g_order_containter.end());
  auto order = std::make_shared<OrderField>(*g_order_containter.at(order_id));
  order->status =
      traded_qty == order->qty ? OrderStatus::kAllFilled : OrderStatus::kActive;
  order->traded_qty = traded_qty;
  order->leaves_qty = order->qty - traded_qty;
  g_order_containter[order_id] = order;
  return std::move(order);
}

auto MakeNewOpenOrder(const std::string& order_id,
                      const std::string& instrument,
                      OrderDirection direction,
                      double price,
                      double qty) {
  return MakeNewOrder(order_id, instrument, PositionEffect::kOpen, direction,
                      price, qty);
}

auto MakeNewCloseOrder(
    const std::string& order_id,
    const std::string& instrument,
    OrderDirection direction,
    double price,
    double qty,
    PositionEffect position_effect = PositionEffect::kClose) {
  return MakeNewOrder(order_id, instrument, position_effect, direction, price,
                      qty);
}

auto MakeCanceledOrder(const std::string& order_id) {
  BOOST_ASSERT(g_order_containter.find(order_id) != g_order_containter.end());
  auto order = std::make_shared<OrderField>(*g_order_containter.at(order_id));
  order->status = OrderStatus::kCanceled;
  return std::move(order);
}

TEST(TestPortflioTest, BuyOrder) {
  g_order_containter.clear();
  double init_cash = 100000;
  Portfolio portflio(init_cash);
  CostBasis cost_basis;
  cost_basis.type = CommissionType::kFixed;
  cost_basis.open_commission = 0;
  cost_basis.close_commission = 0;
  cost_basis.close_today_commission = 0;
  portflio.InitInstrumentDetail("S1", 1.0, 1, cost_basis);
  portflio.HandleOrder(
      MakeNewOpenOrder("A001", "S1", OrderDirection::kBuy, 180.0, 20));
  EXPECT_EQ(3600.0, portflio.frozen_cash());
  EXPECT_EQ(init_cash - 3600.0, portflio.cash());

  portflio.HandleOrder(MakeTradedOrder("A001", 10));
  EXPECT_EQ(1800.0, portflio.frozen_cash());
  EXPECT_EQ(init_cash, portflio.total_value());

  portflio.HandleOrder(MakeTradedOrder("A001", 20));

  EXPECT_EQ(0, portflio.frozen_cash());
  EXPECT_EQ(init_cash, portflio.total_value());
  EXPECT_EQ(0, portflio.realised_pnl());

  portflio.UpdateTick(MakeTick("S1", 190.0, 1));

  EXPECT_EQ(init_cash + 200, portflio.total_value());

  portflio.HandleOrder(
      MakeNewCloseOrder("A002", "S1", OrderDirection::kSell, 200.0, 10));
  EXPECT_EQ(init_cash + 200, portflio.total_value());

  portflio.HandleOrder(MakeTradedOrder("A002", 10));
  EXPECT_EQ(200, portflio.realised_pnl());
  EXPECT_EQ(200, portflio.unrealised_pnl());
  EXPECT_EQ(init_cash + 400, portflio.total_value());

  portflio.HandleOrder(
      MakeNewCloseOrder("A003", "S1", OrderDirection::kSell, 200.0, 10));
  portflio.HandleOrder(MakeTradedOrder("A003", 10));
  EXPECT_EQ(400, portflio.realised_pnl());
  EXPECT_EQ(0, portflio.unrealised_pnl());
  EXPECT_EQ(init_cash + 400, portflio.total_value());
}

TEST(TestPortflioTest, SellOrder) {
  g_order_containter.clear();
  double init_cash = 100000;
  Portfolio portflio(init_cash);
  CostBasis cost_basis;
  cost_basis.type = CommissionType::kFixed;
  cost_basis.open_commission = 0;
  cost_basis.close_commission = 0;
  cost_basis.close_today_commission = 0;
  portflio.InitInstrumentDetail("S1", 1.0, 1, std::move(cost_basis));

  portflio.HandleOrder(
      MakeNewOpenOrder("A001", "S1", OrderDirection::kSell, 180.0, 20));
  EXPECT_EQ(3600.0, portflio.frozen_cash());
  EXPECT_EQ(init_cash - 3600.0, portflio.cash());

  portflio.HandleOrder(MakeTradedOrder("A001", 10));
  EXPECT_EQ(1800.0, portflio.frozen_cash());
  EXPECT_EQ(init_cash, portflio.total_value());

  portflio.HandleOrder(MakeTradedOrder("A001", 20));

  EXPECT_EQ(0, portflio.frozen_cash());
  EXPECT_EQ(init_cash, portflio.total_value());
  EXPECT_EQ(0, portflio.realised_pnl());

  portflio.UpdateTick(MakeTick("S1", 190.0, 1));

  EXPECT_EQ(init_cash - 200, portflio.total_value());

  portflio.HandleOrder(
      MakeNewCloseOrder("A002", "S1", OrderDirection::kBuy, 200.0, 10));
  EXPECT_EQ(init_cash - 200, portflio.total_value());

  portflio.HandleOrder(MakeTradedOrder("A002", 10));
  EXPECT_EQ(-200, portflio.realised_pnl());
  EXPECT_EQ(-200, portflio.unrealised_pnl());
  EXPECT_EQ(init_cash - 400, portflio.total_value());

  portflio.HandleOrder(
      MakeNewCloseOrder("A003", "S1", OrderDirection::kBuy, 200.0, 10));
  portflio.HandleOrder(MakeTradedOrder("A003", 10));
  EXPECT_EQ(-400, portflio.realised_pnl());
  EXPECT_EQ(0, portflio.unrealised_pnl());
  EXPECT_EQ(init_cash - 400, portflio.total_value());
}

TEST(TestPortflioTest, Margin) {
  g_order_containter.clear();
  double init_cash = 100000;
  CostBasis cost_basis;
  cost_basis.type = CommissionType::kFixed;
  cost_basis.open_commission = 0;
  cost_basis.close_commission = 0;
  cost_basis.close_today_commission = 0;
  Portfolio portflio(init_cash);
  portflio.InitInstrumentDetail("S1", 0.5, 20, cost_basis);
  portflio.InitInstrumentDetail("S2", 0.1, 10, cost_basis);

  portflio.HandleOrder(
      MakeNewOpenOrder("A001", "S1", OrderDirection::kSell, 180.0, 20));
  EXPECT_EQ(0, portflio.margin());

  portflio.HandleOrder(MakeTradedOrder("A001", 20));
  EXPECT_EQ(36000, portflio.margin());
  EXPECT_EQ(init_cash - 36000, portflio.cash());

  portflio.HandleOrder(
      MakeNewOpenOrder("A002", "S2", OrderDirection::kBuy, 50.0, 10));
  portflio.HandleOrder(MakeTradedOrder("A002", 10));
  EXPECT_EQ(36500, portflio.margin());

  portflio.HandleOrder(
      MakeNewCloseOrder("A003", "S2", OrderDirection::kSell, 60.0, 10));
  EXPECT_EQ(36500, portflio.margin());
  portflio.HandleOrder(MakeTradedOrder("A003", 10));
  EXPECT_EQ(36000, portflio.margin());
  EXPECT_EQ(1000, portflio.realised_pnl());
  EXPECT_EQ(65000 /*init_cash - 36000 + 1000*/, portflio.cash());
  EXPECT_EQ(101000, portflio.total_value());

  portflio.HandleOrder(
      MakeNewCloseOrder("A004", "S1", OrderDirection::kBuy, 310.0, 10));
  portflio.HandleOrder(MakeTradedOrder("A004", 10));
  EXPECT_EQ(18000, portflio.margin());
  EXPECT_EQ(-26000, portflio.unrealised_pnl());
  EXPECT_EQ(-25000, portflio.realised_pnl());
  EXPECT_EQ(49000, portflio.total_value());

  portflio.HandleOrder(
      MakeNewCloseOrder("A005", "S1", OrderDirection::kBuy, 200.0, 10));
  portflio.HandleOrder(MakeTradedOrder("A005", 10));
  EXPECT_EQ(0, portflio.margin());
  EXPECT_EQ(0, portflio.unrealised_pnl());
  EXPECT_EQ(-29000, portflio.realised_pnl());
  EXPECT_EQ(71000, portflio.cash());
}

TEST(TestPortflioTest, FixedCommission) {
  g_order_containter.clear();
  double init_cash = 100000;
  CostBasis cost_basis;
  cost_basis.type = CommissionType::kFixed;
  cost_basis.open_commission = 120;
  cost_basis.close_commission = 120;
  cost_basis.close_today_commission = 0;
  Portfolio portflio(init_cash);
  portflio.InitInstrumentDetail("S1", 0.5, 20, cost_basis);

  portflio.HandleOrder(
      MakeNewOpenOrder("A001", "S1", OrderDirection::kSell, 180.0, 20));
  EXPECT_EQ(0, portflio.margin());
  portflio.HandleOrder(MakeTradedOrder("A001", 20));
  EXPECT_EQ(36000, portflio.margin());
  EXPECT_EQ(init_cash - 36000 - 24, portflio.cash());
  EXPECT_EQ(24, portflio.daily_commission());

  portflio.HandleOrder(
      MakeNewOpenOrder("A002", "S1", OrderDirection::kSell, 200.0, 10));

  portflio.HandleOrder(MakeTradedOrder("A002", 5));
  EXPECT_EQ(10012, portflio.frozen_cash());
  portflio.HandleOrder(MakeCanceledOrder("A002"));
  EXPECT_EQ(30, portflio.daily_commission());
  EXPECT_EQ(0, portflio.frozen_cash());
}

TEST(TestPortflioTest, RateCommission) {
  g_order_containter.clear();
  double init_cash = 100000;
  CostBasis cost_basis;
  cost_basis.type = CommissionType::kRate;
  cost_basis.open_commission = 5;
  cost_basis.close_commission = 5;
  cost_basis.close_today_commission = 0;
  Portfolio portflio(init_cash);
  portflio.InitInstrumentDetail("S1", 0.5, 20, cost_basis);

  portflio.HandleOrder(
      MakeNewOpenOrder("A001", "S1", OrderDirection::kSell, 180.0, 20));
  EXPECT_EQ(0, portflio.margin());
  portflio.HandleOrder(MakeTradedOrder("A001", 20));
  EXPECT_EQ(36000, portflio.margin());
  EXPECT_EQ(init_cash - 36000 - 36, portflio.cash());
  EXPECT_EQ(36, portflio.daily_commission());

  portflio.HandleOrder(
      MakeNewOpenOrder("A002", "S1", OrderDirection::kSell, 200.0, 10));

  portflio.HandleOrder(MakeTradedOrder("A002", 5));
  EXPECT_EQ(10020, portflio.frozen_cash());
  portflio.HandleOrder(MakeCanceledOrder("A002"));
  EXPECT_EQ(46, portflio.daily_commission());
  EXPECT_EQ(0, portflio.frozen_cash());
}
