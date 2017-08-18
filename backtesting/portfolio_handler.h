#ifndef BACKTESTING_PORTFOLIO_HANDLER_H
#define BACKTESTING_PORTFOLIO_HANDLER_H
#include <fstream>
#include "common/api_struct.h"

bool IsOpenPositionEffect(PositionEffect position_effect) {
  return position_effect == PositionEffect::kOpen;
}

class Position {
 public:
  Position(OrderDirection direction, double price, int qty, double commission) {
    if (direction == OrderDirection::kBuy) {
      buys_ = qty;
      total_bot_ = qty * price;
    } else {
      sells_ = qty;
      total_sld_ = qty * price;
    }
    init_price_ = price;
    net_total_ = total_sld_ - total_bot_;
    total_commission_ = commission;
  }

  void TransactShares(PositionEffect position_effect,
                      OrderDirection direction,
                      double price,
                      int qty,
                      double commission) {
    if (IsOpenPositionEffect(position_effect)) {
      if (direction == OrderDirection::kBuy) {
        buys_ += qty;
      } else {
        sells_ += qty;
      }
    } else {  // Close Position
      if (direction == OrderDirection::kBuy) {
        if (qty <= sells_) {
          sells_ -= qty;
        } else {
          // Can't close graten oopsion qty
          throw std::exception();
        }
      } else {
        if (qty <= buys_) {
          buys_ -= qty;
        } else {
          throw std::exception();
        }
      }
    }
  }

  void UpdateMarketValue(const std::shared_ptr<Tick>& tick) {
    double midpoint = (tick->bid_price1 + tick->ask_price1) / 2;
  }

  bool IsCloseAllPosition() const { return buys_ == 0 && sells_ == 0; }

 private:
  int buys_ = 0;
  int sells_ = 0;
  double net_total_ = 0.0;
  double init_price_ = 0.0;
  double total_bot_ = 0.0;
  double total_sld_ = 0.0;
  double market_value_ = 0.0;
  double cost_basis = 0.0;
  double realised_pnl = 0.0;
  double unrealised_pnl = 0.0;
  double total_commission_ = 0.0;
};

class Portfolio {
 public:
  void UpdateMarketPrice(const std::shared_ptr<Tick>& tick) {
    if (position_ != nullptr) {
      position_->UpdateMarketValue(tick);
    }
  }

  void TransactPosition(const std::shared_ptr<OrderField>& order) {
    if (position_ == nullptr) {
      position_.reset(new Position(order->direction, order->price, order->qty,
                                   init_commission_));
    } else {
      position_->TransactShares(order->position_effect, order->direction,
                                order->price, order->qty, init_commission_);
      if (position_->IsCloseAllPosition()) {
        position_.reset();
      }
    }
  };

  double Equity() const { return equity_; }

 private:
  int init_cash = 0;
  int equity_ = 0;
  double init_commission_ = 0.0;
  std::unique_ptr<Position> position_;
};

class AbstractPortfolioHandler {
 public:
  virtual void HandleTick(const std::shared_ptr<Tick>& tick) = 0;
  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) = 0;
  virtual void HandleCloseMarket() = 0;
};

class BacktestingPortfolioHandler : public AbstractPortfolioHandler {
 public:
  BacktestingPortfolioHandler() : csv_("equitys.csv") {}
  virtual void HandleTick(const std::shared_ptr<Tick>& tick) override {
    portfolio_.UpdateMarketPrice(tick);
    last_tick_ = tick;
  }

  virtual void HandleOrder(const std::shared_ptr<OrderField>& order) override {
    portfolio_.TransactPosition(order);
  }

  virtual void HandleCloseMarket() override {
    csv_ << last_tick_->timestamp << "," << portfolio_.Equity() << "\n";
  }

 private:
  Portfolio portfolio_;
  std::shared_ptr<Tick> last_tick_;
  std::ofstream csv_;
};
#endif  // BACKTESTING_PORTFOLIO_HANDLER_H
