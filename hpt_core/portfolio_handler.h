#ifndef BACKTESTING_PORTFOLIO_HANDLER_H
#define BACKTESTING_PORTFOLIO_HANDLER_H
#include <fstream>
#include "common/api_struct.h"
#include "portfolio.h"

template <class MailBox>
class PortfolioHandler {
 public:
  PortfolioHandler(double init_cash,
                   MailBox* mail_box,
                   std::string instrument,
                   const std::string& csv_file_prefix,
                   double margin_rate,
                   int constract_multiple,
                   CostBasis cost_basis)
      : portfolio_(init_cash),
        mail_box_(mail_box),
        csv_(csv_file_prefix + "_equitys.csv") {
    portfolio_.InitInstrumentDetail(std::move(instrument), margin_rate,
                                    constract_multiple, std::move(cost_basis));
    mail_box_->Subscribe(&PortfolioHandler::HandleTick, this);
    mail_box_->Subscribe(&PortfolioHandler::HandleOrder, this);
    mail_box_->Subscribe(&PortfolioHandler::HandleCloseMarket, this);
    mail_box_->Subscribe(&PortfolioHandler::HandlerInputOrder, this);
  }

  void HandleTick(const std::shared_ptr<TickData>& tick) {
    portfolio_.UpdateTick(tick);
    last_tick_ = tick->tick;
  }

  void HandleOrder(const std::shared_ptr<OrderField>& order) {
    portfolio_.HandleOrder(order);
  }

  void HandleCloseMarket(const CloseMarket&) {
    csv_ << last_tick_->timestamp << ","
         << str(boost::format("%0.2f") % portfolio_.total_value()) << ","
         << str(boost::format("%0.2f") % portfolio_.realised_pnl()) << ","
         << str(boost::format("%0.2f") % portfolio_.daily_commission()) << "\n";
  }

  void HandlerInputOrder(const InputOrderSignal& input_order) {
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

    mail_box_->Send(InputOrder{input_order.instrument_,
                               input_order.position_effect_,
                               input_order.order_direction_, input_order.price_,
                               input_order.qty_, input_order.timestamp_});
  }

 private:
  Portfolio portfolio_;
  std::shared_ptr<Tick> last_tick_;
  std::ofstream csv_;
  MailBox* mail_box_;
};
#endif  // BACKTESTING_PORTFOLIO_HANDLER_H
