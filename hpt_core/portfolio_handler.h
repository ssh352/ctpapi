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
                   const std::string& out_dir,
                   const std::string& csv_file_prefix,
                   double margin_rate,
                   int constract_multiple,
                   CostBasis cost_basis,
                   bool handle_input_signal)
      : portfolio_(init_cash, !handle_input_signal),
        mail_box_(mail_box),
        csv_(out_dir + csv_file_prefix + "_equitys.csv"),
        instrument_(instrument) {
    portfolio_.InitInstrumentDetail(std::move(instrument), margin_rate,
                                    constract_multiple, std::move(cost_basis));
    mail_box_->Subscribe(&PortfolioHandler::HandleTick, this);
    mail_box_->Subscribe(&PortfolioHandler::HandleOrder, this);
    mail_box_->Subscribe(&PortfolioHandler::HandleDaySettleAtom, this);
    mail_box_->Subscribe(&PortfolioHandler::HandlerInputOrder, this);
    mail_box_->Subscribe(&PortfolioHandler::BeforeTrading, this);
  }

  void BeforeTrading(const BeforeTradingAtom&,
                     const TradingTime& trading_time) {
    if (trading_time == TradingTime::kDay) {
      mail_box_->Send(quantitys_);
      mail_box_->Send(histor_orders_);
    } else if (trading_time == TradingTime::kNight) {
      // new trade day for cta
      histor_orders_.clear();
      quantitys_.clear();

      for (const auto& pos : portfolio_.GetPositionList()) {
        if (pos->qty() > 0) {
          quantitys_.push_back(
              OrderPosition{instrument_, pos->direction(), pos->qty()});
        }
      }

      portfolio_.ResetByNewTradingDate();
      mail_box_->Send(quantitys_);
      mail_box_->Send(histor_orders_);
    } else {
      BOOST_ASSERT(false);
    }
  }

  void HandleTick(const std::shared_ptr<TickData>& tick) {
    portfolio_.UpdateTick(tick);
    last_tick_ = tick->tick;
  }

  void HandleOrder(const std::shared_ptr<OrderField>& order) {
    histor_orders_.push_back(order);
    portfolio_.HandleOrder(order);
  }

  void HandleDaySettleAtom(const DaySettleAtom&) {
    if (_finite(portfolio_.total_value()) == 0) {
      int i = 0;
    }
// last_tick_->timestamp == 1490885999000
    csv_ << last_tick_->timestamp << ","
         << str(boost::format("%0.2f") % portfolio_.total_value()) << ","
         << str(boost::format("%0.2f") % portfolio_.realised_pnl()) << ","
         << str(boost::format("%0.2f") % portfolio_.daily_commission()) << "\n";
  }

  void HandlerInputOrder(const InputOrder& input_order) {
    BOOST_ASSERT(unique_order_ids_.find(input_order.order_id) ==
                 unique_order_ids_.end());
    unique_order_ids_.insert(input_order.order_id);
    if (input_order.position_effect == PositionEffect::kClose) {
      int position_qty = portfolio_.GetPositionCloseableQty(
          input_order.instrument,
          AdjustDirectionByPositionEffect(input_order.position_effect,
                                          input_order.direction));
      if (position_qty < input_order.qty) {
        BOOST_ASSERT(false);
        return;
      }
      portfolio_.HandleNewInputCloseOrder(
          input_order.instrument,
          AdjustDirectionByPositionEffect(input_order.position_effect,
                                          input_order.direction),
          input_order.qty);
    }

    mail_box_->Send(InputOrderSignal{
        input_order.instrument, input_order.order_id,
        input_order.position_effect, input_order.direction, input_order.price,
        input_order.qty, input_order.timestamp});
  }

 private:
  Portfolio portfolio_;
  std::string instrument_;
  std::vector<OrderPosition> quantitys_;
  std::vector<std::shared_ptr<const OrderField>> histor_orders_;
  std::unordered_set<std::string> unique_order_ids_;
  std::shared_ptr<Tick> last_tick_;
  std::ofstream csv_;
  MailBox* mail_box_;
};
#endif  // BACKTESTING_PORTFOLIO_HANDLER_H
