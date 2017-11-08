#ifndef FOLLOW_STRATEGY_DELAYED_OPEN_STRATEGY_EX_H
#define FOLLOW_STRATEGY_DELAYED_OPEN_STRATEGY_EX_H
#include <boost/log/sources/logger.hpp>
#include <boost/log/common.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include "follow_strategy/logging_defines.h"
#include "follow_strategy/string_util.h"
#include "follow_strategy/logging_defines.h"
#include "hpt_core/time_util.h"
#include "order_util.h"
#include "simply_portfolio.h"
#include "hpt_core/instrument_mananger.h"

class DelayedOpenStrategyEx {
 public:
  class Delegate {
   public:
    virtual void EnterOrder(InputOrder) = 0;
    virtual void CancelOrder(const std::string& instrument,
                             const std::string& order_id,
                             OrderDirection direction,
                             int cancel_qty) = 0;
  };

  struct StrategyParam {
    int delayed_open_after_seconds;
    double price_offset;
  };
  DelayedOpenStrategyEx(Delegate* delegate,
                        StrategyParam strategy_param);

  void HandleTick(const std::shared_ptr<TickData>& tick);

  void HandleRtnOrder(const std::shared_ptr<OrderField>& rtn_order);

  void HandleCTARtnOrderSignal(const std::shared_ptr<OrderField>& rtn_order,
                               const CTAPositionQty& position_qty);

  /*void HandleCTACloseing(auto order) {
    int qty = context_->GetCloseableQty(order->instrument, order->direction);
    if (qty > 0) {
      CloseOrder(order_data->instrument_id, GenerateOrderId(),
                 order_data->direction, order_data->position_effect,
                 order_data->input_price, std::min<int>(order_data->qty, qty));
    }
  }
  void HandleCTAClosed(auto order) {
    if (order->position_qty == 0) {
      RemoveInputOrderInPendingDelayQueue(order->instrument, order->direction);
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

  void HandleCTACanceled() {
    // TODO: Do something
  }

*/
  void HandleOpening(const std::shared_ptr<const OrderField>& rtn_order,
                     const CTAPositionQty& position_qty);

  void HandleOpened(const std::shared_ptr<const OrderField>& rtn_order,
                    const CTAPositionQty& position_qty);

  void HandleCloseing(const std::shared_ptr<const OrderField>& rtn_order,
                      const CTAPositionQty& position_qty);

  void HandleClosed(const std::shared_ptr<const OrderField>& rtn_order,
                    const CTAPositionQty& position_qty);

  void HandleCanceled(const std::shared_ptr<const OrderField>& rtn_order,
                      const CTAPositionQty& position_qty);

 private:
  bool ImmediateOpenOrderIfPriceArrive(const InputOrderSignal& order,
                                       const std::shared_ptr<Tick>& tick);

  void RemoveSpecificPendingOpenOrders(const std::string& instrument,
                                       OrderDirection direction);

  void CancelSpecificOpeningOrders(const std::string& instrument,
                                   OrderDirection direction);

  std::vector<InputOrderSignal> GetSpecificOrdersInPendingOpenQueue(
      const std::string& instrument,
      OrderDirection direction);

  void DecreasePendingOpenOrderQty(const std::string& instrument,
                                   OrderDirection direction,
                                   int qty);

  void CancelUnfillOpeningOrders(const std::string& instrument,
                                 OrderDirection direciton,
                                 int leaves_cancel_qty);

  int PendingOpenQty(const std::string& instrument,
                     OrderDirection order_direction);

  std::string GenerateOrderId();

  std::list<InputOrderSignal> pending_delayed_open_order_;
  std::map<std::string, std::string> cta_to_strategy_closing_order_id_;
  SimplyPortfolio portfolio_;
  StrategyParam strategy_param_;
  int order_id_seq_ = 0;
  std::shared_ptr<TickData> last_tick_;
  boost::log::sources::logger log_;
  TimeStamp last_timestamp_ = 0;
  Delegate* delegate_;
};

template <typename MailBox>
class DelayOpenStrategyAgent : public DelayedOpenStrategyEx::Delegate {
 public:
  DelayOpenStrategyAgent(MailBox* mail_box,
                         DelayedOpenStrategyEx::StrategyParam param)
      : mail_box_(mail_box),
        strategy_(this, std::move(param)) {
    mail_box_->Subscribe(&DelayOpenStrategyAgent::HandleCTARtnOrderSignal,
                         this);
    mail_box_->Subscribe(&DelayedOpenStrategyEx::HandleTick, &strategy_);
    mail_box_->Subscribe(&DelayOpenStrategyAgent::HandleRtnOrder, this);
  }

  void HandleCTARtnOrderSignal(const std::shared_ptr<OrderField>& rtn_order,
                               const CTAPositionQty& position_qty) {
    if (waiting_reply_order_.empty() ||
        std::find_if(waiting_reply_order_.begin(), waiting_reply_order_.end(),
                     [&rtn_order](const auto& item) {
                       return item.instrument == rtn_order->instrument_id &&
                              item.direction == rtn_order->direction;
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
                     cta_signal.first->direction == rtn_order->direction;
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

  virtual void EnterOrder(InputOrder order) override {
    waiting_reply_order_.push_back(
        OutstandingRequest{order.instrument, order.order_id, order.direction,
                           OutStandingEvent::kInputOrder});
    mail_box_->Send(std::move(order));
  }

  virtual void CancelOrder(const std::string& instrument,
                           const std::string& order_id,
                           OrderDirection direction,
                           int cancel_qty) override {
    waiting_reply_order_.push_back(OutstandingRequest{
        instrument, order_id, direction, OutStandingEvent::kCancelOrder});
    mail_box_->Send(CancelOrderSignal{order_id, instrument, cancel_qty});
  }

 private:
  enum class OutStandingEvent {
    kInputOrder,
    kCancelOrder,
  };

  struct OutstandingRequest {
    std::string instrument;
    std::string order_id;
    OrderDirection direction;
    OutStandingEvent event_type;
  };

  MailBox* mail_box_;
  DelayedOpenStrategyEx strategy_;

  std::vector<OutstandingRequest> waiting_reply_order_;
  std::list<std::pair<std::shared_ptr<OrderField>, CTAPositionQty> >
      pending_cta_signal_queue_;
};
#endif  // FOLLOW_STRATEGY_DELAYED_OPEN_STRATEGY_EX_H
