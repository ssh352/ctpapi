#ifndef FOLLOW_STRATEGY_DELAY_OPEN_STRATEGY_AGENT_H
#define FOLLOW_STRATEGY_DELAY_OPEN_STRATEGY_AGENT_H
#include <boost/property_tree/ptree.hpp>
#include "delayed_open_strategy_ex.h"
#include "product_info_manager.h"
#include "bft_core/channel_delegate.h"
#include "live_trade/live_trade_system.h"
#include "bft_core/make_message.h"

template <typename Strategy>
class DelayOpenStrategyAgent : public Strategy::Delegate {
 public:
  DelayOpenStrategyAgent(bft::ChannelDelegate* mail_box,
                         boost::property_tree::ptree* strategy_config,
                         ProductInfoMananger* product_info_mananger,
                         boost::log::sources::logger* log)
      : mail_box_(mail_box),
        strategy_(this, strategy_config, product_info_mananger, log) {
    bft::MessageHandler message_handler;
    message_handler.Subscribe(&DelayOpenStrategyAgent::HandleCTARtnOrderSignal,
                             this);
    message_handler.Subscribe(&Strategy::HandleExchangeStatus, &strategy_);
    message_handler.Subscribe(&Strategy::HandleTick, &strategy_);
    message_handler.Subscribe(&Strategy::InitPosition, &strategy_);
    message_handler.Subscribe(&DelayOpenStrategyAgent::HandleNearCloseMarket,
                             this);
    message_handler.Subscribe(&DelayOpenStrategyAgent::HandleRtnOrder, this);
    mail_box_->Subscribe(std::move(message_handler));
  }

  void HandleCTARtnOrderSignal(const std::shared_ptr<OrderField>& rtn_order,
                               const CTAPositionQty& position_qty) {
    if (waiting_reply_order_.empty() ||
        std::find_if(waiting_reply_order_.begin(), waiting_reply_order_.end(),
                     [&rtn_order](const auto& item) {
                       return item.instrument == rtn_order->instrument_id &&
                              item.position_effect_direction ==
                                  rtn_order->position_effect_direction;
                     }) == waiting_reply_order_.end()) {
      strategy_.HandleCTARtnOrderSignal(rtn_order, position_qty);
    } else {
      pending_cta_signal_queue_.emplace_back(rtn_order, position_qty);
    }
  }

  void HandleRtnOrder(const std::shared_ptr<OrderField>& rtn_order) {
    strategy_.HandleRtnOrder(rtn_order);
    if (!waiting_reply_order_.empty()) {
      auto it = std::find_if(
          waiting_reply_order_.begin(), waiting_reply_order_.end(),
          [&](const auto& i) { return i.order_id == rtn_order->order_id; });

      if (it != waiting_reply_order_.end()) {
        std::list<std::pair<std::shared_ptr<OrderField>, CTAPositionQty> >
            try_handing_signals;
        auto it_erase = std::remove_if(
            pending_cta_signal_queue_.begin(), pending_cta_signal_queue_.end(),
            [&rtn_order](const auto& cta_signal) {
              return cta_signal.first->instrument_id ==
                         rtn_order->instrument_id &&
                     cta_signal.first->position_effect_direction ==
                         rtn_order->position_effect_direction;
            });
        std::copy(it_erase, pending_cta_signal_queue_.end(),
                  std::back_inserter(try_handing_signals));
        pending_cta_signal_queue_.erase(it_erase,
                                        pending_cta_signal_queue_.end());
        waiting_reply_order_.erase(it);
        std::for_each(try_handing_signals.begin(), try_handing_signals.end(),
                      [=](const auto& cta_signal) {
                        HandleCTARtnOrderSignal(cta_signal.first,
                                                cta_signal.second);
                      });
      }
    }
  }

  void HandleNearCloseMarket(const CloseMarketNearAtom&) {
    strategy_.HandleNearCloseMarket();
  }

  virtual void HandleEnterOrder(
      InputOrder order,
      OrderDirection position_effect_direction) override {
    waiting_reply_order_.push_back(OutstandingRequest{
        order.instrument, order.order_id, position_effect_direction, 0.0, 0,
        OutStandingEvent::kInputOrder});
    mail_box_->Send(bft::MakeMessage(std::move(order)));
  }

  virtual void HandleCancelOrder(
      const std::string& instrument,
      const std::string& order_id,
      OrderDirection position_effect_direction) override {
    waiting_reply_order_.push_back(
        OutstandingRequest{instrument, order_id, position_effect_direction, 0.0,
                           0, OutStandingEvent::kCancelOrder});
    mail_box_->Send(bft::MakeMessage(CancelOrder{order_id, instrument}));
  }

  virtual void HandleActionOrder(
      OrderAction action_order,
      OrderDirection position_effect_direction) override {
    waiting_reply_order_.push_back(OutstandingRequest{
        action_order.instrument, action_order.order_id,
        position_effect_direction, action_order.new_price, action_order.new_qty,
        OutStandingEvent::kModifyPriceOrQtyOrBoth});
    mail_box_->Send(bft::MakeMessage(std::move(action_order)));
  }

 private:
  enum class OutStandingEvent {
    kInputOrder,
    kCancelOrder,
    kModifyPriceOrQtyOrBoth,
  };

  struct OutstandingRequest {
    std::string instrument;
    std::string order_id;
    OrderDirection position_effect_direction;
    double new_price;
    int new_qty;
    OutStandingEvent event_type;
  };

  bft::ChannelDelegate* mail_box_;
  Strategy strategy_;

  std::vector<OutstandingRequest> waiting_reply_order_;
  std::list<std::pair<std::shared_ptr<OrderField>, CTAPositionQty> >
      pending_cta_signal_queue_;
};

#endif  // FOLLOW_STRATEGY_DELAY_OPEN_STRATEGY_AGENT_H
