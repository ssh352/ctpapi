#include "portfolio_handler.h"
#include <boost/format.hpp>

BacktestingPortfolioHandler::BacktestingPortfolioHandler(
    double init_cash,
    AbstractEventFactory* event_factory,
    std::string instrument,
    double margin_rate,
    int constract_multiple,
    CostBasis cost_basis)
    : portfolio_(init_cash),
      event_factory_(event_factory),
      csv_("equitys.csv") {
  portfolio_.InitInstrumentDetail(std::move(instrument), margin_rate,
                                  constract_multiple, std::move(cost_basis));
}

void BacktestingPortfolioHandler::HandleTick(
    const std::shared_ptr<TickData>& tick) {
  portfolio_.UpdateTick(tick);
  last_tick_ = tick->tick;
}

void BacktestingPortfolioHandler::HandleOrder(
    const std::shared_ptr<OrderField>& order) {
  portfolio_.HandleOrder(order);
}

void BacktestingPortfolioHandler::HandleCloseMarket() {
  csv_ << last_tick_->timestamp << ","
       << str(boost::format("%0.2f") % portfolio_.total_value()) << ","
       << str(boost::format("%0.2f") % portfolio_.realised_pnl()) << ","
       << str(boost::format("%0.2f") % portfolio_.daily_commission()) << "\n";
}

inline void BacktestingPortfolioHandler::HandlerInputOrder(
    const std::string& instrument,
    PositionEffect position_effect,
    OrderDirection direction,
    double price,
    int qty,
    TimeStamp timestamp) {
  if (position_effect == PositionEffect::kClose ||
      position_effect == PositionEffect::kCloseToday) {
    int position_qty = portfolio_.GetPositionCloseableQty(
        instrument, direction == OrderDirection::kBuy ? OrderDirection::kSell
                                                      : OrderDirection::kBuy);
    if (position_qty < qty) {
      return;
    }
    portfolio_.HandleNewInputCloseOrder(instrument, direction, qty);
  }

  event_factory_->EnqueueInputOrderEvent(instrument, position_effect, direction,
                                         price, qty, timestamp);
}
