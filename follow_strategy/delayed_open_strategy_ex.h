#ifndef FOLLOW_STRATEGY_DELAYED_OPEN_STRATEGY_EX_H
#define FOLLOW_STRATEGY_DELAYED_OPEN_STRATEGY_EX_H
#include <boost/log/sources/logger.hpp>
#include <boost/log/common.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include "follow_strategy/logging_defines.h"
#include "follow_strategy/string_util.h"
#include "follow_strategy/logging_defines.h"
#include "hpt_core/time_util.h"
#include "hpt_core/order_util.h"
#include "hpt_core/simply_portfolio.h"

class DelayedOpenStrategyEx {
 public:
  class Delegate {
   public:
    virtual void HandleEnterOrder(InputOrder input_order,
                                  OrderDirection position_effect_direction) = 0;
    virtual void HandleCancelOrder(
        const std::string& instrument,
        const std::string& order_id,
        OrderDirection position_effect_direction) = 0;
    virtual void HandleActionOrder(
        OrderAction action_order,
        OrderDirection position_effect_direction) = 0;
  };

  struct StrategyParam {
    int delayed_open_after_seconds;
    double price_offset;
  };
  DelayedOpenStrategyEx(
      Delegate* delegate,
      std::unordered_map<std::string, StrategyParam> strategy_params,
      boost::log::sources::logger* log);

  void HandleTick(const std::shared_ptr<TickData>& tick);

  void HandleRtnOrder(const std::shared_ptr<OrderField>& rtn_order);

  void HandleCTARtnOrderSignal(const std::shared_ptr<OrderField>& rtn_order,
                               const CTAPositionQty& position_qty);

  void HandleNearCloseMarket();

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
  bool ImmediateOpenOrderIfPriceArrive(const InputOrder& order,
                                       const std::shared_ptr<Tick>& tick);

  void RemoveSpecificPendingOpenOrders(const std::string& instrument,
                                       OrderDirection direction);

  void CancelSpecificOpeningOrders(const std::string& instrument,
                                   OrderDirection direction);

  std::vector<InputOrder> GetSpecificOrdersInPendingOpenQueue(
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

  std::list<InputOrder> pending_delayed_open_order_;
  std::map<std::string, std::string> cta_to_strategy_closing_order_id_;
  SimplyPortfolio portfolio_;
  std::unordered_map<std::string, StrategyParam> strategy_params_;
  int order_id_seq_ = 0;
  std::shared_ptr<TickData> last_tick_;
  boost::log::sources::logger* log_;
  TimeStamp last_timestamp_ = 0;
  Delegate* delegate_;
  double GetStrategyParamPriceOffset(const std::string& instrument) const;
  int GetStrategyParamDealyOpenAfterSeconds(
      const std::string& instrument) const;
};

#endif  // FOLLOW_STRATEGY_DELAYED_OPEN_STRATEGY_EX_H
