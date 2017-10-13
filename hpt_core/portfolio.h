#ifndef BACKTESTING_PROTFOLIO_H
#define BACKTESTING_PROTFOLIO_H
#include <memory>
#include <unordered_map>
#include <boost/optional.hpp>
#include "common/api_struct.h"
#include "position.h"

class Portfolio {

 public:
  Portfolio(double init_cash, bool frozen_close_qty_by_rtn_order = true);

  void ResetByNewTradingDate();

  void InitInstrumentDetail(std::string instrument,
                            double margin_rate,
                            int constract_multiple,
                            CostBasis cost_basis);

  void UpdateTick(const std::shared_ptr<TickData>& tick);

  void HandleOrder(const std::shared_ptr<OrderField>& order);

  void HandleNewInputCloseOrder(const std::string& instrument,
                                OrderDirection direction,
                                int qty);

  int GetPositionQty(const std::string& instrument, OrderDirection direction) const;

  int GetPositionCloseableQty(const std::string& instrument,
                              OrderDirection direction) const;

  double total_value() const {
    return cash_ + frozen_cash_ + margin_ + unrealised_pnl_;
  }

  double realised_pnl() const { return realised_pnl_; }

  double unrealised_pnl() const { return unrealised_pnl_; }

  double frozen_cash() const { return frozen_cash_; }

  double cash() const { return cash_; }

  double margin() const { return margin_; }

  double daily_commission() const { return daily_commission_; }

  boost::optional<Position> position(const std::string& instrument) const {
    if (position_container_.find(instrument) == position_container_.end()) {
      return boost::optional<Position>();
    }

    return position_container_.at(instrument);
  }

  int UnfillOpenQty(const std::string& instrument,
                    OrderDirection direction) const;

  int UnfillCloseQty(const std::string& instrument,
                     OrderDirection directon) const;

  std::vector<std::shared_ptr<OrderField> > UnfillOpenOrders(
      const std::string& instrument,
      OrderDirection direction) const;

  std::vector<std::shared_ptr<OrderField> > UnfillCloseOrders(
      const std::string& instrument,
      OrderDirection direction) const;

  const std::unordered_map<std::string, Position>& positions() const {
    return position_container_;
  }

  int PositionQty(const std::string& instrument,
                  OrderDirection direction) const;

  double UpdateCostBasis(PositionEffect position_effect,
                         double price,
                         int qty,
                         int constract_multiple,
                         const CostBasis& const_basis);

  double CalcCommission(PositionEffect position_effect,
                        double price,
                        int qty,
                        int constract_multiple,
                        const CostBasis& cost_basis);

  std::vector<std::string> InstrumentList() const;

  double init_cash_ = 0.0;
  double cash_ = 0.0;
  double frozen_cash_ = 0.0;
  double market_value_ = 0.0;
  double realised_pnl_ = 0.0;
  double unrealised_pnl_ = 0.0;
  double margin_ = 0.0;
  double daily_commission_ = 0.0;
  std::unordered_map<std::string, std::shared_ptr<OrderField> >
      order_container_;
  std::unordered_map<std::string, Position> position_container_;
  std::unordered_map<std::string, std::tuple<double, int, CostBasis> >
      instrument_info_container_;
  bool frozen_close_qty_by_rtn_order_;
};

#endif  // BACKTESTING_PROTFOLIO_H
