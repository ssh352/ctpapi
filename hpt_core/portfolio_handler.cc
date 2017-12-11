#include "portfolio_handler.h"
#include <boost/format.hpp>
#include "order_util.h"
#include "bft_core/make_message.h"

PortfolioHandler::PortfolioHandler(double init_cash,
                                   bft::ChannelDelegate* mail_box,
                                   std::string instrument,
                                   const std::string& out_dir,
                                   const std::string& csv_file_prefix,
                                   double margin_rate,
                                   int constract_multiple,
                                   CostBasis cost_basis,
                                   bool handle_input_signal)
    : portfolio_(init_cash, !handle_input_signal),
      channel_delegate_(mail_box),
      csv_(out_dir + csv_file_prefix + "_equitys.csv"),
      instrument_(instrument) {
  portfolio_.InitInstrumentDetail(std::move(instrument), margin_rate,
                                  constract_multiple, std::move(cost_basis));

  bft::MessageHandler handler;
  handler.Subscribe(&PortfolioHandler::HandleTick, this);
  handler.Subscribe(&PortfolioHandler::HandleOrder, this);
  handler.Subscribe(&PortfolioHandler::HandleDaySettleAtom, this);
  handler.Subscribe(&PortfolioHandler::HandlerInputOrder, this);
  handler.Subscribe(&PortfolioHandler::HandlerActionOrder, this);
  handler.Subscribe(&PortfolioHandler::HandlerCancelOrder, this);
  handler.Subscribe(&PortfolioHandler::BeforeTrading, this);
  channel_delegate_->Subscribe(std::move(handler));
}

void PortfolioHandler::HandlerCancelOrder(const CancelOrder& action_order) {
  channel_delegate_->Send(
      bft::MakeMessage(BacktestingAtom::value, std::move(action_order)));
}

void PortfolioHandler::HandlerActionOrder(const OrderAction& action_order) {
  channel_delegate_->Send(
      bft::MakeMessage(BacktestingAtom::value, std::move(action_order)));
}

void PortfolioHandler::HandlerInputOrder(const InputOrder& input_order) {
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

  channel_delegate_->Send(bft::MakeMessage(BacktestingAtom::value, input_order));
}

void PortfolioHandler::HandleDaySettleAtom(DaySettleAtom) {
  csv_ << last_tick_->timestamp << ","
       << str(boost::format("%0.2f") % portfolio_.total_value()) << ","
       << str(boost::format("%0.2f") % portfolio_.realised_pnl()) << ","
       << str(boost::format("%0.2f") % portfolio_.daily_commission()) << "\n";
}

void PortfolioHandler::HandleOrder(const std::shared_ptr<OrderField>& order) {
  histor_orders_.push_back(order);
  portfolio_.HandleOrder(order);
}

void PortfolioHandler::HandleTick(const std::shared_ptr<TickData>& tick) {
  portfolio_.UpdateTick(tick);
  last_tick_ = tick->tick;
}

void PortfolioHandler::BeforeTrading(BeforeTradingAtom,
                                     const TradingTime& trading_time) {
  // if (trading_time == TradingTime::kDay) {
  //  mail_box_->Send(quantitys_);
  //  mail_box_->Send(histor_orders_);
  //} else if (trading_time == TradingTime::kNight) {
  //  // new trade day for cta
  //  histor_orders_.clear();
  //  quantitys_.clear();

  //  for (const auto& pos : portfolio_.GetPositionList()) {
  //    if (pos->qty() > 0) {
  //      quantitys_.push_back(
  //          OrderPosition{instrument_, pos->direction(), pos->qty()});
  //    }
  //  }

  //  portfolio_.ResetByNewTradingDate();
  //  mail_box_->Send(quantitys_);
  //  mail_box_->Send(histor_orders_);
  //} else {
  //  BOOST_ASSERT(false);
  //}
}
