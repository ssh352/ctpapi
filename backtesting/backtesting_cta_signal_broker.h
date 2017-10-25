#include "..\follow_strategy\order_util.h"
#ifndef BACKTESTING_BACKTESTING_CTA_SIGNAL_BROKER_H
#define BACKTESTING_BACKTESTING_CTA_SIGNAL_BROKER_H

template <class MailBox>
class BacktestingCTASignalBroker {
 public:
  BacktestingCTASignalBroker(
      MailBox* mail_box,
      std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>
          cta_signal_container,
      std::string instrument)
      : mail_box_(mail_box),
        keep_memory_(std::move(cta_signal_container)),
        portfolio_(100 * 10000),
        instrument_(std::move(instrument)),
        csv_("debug_log.txt") {
    auto null_deleter = [](CTATransaction*) {};
    for (auto& item : keep_memory_) {
      for (int i = 0; i < item.second; ++i) {
        transactions_.push_back(std::shared_ptr<CTATransaction>(
            &item.first.get()[i], null_deleter));
      }
    }
    range_beg_it_ = transactions_.begin();

    portfolio_.InitInstrumentDetail(instrument_, 0.1, 10,
                                    {CommissionType::kFixed, 0, 0, 0});
    mail_box_->Subscribe(&BacktestingCTASignalBroker::HandleTick, this);
    mail_box_->Subscribe(&BacktestingCTASignalBroker::BeforeTrading, this);
  }

  // struct OrderPosition {
  //   std::string instrument;
  //   OrderDirection order_direction;
  //   int quantity;
  // };

  void BeforeTrading(const BeforeTradingAtom&,
                     const TradingTime& trading_time) {
    csv_ << "==================================================\n";
    if (trading_time == TradingTime::kDay) {
      mail_box_->Send(CTASignalAtom::value, quantitys_);
      mail_box_->Send(CTASignalAtom::value, histor_orders_);
      csv_ << "Day open:"
           << "positions:" << quantitys_.size()
           << "history order:" << histor_orders_.size() << "\n";
    } else if (trading_time == TradingTime::kNight) {
      // new trade day for cta
      histor_orders_.clear();
      quantitys_.clear();

      for (const auto& key_and_value : portfolio_.positions()) {
        if (key_and_value.second.long_qty() > 0) {
          quantitys_.push_back(OrderPosition{instrument_, OrderDirection::kBuy,
                                             key_and_value.second.long_qty()});
        }

        if (key_and_value.second.short_qty() > 0) {
          quantitys_.push_back(OrderPosition{instrument_, OrderDirection::kSell,
                                             key_and_value.second.long_qty()});
        }
      }

      portfolio_.ResetByNewTradingDate();
      mail_box_->Send(CTASignalAtom::value, quantitys_);
      mail_box_->Send(CTASignalAtom::value, histor_orders_);

      csv_ << "Night open:"
           << "positions:" << quantitys_.size()
           << "history order:" << histor_orders_.size() << "\n";
    } else {
      BOOST_ASSERT(false);
    }
  }

  void HandleTick(const std::shared_ptr<TickData>& tick) {
    if (range_beg_it_ == transactions_.end()) {
      return;
    }

    auto end_it =
        std::find_if(range_beg_it_, transactions_.end(),
                     [=](const std::shared_ptr<CTATransaction>& tran) {
                       return tick->tick->timestamp < tran->timestamp;
                     });

    for (auto i = range_beg_it_; i != end_it; ++i) {
      // if ((*i)->position_effect == backtesting_position_effect_) {
      //  delay_input_order_.push_back((*i));
      //} else {
      std::string order_id =
          boost::lexical_cast<std::string>(current_order_id_++);
      {
        auto order = std::make_shared<OrderField>();
        order->order_id = order_id;
        order->instrument_id = instrument_;
        order->position_effect = ParseThostFtdcPosition((*i)->position_effect);
        order->direction = ParseThostFtdcOrderDirection((*i)->direction);
        order->status = OrderStatus::kActive;
        order->input_price = (*i)->price;
        order->avg_price = (*i)->price;
        order->trading_price = 0.0;
        order->leaves_qty = (*i)->qty;
        order->qty = (*i)->qty;
        order->trading_qty = 0;
        order->input_timestamp = tick->tick->timestamp;
        order->update_timestamp = tick->tick->timestamp;
        SendOrder(std::move(order));
      }

      if (static_cast<OrderStatus>((*i)->status) == OrderStatus::kAllFilled) {
        auto order = std::make_shared<OrderField>();
        order->order_id = order_id;
        order->instrument_id = instrument_;
        order->position_effect = ParseThostFtdcPosition((*i)->position_effect);
        order->direction = ParseThostFtdcOrderDirection((*i)->direction);
        order->status = OrderStatus::kAllFilled;
        order->input_price = (*i)->price;
        order->avg_price = (*i)->price;
        order->trading_price = (*i)->price;
        order->leaves_qty = 0;
        order->qty = (*i)->qty;
        order->trading_qty = (*i)->qty;
        order->input_timestamp = tick->tick->timestamp;
        order->update_timestamp = tick->tick->timestamp;
        SendOrder(std::move(order));
      } else if (static_cast<OrderStatus>((*i)->status) ==
                 OrderStatus::kCanceled) {
        if ((*i)->traded_qty != 0) {
          auto order = std::make_shared<OrderField>();
          order->order_id = order_id;
          order->instrument_id = instrument_;

          order->position_effect =
              ParseThostFtdcPosition((*i)->position_effect);
          order->direction = ParseThostFtdcOrderDirection((*i)->direction);
          order->status = OrderStatus::kActive;
          order->input_price = (*i)->price;
          order->avg_price = (*i)->price;
          order->trading_price = 0;
          order->leaves_qty = (*i)->qty - (*i)->traded_qty;
          order->qty = (*i)->qty;
          order->trading_price = 0;
          order->input_timestamp = tick->tick->timestamp;
          order->update_timestamp = tick->tick->timestamp;
          SendOrder(std::move(order));
        }
        auto order = std::make_shared<OrderField>();
        order->order_id = order_id;
        order->instrument_id = instrument_;
        order->position_effect = ParseThostFtdcPosition((*i)->position_effect);
        order->direction = ParseThostFtdcOrderDirection((*i)->direction);
        order->status = OrderStatus::kCanceled;
        order->input_price = (*i)->price;
        order->avg_price = (*i)->price;
        order->trading_price = 0.0;
        order->leaves_qty = (*i)->qty - (*i)->traded_qty;
        order->qty = (*i)->qty;
        order->trading_qty = 0;
        order->input_timestamp = tick->tick->timestamp;
        order->update_timestamp = tick->tick->timestamp;
        SendOrder(std::move(order));
      } else {
      }
    }

    range_beg_it_ = end_it;
  }

