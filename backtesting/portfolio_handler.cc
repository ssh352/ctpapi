#include "portfolio_handler.h"
#include <boost/format.hpp>

BacktestingPortfolioHandler::BacktestingPortfolioHandler(
    double init_cash,
    AbstractEventFactory* event_factory,
    std::string instrument,
    const std::string& csv_file_prefix,
    double margin_rate,
    int constract_multiple,
    CostBasis cost_basis)
    : portfolio_(init_cash),
      event_factory_(event_factory),
      csv_(csv_file_prefix + "_equitys.csv") {
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
    const InputOrder& input_order) {
  if (input_order.position_effect_ == PositionEffect::kClose ||
      input_order.position_effect_ == PositionEffect::kCloseToday) {
    int position_qty = portfolio_.GetPositionCloseableQty(
        input_order.instrument_,
        input_order.order_direction_ == OrderDirection::kBuy
            ? OrderDirection::kSell
            : OrderDirection::kBuy);
    if (position_qty < input_order.qty_) {
      return;
    }
    portfolio_.HandleNewInputCloseOrder(input_order.instrument_,
                                        input_order.order_direction_,
                                        input_order.qty_);
  }

  event_factory_->EnqueueInputOrderEvent(input_order);
}
