#ifndef FOLLOW_STRATEGY_DELAYED_OPEN_STRATEGY_H
#define FOLLOW_STRATEGY_DELAYED_OPEN_STRATEGY_H
#include <boost/log/sources/logger.hpp>
#include <boost/log/common.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include "hpt_core/portfolio.h"
#include "follow_strategy/cta_generic_strategy.h"
#include "follow_strategy/cta_signal.h"
#include "follow_strategy/cta_signal_dispatch.h"
#include "follow_strategy/logging_defines.h"
#include "follow_strategy/strategy_order_dispatch.h"
#include "follow_strategy/string_util.h"
#include "follow_strategy/logging_defines.h"
#include "hpt_core/time_util.h"
#include "order_util.h"




template <typename MailBox>
class DelayedOpenStrategy : public EnterOrderObserver,
                            public CTASignalObserver {
 public:
  DelayedOpenStrategy(MailBox* mail_box,
                      std::string master_account_id,
                      std::string slave_account_id,
                      int delayed_open_order,
                      const std::string& instrument)
      : mail_box_(mail_box),
        master_account_id_(master_account_id),
        slave_account_id_(slave_account_id),
        delayed_open_order_by_seconds_(delayed_open_order),
        portfolio_(1000000, false),
        master_portfolio_(1000000) {
    portfolio_.InitInstrumentDetail(instrument, 0.02, 10,
                                    CostBasis{CommissionType::kFixed, 0, 0, 0});
    master_portfolio_.InitInstrumentDetail(
        instrument, 0.02, 10, CostBasis{CommissionType::kFixed, 0, 0, 0});

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
    mail_box_->Subscribe(&DelayedOpenStrategy::CloseMarketNear, this);

  }

  void BeforeTrading(const BeforeTradingAtom&,
                     const TradingTime& trading_time) {}

  void BeforeCloseMarket(const BeforeCloseMarketAtom&) {}

  void CloseMarketNear(const CloseMarketNearAtom&) {
    BOOST_LOG(log_) 
          << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
      << "收到临近收市事件:\n";
    auto instruments = portfolio_.InstrumentList();
    for (const auto& instrument : instruments) {
      ProcessRiskCloseOrderWhenCloseMarketNear(instrument,
                                               OrderDirection::kBuy);
      ProcessRiskCloseOrderWhenCloseMarketNear(instrument,
                                               OrderDirection::kSell);
    }

  }

  void ProcessRiskCloseOrderWhenCloseMarketNear(const std::string& instrument,
                                                OrderDirection direction) {
    int master_qty = master_portfolio_.UnfillCloseQty(instrument, direction);
    int qty = portfolio_.UnfillCloseQty(instrument, direction);
    int force_close_qty = qty - master_qty;
    int leaves_force_close_qty = force_close_qty;
    if (force_close_qty > 0) {
      std::vector<std::shared_ptr<OrderField>> orders =
          portfolio_.UnfillCloseOrders(instrument, direction);
      std::sort(orders.begin(), orders.end(),
                [direction](const std::shared_ptr<OrderField>& l,
                            const std::shared_ptr<OrderField>& r) {
                  return direction == OrderDirection::kBuy
                             ? l->input_price < r->input_price
                             : l->input_price > r->input_price;
                });
      for (const auto& order : orders) {
        int order_force_close_qty = std::min<int>(order->qty, force_close_qty);
        BOOST_LOG(log_) 
          << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
          << "临近收市撤单:" << order->order_id;
        signal_dispatch_->CancelOrder(order->order_id);
        if (order_force_close_qty < order->qty) {
          std::string order_id = GenerateOrderId();
          BOOST_LOG(log_) 
          << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
            << "临近收市重新平仓:" << order_id;
          signal_dispatch_->CloseOrder(order->instrument_id, std::move(order_id),
                                       order->direction, order->position_effect,
                                       order->input_price,
                                       order->qty - order_force_close_qty);
        }

        leaves_force_close_qty -= order_force_close_qty;
        if (leaves_force_close_qty <= 0) {
          break;
        }
      }
      std::string order_id = GenerateOrderId();
      BOOST_LOG(log_) 
          << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
        << "临近收市强平:" << order_id;
      signal_dispatch_->CloseOrder(
          instrument, std::move(order_id), direction, PositionEffect::kOpen,
          direction == OrderDirection::kBuy ? last_tick_->tick->ask_price1
                                            : last_tick_->tick->bid_price1,
          force_close_qty);
    }
  }

  void HandleTick(const std::shared_ptr<TickData>& tick) {
    last_timestamp_ = tick->tick->timestamp;
    auto it_end = std::find_if(
        pending_delayed_open_order_.begin(), pending_delayed_open_order_.end(),
        [=, &tick](const auto& order) {
          return tick->tick->timestamp <
                 order.timestamp_ + delayed_open_order_by_seconds_ * 1000;
        });

    if (it_end != pending_delayed_open_order_.begin()) {
    BOOST_LOG(log_) << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
      << "延迟开仓订单到期 当前 Tick :" << *(tick->instrument) << ":"
      << " Last:" << tick->tick->last_price << "(" << tick->tick->qty << ")"
      << " Bid1:" << tick->tick->bid_price1 << "(" << tick->tick->bid_qty1 << ")"
      << " Ask1:" << tick->tick->ask_price1 << "(" << tick->tick->ask_qty1 << ")";

    for (auto it = pending_delayed_open_order_.begin(); it != it_end; ++it) {
      signal_dispatch_->OpenOrder(it->instrument_, GenerateOrderId(),
                                  it->order_direction_, it->price_, it->qty_);
    }
    pending_delayed_open_order_.erase(pending_delayed_open_order_.begin(),
                                      it_end);

    }
    last_tick_ = tick;
  }

  void HandleInitPosition(const std::vector<OrderPosition>& quantitys) {}

  void HandleOrder(const std::shared_ptr<OrderField>& order) {
    last_timestamp_ = order->update_timestamp;
    BOOST_LOG(log_) << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
      << "RECV: " << " ("
      << StringifyPositionEffect(order->position_effect) << ") "
      << "Order Event \"" << order->order_id << "\": "
      << order->input_price << "("
       << order->leaves_qty << "/" << order->qty << ")";
    portfolio_.HandleOrder(order);
    signal_dispatch_->RtnOrder(order);
  }

  void HandleHistoryOrder(
      const std::vector<std::shared_ptr<const OrderField>>& orders) {
  }
  // CTASignal
  void HandleCTASignalInitPosition(
      const CTASignalAtom&,
      const std::vector<OrderPosition>& quantitys) {}

  void HandleCTASignalHistoryOrder(
      const CTASignalAtom&,
      const std::vector<std::shared_ptr<const OrderField>>& orders) {}

  void HandleCTASignalOrder(const CTASignalAtom& cta_signal_atom,
                            const std::shared_ptr<OrderField>& order) {
    last_timestamp_ = order->update_timestamp;
    BOOST_LOG(log_) << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
      << "RECV: " << "__CTA__" << " ("
      << StringifyPositionEffect(order->position_effect) << ") "
      << "Order Event \"" << order->order_id << "\": "
      << order->input_price << "("
       << order->leaves_qty << "/" << order->qty << ")";
    master_portfolio_.HandleOrder(order);
    signal_dispatch_->RtnOrder(order);

  }

  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_id,
                         OrderDirection direction,
                         double price,
                         int quantity) override {
    BOOST_LOG(log_) << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
      << "持仓信息:" << portfolio_.GetPositionQty(instrument, OrderDirection::kBuy) << "(" << StringifyOrderDirection(OrderDirection::kBuy) << ")"
      << portfolio_.GetPositionQty(instrument, OrderDirection::kSell) << "(" << StringifyOrderDirection(OrderDirection::kSell) << ")";

    BOOST_LOG(log_) << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
    << "发送开仓指令:" << "<" << order_id << ">"
      << "<" << StringifyOrderDirection(direction) << ">"
      << price << "(" << quantity << ")";
    mail_box_->Send(InputOrder{instrument, order_id, slave_account_id_,
                               PositionEffect::kOpen, direction, price,
                               quantity, 0});

  }

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_id,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          double price,
                          int quantity) override {
    BOOST_LOG(log_) << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
      << "持仓信息:" << portfolio_.GetPositionQty(instrument, OrderDirection::kBuy) << "(" << StringifyOrderDirection(OrderDirection::kBuy) << ")"
      << portfolio_.GetPositionQty(instrument, OrderDirection::kSell) << "(" << StringifyOrderDirection(OrderDirection::kSell) << ")";

    BOOST_LOG(log_) << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
    << "发送平仓指令:" << "<" << order_id << ">"
      << "<" << StringifyOrderDirection(direction) << ">"
      << "<" << StringifyPositionEffect(position_effect) << ">"
      << price << "(" << quantity << ")";
    BOOST_ASSERT(portfolio_.GetPositionCloseableQty(
                     instrument, OppositeOrderDirection(direction)) >=
                 quantity);
    portfolio_.HandleNewInputCloseOrder(instrument, direction, quantity);
    mail_box_->Send(InputOrder{instrument, order_id, slave_account_id_,
                               position_effect, direction, price, quantity, 0});
  }

  virtual void CancelOrder(const std::string& order_id) override {
    BOOST_LOG(log_) << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
    << "发送撤单指令:" << "<" << order_id << ">";
    mail_box_->Send(CancelOrderSignal{slave_account_id_, order_id});
  }

  // CTASignalObserver
  virtual void HandleOpening(
      const std::shared_ptr<const OrderField>& order_data) override {
    if (order_data->strategy_id != master_account_id_) {
      return;
    }

    int qty = master_portfolio_.PositionQty(
        order_data->instrument_id,
        OppositeOrderDirection(order_data->direction));
    if (qty != 0) {
      int slave_qty =
          portfolio_.PositionQty(order_data->instrument_id,
                                 OppositeOrderDirection(order_data->direction));

      if (slave_qty > 0) {
        signal_dispatch_->OpenOrder(
            order_data->instrument_id, GenerateOrderId(), order_data->direction,
            order_data->input_price, std::min<int>(order_data->qty, slave_qty));
      }
    }
  }

  virtual void HandleCloseing(
      const std::shared_ptr<const OrderField>& order_data) override {
    if (order_data->strategy_id != master_account_id_) {
      return;
    }
    int qty = portfolio_.GetPositionCloseableQty(
        order_data->instrument_id,
        OppositeOrderDirection(order_data->direction));
    if (qty > 0) {
      signal_dispatch_->CloseOrder(
          order_data->instrument_id, GenerateOrderId(), order_data->direction,
          order_data->position_effect, order_data->input_price,
          std::min<int>(order_data->qty, qty));
    }
    // auto positions = portfolio_.positions();
    // if (positions.find(order_data->instrument_id) != positions.end()) {
    //  const auto& position = positions.at(order_data->instrument_id);
    //  int qty = order_data->direction == OrderDirection::kBuy
    //                ? position.short_qty()
    //                : position.long_qty();
    //  if (qty > 0) {
    //    signal_dispatch_->CloseOrder(
    //        order_data->instrument_id, GenerateOrderId(),
    //        order_data->direction, order_data->position_effect,
    //        order_data->input_price, std::min<int>(order_data->qty, qty));
    //  }
    //}
  }

  virtual void HandleCanceled(
      const std::shared_ptr<const OrderField>& order_data) override {
    if (order_data->strategy_id != master_account_id_) {
      return;
    }

    pending_delayed_open_order_.clear();
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

    // CancelAndReopeningOrder(order_data->instrument_id, position_direction);

    if (qty == 0) {
      auto orders = portfolio_.UnfillOpenOrders(order_data->instrument_id,

                                                position_direction);
      std::for_each(orders.begin(), orders.end(), [=](const auto& order) {
        signal_dispatch_->CancelOrder(order->order_id);
      });
      auto remove_it = std::remove_if(
          pending_delayed_open_order_.begin(),
          pending_delayed_open_order_.end(),
          [order_data,
           position_direction](const InputOrderSignal& pending_input_order) {
            return pending_input_order.instrument_ ==
                       order_data->instrument_id &&
                   pending_input_order.order_direction_ == position_direction;

          });
      pending_delayed_open_order_.erase(remove_it,
                                        pending_delayed_open_order_.end());

    } else {
      int leaves_opening_qty = master_portfolio_.PositionQty(
          order_data->instrument_id, position_direction);
      int leaves_cancel_qty =
          PendingOpenQty(order_data->instrument_id, position_direction) +
          portfolio_.UnfillOpenQty(order_data->instrument_id,
                                   position_direction) -
          leaves_opening_qty;

      leaves_cancel_qty = RemovePendingInputOrder(
          order_data->instrument_id, position_direction, leaves_cancel_qty);
      CancelUnfillOpeningOrders(order_data->instrument_id, position_direction,
                                leaves_cancel_qty);
    }
  }

  void CancelUnfillOpeningOrders(const std::string& instrument,
                                 OrderDirection position_direction,
                                 int leaves_cancel_qty) {
    auto orders = portfolio_.UnfillOpenOrders(instrument, position_direction);
    std::sort(orders.begin(), orders.end(), [](const auto& l, const auto& r) {
      return l->input_price < r->input_price;
    });

    auto for_each_lambda = [=, &leaves_cancel_qty](const auto& order) {
      if (leaves_cancel_qty <= 0) {
        return;
      }
      int cancel_qty = std::min<int>(order->leaves_qty, leaves_cancel_qty);
      signal_dispatch_->CancelOrder(order->order_id);

      int open_qty = order->leaves_qty - cancel_qty;
      if (open_qty > 0) {
        signal_dispatch_->OpenOrder(order->instrument_id, GenerateOrderId(),
                                    position_direction, order->input_price,
                                    open_qty);
      }
      leaves_cancel_qty -= cancel_qty;
    };

    if (position_direction == OrderDirection::kBuy) {
      std::for_each(orders.begin(), orders.end(), for_each_lambda);
    } else {
      std::for_each(orders.rbegin(), orders.rend(), for_each_lambda);
    }
  }

  virtual void HandleOpened(
      const std::shared_ptr<const OrderField>& order_data) override {
    if (order_data->strategy_id != master_account_id_) {
      return;
    }

    OrderDirection position_direction =
        OppositeOrderDirection(order_data->direction);
    int opposite_qty = master_portfolio_.PositionQty(order_data->instrument_id,
                                                     position_direction);
    if (opposite_qty > 0) {
      int qty = master_portfolio_.PositionQty(order_data->instrument_id,
                                              order_data->direction);
      if (opposite_qty == qty) {
        auto orders = portfolio_.UnfillOpenOrders(order_data->instrument_id,
                                                  position_direction);
        std::for_each(orders.begin(), orders.end(), [=](const auto& order) {
          signal_dispatch_->CancelOrder(order->order_id);
        });
      } else {
        int leaves_opening_qty =
            master_portfolio_.UnfillOpenQty(order_data->instrument_id,
                                            position_direction) +
            master_portfolio_.PositionQty(order_data->instrument_id,
                                          position_direction) -
            master_portfolio_.UnfillOpenQty(order_data->instrument_id,
                                            order_data->direction) -
            master_portfolio_.PositionQty(order_data->instrument_id,
                                          order_data->direction);

        int leaves_cancel_qty =
            portfolio_.UnfillOpenQty(order_data->instrument_id,
                                     position_direction) -
            leaves_opening_qty;

        auto orders = portfolio_.UnfillOpenOrders(order_data->instrument_id,
                                                  position_direction);
        std::sort(orders.begin(), orders.end(),
                  [](const auto& l, const auto& r) {
                    return l->input_price < r->input_price;
                  });

        auto for_each_lambda = [=, &leaves_cancel_qty](const auto& order) {
          if (leaves_cancel_qty <= 0) {
            return;
          }
          int cancel_qty = std::min<int>(order->leaves_qty, leaves_cancel_qty);
          signal_dispatch_->CancelOrder(order->order_id);

          int open_qty = order->leaves_qty - cancel_qty;
          if (open_qty > 0) {
            signal_dispatch_->OpenOrder(order->instrument_id, GenerateOrderId(),
                                        position_direction, order->input_price,
                                        open_qty);
          }
          leaves_cancel_qty -= cancel_qty;
        };

        if (position_direction == OrderDirection::kBuy) {
          std::for_each(orders.begin(), orders.end(), for_each_lambda);
        } else {
          std::for_each(orders.rbegin(), orders.rend(), for_each_lambda);
        }
      }
    } else {
      BOOST_LOG(log_) << boost::log::add_value("quant_timestamp", TimeStampToPtime(last_timestamp_))
        << "加入延迟开仓队列 延迟时间:" << delayed_open_order_by_seconds_ << "(秒)"
        << "CTA订单号:" << order_data->order_id
        << " 价格:" << order_data->trading_price
        << " 数量:" << order_data->trading_qty
        << " 方向:" << StringifyOrderDirection(order_data->direction);

      pending_delayed_open_order_.push_back(InputOrderSignal{
          order_data->instrument_id, "", "S1", PositionEffect::kOpen,
          order_data->direction, order_data->trading_price,
          order_data->trading_qty, order_data->update_timestamp});
    }
  }

 private:
  int PendingOpenQty(const std::string& instrument,
                     OrderDirection order_direction) {
    return std::accumulate(
        pending_delayed_open_order_.begin(), pending_delayed_open_order_.end(),
        0,
        [&instrument, order_direction](int val,
                                       const InputOrderSignal& input_order) {
          if (input_order.instrument_ != instrument ||
              input_order.order_direction_ != order_direction) {
            return val;
          }
          return val + input_order.qty_;
        });
  }

  int RemovePendingInputOrder(const std::string& instrument,
                              OrderDirection direction,
                              int remove_qty) {
    if (remove_qty <= 0)
      return remove_qty;
    int leaves_qty = remove_qty;
    pending_delayed_open_order_.sort([direction](const auto& l, const auto& r) {
      return direction == OrderDirection::kBuy ? (l.price_ < r.price_)
                                               : (l.price_ > r.price_);
    });
    for (auto& input_order : pending_delayed_open_order_) {
      if (input_order.instrument_ != instrument ||
          input_order.order_direction_ != direction) {
        continue;
      }
      int minus_qty = std::min<int>(input_order.qty_, leaves_qty);
      input_order.qty_ -= minus_qty;
      leaves_qty -= minus_qty;
      if (leaves_qty <= 0)
        break;
    }

    auto remove_it = std::remove_if(
        pending_delayed_open_order_.begin(), pending_delayed_open_order_.end(),
        [](const auto& input_order) { return input_order.qty_ == 0; });

    pending_delayed_open_order_.erase(remove_it,
                                      pending_delayed_open_order_.end());

    pending_delayed_open_order_.sort([](const auto& l, const auto& r) -> bool {
      return l.timestamp_ < r.timestamp_;
    });
    return leaves_qty;
  }
  std::string GenerateOrderId() {
    return boost::lexical_cast<std::string>(order_id_seq_++);
  }

  std::string StringifyOrderStatus(OrderStatus status) const
  {
    std::string ret = "Unkown";
    switch (status) {
    case OrderStatus::kActive:
      ret = "Action";
      break;
    case OrderStatus::kAllFilled:
      ret = "AllFilled";
      break;
    case OrderStatus::kCanceled:
      ret = "Canceled";
      break;
    case OrderStatus::kInputRejected:
      ret = "InputRejected";
      break;
    case OrderStatus::kCancelRejected:
      ret = "CancelRejected";
      break;
    default:
      break;
    }
    return ret;
  }



  std::string StringifyPositionEffect(PositionEffect position_effect) const
  {
    std::string ret = "Unkown";
    switch (position_effect) {
    case PositionEffect::kUndefine:
      ret = "Undefine";
      break;
    case PositionEffect::kOpen:
      ret = "Open";
      break;
    case PositionEffect::kClose:
      ret = "Close";
      break;
    case PositionEffect::kCloseToday:
      ret = "CloseToday";
      break;
    default:
      break;
    }
    return ret;
  }



  std::string StringifyOrderDirection(OrderDirection order_direction) const {
    std::string ret = "Unkown";
    switch (order_direction) {
    case OrderDirection::kUndefine:
      break;
    case OrderDirection::kBuy:
      ret = "Buy";
      break;
    case OrderDirection::kSell:
      ret = "Sell";
      break;
    default:
      break;
    }
    return ret;
  }

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
  std::shared_ptr<TickData> last_tick_;
  boost::log::sources::logger log_;
  TimeStamp last_timestamp_ = 0;
};

#endif  // FOLLOW_STRATEGY_DELAYED_OPEN_STRATEGY_H