 private:
  class CompareOrderId {
   public:
    using is_transparent = void;
    bool operator()(const std::shared_ptr<OrderField>& l,
                    const std::shared_ptr<OrderField>& r) const {
      return l->order_id < r->order_id;
    }

    bool operator()(const std::string& order_id,
                    const std::shared_ptr<OrderField>& r) const {
      return order_id < r->order_id;
    }

    bool operator()(const std::shared_ptr<OrderField>& l,
                    const std::string& order_id) const {
      return l->order_id < order_id;
    }
    bool operator()(const std::shared_ptr<OrderField>& l,
                    TimeStamp timestamp) const {
      return l->input_timestamp < timestamp;
    }
    bool operator()(TimeStamp timestamp,
                    const std::shared_ptr<OrderField>& l) const {
      return timestamp < l->input_timestamp;
    }
  };

  PositionEffect ParseThostFtdcPosition(int position_effect) const {
    PositionEffect ret = PositionEffect::kUndefine;
    switch (position_effect) {
      case 0:
        ret = PositionEffect::kOpen;
        break;
      case 1:
        ret = PositionEffect::kClose;
        break;
      case 2:
        ret = PositionEffect::kCloseToday;
        break;
      default:
        break;
    }
    return ret;
  }

  OrderDirection ParseThostFtdcOrderDirection(int order_direction) const {
    OrderDirection ret = OrderDirection::kUndefine;
    switch (order_direction) {
      case 0:
        ret = OrderDirection::kBuy;
        break;
      case 1:
        ret = OrderDirection::kSell;
        break;
      default:
        break;
    }
    return ret;
  }

  void SendOrder(std::shared_ptr<OrderField> order) {
    order->strategy_id = "cta";
    //if (order->status == OrderStatus::kActive &&
    //    IsCloseOrder(order->position_effect)) {
    //  portfolio_.HandleNewInputCloseOrder(order->instrument_id,
    //                                      order->direction, order->qty);
    //}
    csv_ << "BeforeHandleOrder:"
         << "(" << order->order_id << ")"
         << (order->position_effect == PositionEffect::kOpen ? "O," : "C,")
         << (order->direction == OrderDirection::kBuy ? "B," : "S,")
         << (order->status == OrderStatus::kActive ? "N" : "F") << ","
         << order->trading_price << "," << order->qty << ","
         << order->trading_qty << "\n";
    portfolio_.HandleOrder(order);

    int long_qty = 0;
    int short_qty = 0;
    for (const auto& pos : portfolio_.positions()) {
      long_qty += pos.second.long_qty();
      short_qty += pos.second.short_qty();
    }

    csv_ << "ShowPortfolio:"
         << "cash:" << portfolio_.cash()
         << " frozent_cash:" << portfolio_.frozen_cash()
         << " margin:" << portfolio_.margin()
         << " realised_pnl:" << portfolio_.realised_pnl()
         << " daily_commission:" << portfolio_.daily_commission()
         << " long qty:" << long_qty << " short qty:" << short_qty << "\n";

    histor_orders_.push_back(order);
    mail_box_->Send(CTASignalAtom::value, std::move(order));
  }

  MailBox* mail_box_;
  std::list<std::shared_ptr<CTATransaction>> transactions_;
  std::list<std::shared_ptr<CTATransaction>>::iterator range_beg_it_;
  std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>> keep_memory_;
  std::list<std::shared_ptr<CTATransaction>> delay_input_order_;

  Portfolio portfolio_;
  std::vector<OrderPosition> quantitys_;
  std::vector<std::shared_ptr<const OrderField>> histor_orders_;

  int delayed_input_order_minute_ = 0;
  int cancel_order_after_minute_ = 0;
  int backtesting_position_effect_ = 0;
  int current_order_id_ = 0;
  std::ofstream csv_;
  std::string instrument_;
};

#endif  // BACKTESTING_BACKTESTING_CTA_SIGNAL_BROKER_H
