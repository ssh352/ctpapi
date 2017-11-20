#ifndef STRATEGY_UNITTEST_DEFAULT_DELAY_OPEN_STRATEGY_EX_FIXTURE_H
#define STRATEGY_UNITTEST_DEFAULT_DELAY_OPEN_STRATEGY_EX_FIXTURE_H
#include "strategy_fixture.h"

class DefaultDelayedOpenStrategyExFixture : public StrategyFixture {
 public:
  DefaultDelayedOpenStrategyExFixture(std::string master_account_id,
                                      std::string slave_account_id,
                                      std::string instrument,
                                      int tick_qty)
      : StrategyFixture(slave_account_id),
        master_account_id_(std::move(master_account_id)),
        slave_account_id_(std::move(slave_account_id)),
        defalut_instrument_id_(std::move(instrument)),
        default_market_tick_qty_(tick_qty) {}
  template <typename... Ts>
  void MasterNewOpenOrder(const std::string& order_id,
                          OrderDirection direction,
                          double price,
                          int qty,
                          Ts&&... params) {
    Send(MakeNewOpenOrder(master_account_id_, order_id, defalut_instrument_id_,
                          direction, price, qty),
         CTAPositionQty{std::forward<Ts>(params)...});
  }

  template <typename... Ts>
  void MasterNewCloseOrder(const std::string& order_id,
                           OrderDirection direction,
                           double price,
                           int qty,
                           Ts&&... params) {
    Send(MakeNewCloseOrder(master_account_id_, order_id, defalut_instrument_id_,
                           direction, price, qty),
         CTAPositionQty{std::forward<Ts>(params)...});
  }

  template <typename... Ts>
  void MasterTradedOrder(const std::string& order_id,
                         int trading_qty,
                         Ts&&... params) {
    Send(MakeTradedOrder(master_account_id_, order_id, trading_qty),
         CTAPositionQty{std::forward<Ts>(params)...});
  }

  template <typename... Ts>
  void MasterTradedOrderWithTradingPrice(const std::string& order_id,
                                         double trading_price,
                                         int trading_qty,
                                         Ts&&... params) {
    Send(MakeTradedOrder(master_account_id_, order_id, trading_qty),
         CTAPositionQty{std::forward<Ts>(params)...});
  }

  template <typename... Ts>
  void MasterCancelOrder(const std::string& order_id, Ts&&... params) {
    auto order = MakeCanceledOrder(master_account_id_, order_id);
    Send(order, CTAPositionQty{std::forward<Ts>(params)...});
  }

  void MarketTick(double last_price) {
    Send(MakeTick(defalut_instrument_id_, last_price, default_market_tick_qty_,
                  now_timestamp_));
  }

  void MarketTick(double last_price, double bid_price, double ask_price) {
    Send(MakeTick(defalut_instrument_id_, last_price, bid_price, ask_price,
                  default_market_tick_qty_, now_timestamp_));
  }

  void TradedOrder(const std::string& order_id, int trading_qty) {
    Send(MakeTradedOrder(slave_account_id_, order_id, trading_qty));
  }

  void TradedOrderWithTradingPrice(const std::string& order_id,
                                   double trading_price,
                                   int trading_qty) {
    Send(MakeTradedOrder(slave_account_id_, order_id, trading_price,
                         trading_qty));
  }

  void CancelOrderWithOrderNo(const std::string& order_id) {
    Send(MakeCanceledOrder(slave_account_id_, order_id));
  }

 private:
  std::string slave_account_id_;
  std::string master_account_id_;
  std::string defalut_instrument_id_;
  int default_market_tick_qty_;
};

#endif  // STRATEGY_UNITTEST_DEFAULT_DELAY_OPEN_STRATEGY_EX_FIXTURE_H
