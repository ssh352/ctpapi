#ifndef BACKTESTING_BACKTESTING_CTA_SIGNAL_BROKER_H
#define BACKTESTING_BACKTESTING_CTA_SIGNAL_BROKER_H

template <class MailBox>
class BacktestingCTASignalBroker {
 public:
  BacktestingCTASignalBroker(
      MailBox* mail_box,
      std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>
          cta_signal_container)
      : mail_box_(mail_box),
        keep_memory_(std::move(cta_signal_container)),
        portfolio_(100 * 10000) {
    auto null_deleter = [](CTATransaction*) {};
    for (auto& item : keep_memory_) {
      for (int i = 0; i < item.second; ++i) {
        transactions_.push_back(std::shared_ptr<CTATransaction>(
            &item.first.get()[i], null_deleter));
      }
    }
    range_beg_it_ = transactions_.begin();
    mail_box_->Subscribe(&BacktestingCTASignalBroker::HandleTick, this);
    mail_box_->Subscribe(&BacktestingCTASignalBroker::BeforeTrading, this);
  }

  // struct OrderPosition {
  //   std::string instrument;
  //   OrderDirection order_direction;
  //   int quantity;
  // };

  void BeforeTrading(const BeforeTradingAtom&, const TimeStamp& time_stamp) {
    boost::posix_time::ptime now(boost::gregorian::date(1970, 1, 1),
                                 boost::posix_time::milliseconds(time_stamp));
    boost::posix_time::ptime day(now.date(),
                                 boost::posix_time::time_duration(9, 0, 0));
    boost::posix_time::ptime night(now.date(),
                                   boost::posix_time::time_duration(21, 0, 0));

    if (std::abs((now - day).total_seconds()) < 600) {
      mail_box_->Send(CTASignalAtom::value, quantitys_);
      mail_box_->Send(CTASignalAtom::value, histor_orders_);
    } else if (std::abs((now - night).total_seconds()) < 600) {
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

      mail_box_->Send(CTASignalAtom::value, quantitys_);
      mail_box_->Send(CTASignalAtom::value, histor_orders_);
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
        order->position_effect =
            static_cast<PositionEffect>((*i)->position_effect);
        order->direction = static_cast<OrderDirection>((*i)->direction);
        order->status = OrderStatus::kActive;
        order->price = (*i)->price;
        order->avg_price = (*i)->price;
        order->leaves_qty = (*i)->qty;
        order->qty = (*i)->qty;
        order->traded_qty = 0;
        order->input_timestamp = tick->tick->timestamp;
        order->update_timestamp = tick->tick->timestamp;
        SendOrder(std::move(order));
      }

      if (static_cast<OrderStatus>((*i)->status) == OrderStatus::kAllFilled) {
        auto order = std::make_shared<OrderField>();
        order->order_id = order_id;
        order->instrument_id = instrument_;
        order->position_effect =
            static_cast<PositionEffect>((*i)->position_effect);
        order->direction = static_cast<OrderDirection>((*i)->direction);
        order->status = OrderStatus::kAllFilled;
        order->price = (*i)->price;
        order->avg_price = (*i)->price;
        order->leaves_qty = 0;
        order->qty = (*i)->qty;
        order->traded_qty = (*i)->qty;
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
              static_cast<PositionEffect>((*i)->position_effect);
          order->direction = static_cast<OrderDirection>((*i)->direction);
          order->status = OrderStatus::kActive;
          order->price = (*i)->price;
          order->avg_price = (*i)->price;
          order->leaves_qty = (*i)->qty - (*i)->traded_qty;
          order->qty = (*i)->qty;
          order->traded_qty = (*i)->traded_qty;
          order->input_timestamp = tick->tick->timestamp;
          order->update_timestamp = tick->tick->timestamp;
          SendOrder(std::move(order));
        }
        auto order = std::make_shared<OrderField>();
        order->order_id = order_id;
        order->instrument_id = instrument_;
        order->position_effect =
            static_cast<PositionEffect>((*i)->position_effect);
        order->direction = static_cast<OrderDirection>((*i)->direction);
        order->status = OrderStatus::kCanceled;
        order->price = (*i)->price;
        order->avg_price = (*i)->price;
        order->leaves_qty = (*i)->qty - (*i)->traded_qty;
        order->qty = (*i)->qty;
        order->traded_qty = (*i)->traded_qty;
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

  void SendOrder(std::shared_ptr<OrderField> order) {
    // portfolio_.HandleOrder(order);
    histor_orders_.push_back(order);
    mail_box_->Send(std::move(order));
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
  std::string instrument_;
};

#endif  // BACKTESTING_BACKTESTING_CTA_SIGNAL_BROKER_H
