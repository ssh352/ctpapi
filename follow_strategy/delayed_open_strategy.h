#ifndef FOLLOW_STRATEGY_DELAYED_OPEN_STRATEGY_H
#define FOLLOW_STRATEGY_DELAYED_OPEN_STRATEGY_H
#include "hpt_core/portfolio.h"
#include "follow_strategy/cta_generic_strategy.h"
#include "follow_strategy/cta_signal.h"
#include "follow_strategy/cta_signal_dispatch.h"
#include "follow_strategy/logging_defines.h"
#include "follow_strategy/strategy_order_dispatch.h"
#include "follow_strategy/string_util.h"
#include "order_util.h"

template <typename MailBox>
class DelayedOpenStrategy : public EnterOrderObserver,
                            public CTASignalObserver {
 public:
  DelayedOpenStrategy(MailBox* mail_box,
                      std::string master_account_id,
                      std::string slave_account_id,
                      int delayed_open_order)
      : mail_box_(mail_box),
        master_account_id_(master_account_id),
        slave_account_id_(slave_account_id),
        delayed_open_order_by_seconds_(delayed_open_order),
        portfolio_(1000000),
        master_portfolio_(1000000) {
    // TODO: For test
    portfolio_.InitInstrumentDetail("I1", 0.02, 10,
                                    CostBasis{CommissionType::kFixed, 0, 0, 0});
    master_portfolio_.InitInstrumentDetail(
        "I1", 0.02, 10, CostBasis{CommissionType::kFixed, 0, 0, 0});

    signal_dispatch_ =
        std::make_shared<CTASignalDispatch>(this, slave_account_id_);
    signal_dispatch_->SubscribeEnterOrderObserver(this);

    mail_box_->Subscribe(&DelayedOpenStrategy::HandleInitPosition, this);
    mail_box_->Subscribe(&DelayedOpenStrategy::HandleHistoryOrder, this);
    mail_box_->Subscribe(&DelayedOpenStrategy::HandleOrder, this);
    mail_box_->Subscribe(&DelayedOpenStrategy::HandleCTASignalInitPosition,
                         this);
    mail_box_->Subscribe(&DelayedOpenStrategy::HandleCTASignalHistoryOrder,
                         this);
    mail_box_->Subscribe(&DelayedOpenStrategy::HandleCTASignalOrder, this);
    mail_box_->Subscribe(&DelayedOpenStrategy::BeforeTrading, this);
    mail_box_->Subscribe(&DelayedOpenStrategy::BeforeCloseMarket, this);
    mail_box_->Subscribe(&DelayedOpenStrategy::HandleTick, this);
  }

  void BeforeTrading(const BeforeTradingAtom&,
                     const TradingTime& trading_time) {}

  void BeforeCloseMarket(const BeforeCloseMarketAtom&) {}

  void HandleTick(const std::shared_ptr<TickData>& tick) {
    auto it_end = std::find_if(
        pending_delayed_open_order_.begin(), pending_delayed_open_order_.end(),
        [=, &tick](const auto& order) {
          return tick->tick->timestamp <
                 order.timestamp_ + delayed_open_order_by_seconds_ * 1000;
        });
    for (auto it = pending_delayed_open_order_.begin(); it != it_end; ++it) {
      signal_dispatch_->OpenOrder(it->instrument_, GenerateOrderId(),
                                  it->order_direction_, it->price_, it->qty_);
    }
    pending_delayed_open_order_.erase(pending_delayed_open_order_.begin(),
                                      it_end);
  }

  void HandleInitPosition(const std::vector<OrderPosition>& quantitys) {}

  void HandleOrder(const std::shared_ptr<OrderField>& order) {
    portfolio_.HandleOrder(order);
    signal_dispatch_->RtnOrder(order);
  }

  void HandleHistoryOrder(
      const std::vector<std::shared_ptr<const OrderField> >& orders) {}
  // CTASignal
  void HandleCTASignalInitPosition(
      const CTASignalAtom&,
      const std::vector<OrderPosition>& quantitys) {}

  void HandleCTASignalHistoryOrder(
      const CTASignalAtom&,
      const std::vector<std::shared_ptr<const OrderField> >& orders) {}

  void HandleCTASignalOrder(const CTASignalAtom& cta_signal_atom,
                            const std::shared_ptr<OrderField>& order) {
    master_portfolio_.HandleOrder(order);
    signal_dispatch_->RtnOrder(order);
  }

  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_id,
                         OrderDirection direction,
                         double price,
                         int quantity) override {
    mail_box_->Send(InputOrder{instrument, order_id, "S1",
                               PositionEffect::kOpen, direction, price,
                               quantity, 0});
  }

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_id,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          double price,
                          int quantity) override {
    mail_box_->Send(InputOrder{instrument, order_id, "S1", position_effect,
                               direction, price, quantity, 0});
  }

  virtual void CancelOrder(const std::string& order_id) override {
    mail_box_->Send(CancelOrderSignal{order_id});
  }

  // CTASignalObserver
  virtual void HandleOpening(
      const std::shared_ptr<const OrderField>& order_data) override {
    if (order_data->strategy_id != master_account_id_) {
      return;
    }
  }

  virtual void HandleCloseing(
      const std::shared_ptr<const OrderField>& order_data) override {
    if (order_data->strategy_id != master_account_id_) {
      return;
    }
    auto positions = portfolio_.positions();
    if (positions.find(order_data->instrument_id) != positions.end()) {
      const auto& position = positions.at(order_data->instrument_id);
      int qty = order_data->direction == OrderDirection::kBuy
                    ? position.short_qty()
                    : position.long_qty();
      if (qty > 0) {
        signal_dispatch_->CloseOrder(
            order_data->instrument_id, GenerateOrderId(), order_data->direction,
            order_data->position_effect, order_data->input_price,
            qty > order_data->qty ? qty : std::min<int>(order_data->qty, qty));
      }
    }
  }

  virtual void HandleCanceled(
      const std::shared_ptr<const OrderField>& order_data) override {
    throw std::logic_error("The method or operation is not implemented.");
  }

  virtual void HandleClosed(
      const std::shared_ptr<const OrderField>& order_data) override {
    if (order_data->strategy_id != master_account_id_) {
      return;
    }

    OrderDirection position_direction =
        OppositeOrderDirection(order_data->direction);
    int qty = master_portfolio_.PositionQty(order_data->instrument_id,
                                            position_direction);
    if (qty == 0) {
      auto orders = portfolio_.UnfillOrders(order_data->instrument_id,
                                            position_direction);
      std::for_each(orders.begin(), orders.end(), [=](const auto& order) {
        signal_dispatch_->CancelOrder(order->order_id);
      });
    } else {
      auto orders = portfolio_.UnfillOrders(order_data->instrument_id,
                                            position_direction);
      std::for_each(orders.begin(), orders.end(), [=](const auto& order) {
        signal_dispatch_->CancelOrder(order->order_id);
        signal_dispatch_->OpenOrder(order->instrument_id, GenerateOrderId(),
                                    position_direction, order->input_price,
                                    qty);
      });
    }
  }

  virtual void HandleOpened(
      const std::shared_ptr<const OrderField>& order_data) override {
    if (order_data->strategy_id != master_account_id_) {
      return;
    }
    pending_delayed_open_order_.push_back(InputOrderSignal{
        order_data->instrument_id, "", "S1", PositionEffect::kOpen,
        order_data->direction, order_data->trading_price,
        order_data->trading_qty, order_data->update_timestamp});
  }

 private:
  std::string GenerateOrderId() {
    return boost::lexical_cast<std::string>(order_id_seq_++);
  }
  // class CompareOrderId {
  // public:
  //  using is_transparent = void;
  //  bool operator()(const InputOrderSignal& l,
  //                  const InputOrderSignal& r) const {
  //    return l.timestamp_ < r.timestamp_;
  //  }

  //  bool operator()(const std::string& order_id,
  //                  const InputOrderSignal& r) const {
  //    return order_id < r.order_id;
  //  }

  //  bool operator()(const InputOrderSignal& l,
  //                  const std::string& order_id) const {
  //    return l.order_id < order_id;
  //  }
  //  bool operator()(const InputOrderSignal& l, TimeStamp timestamp) const {
  //    return l.timestamp_ < timestamp;
  //  }
  //  bool operator()(TimeStamp timestamp, const InputOrderSignal& l) const {
  //    return timestamp < l.timestamp_;
  //  }
  //};
  Portfolio master_portfolio_;
  Portfolio portfolio_;
  MailBox* mail_box_;
  std::string default_order_exchange_id_;
  std::string master_account_id_;
  std::string slave_account_id_;
  std::shared_ptr<CTASignalDispatch> signal_dispatch_;
  std::list<InputOrderSignal> pending_delayed_open_order_;
  int delayed_open_order_by_seconds_ = 0;
  int order_id_seq_ = 0;
};

#endif  // FOLLOW_STRATEGY_DELAYED_OPEN_STRATEGY_H
