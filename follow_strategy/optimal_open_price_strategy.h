#ifndef FOLLOW_STRATEGY_OPTIMAL_OPEN_PRICE_STRATEGY_H
#define FOLLOW_STRATEGY_OPTIMAL_OPEN_PRICE_STRATEGY_H
#include <boost/log/sources/logger.hpp>
#include <boost/log/common.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/property_tree/ptree.hpp>
#include "follow_strategy/logging_defines.h"
#include "follow_strategy/string_util.h"
#include "follow_strategy/logging_defines.h"
#include "hpt_core/time_util.h"
#include "hpt_core/order_util.h"
#include "hpt_core/simply_portfolio.h"

class OptimalOpenPriceStrategy {
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
    int wait_optimal_open_price_fill_seconds;
    double price_offset;
  };
  OptimalOpenPriceStrategy(
      Delegate* delegate,
      boost::property_tree::ptree* strategy_config,
      boost::log::sources::logger* log);

  void HandleTick(const std::shared_ptr<TickData>& tick);

  void HandleRtnOrder(const std::shared_ptr<OrderField>& rtn_order);

  void HandleCTARtnOrderSignal(const std::shared_ptr<OrderField>& rtn_order,
                               const CTAPositionQty& position_qty);

  void HandleNearCloseMarket();

  void InitPosition(const std::vector<OrderPosition>& positions);

  void HandleOpening(const std::shared_ptr<const OrderField>& rtn_order,
                     const CTAPositionQty& position_qty);

  void HandleOpened(const std::shared_ptr<const OrderField>& rtn_order,
                    const CTAPositionQty& position_qty);

  void OpenOptimalPriceOrder(InputOrder input_order);

  void HandleCloseing(const std::shared_ptr<const OrderField>& rtn_order,
                      const CTAPositionQty& position_qty);

  void HandleClosed(const std::shared_ptr<const OrderField>& rtn_order,
                    const CTAPositionQty& position_qty);

  void HandleCanceled(const std::shared_ptr<const OrderField>& rtn_order,
                      const CTAPositionQty& position_qty);

 private:
  struct OptimalOpenOrder {
    std::string instrument;
    std::string order_id;
    OrderDirection direction;
    double optimal_price;
    double price;
    TimeStamp timestamp;
  };

  void CancelSpecificOpeningOrders(const std::string& instrument,
                                   OrderDirection direction);

  void CancelUnfillOpeningOrders(const std::string& instrument,
                                 OrderDirection direciton,
                                 int leaves_cancel_qty);

  double GetStrategyParamPriceOffset(const std::string& instrument,
                                     const std::string& product_code) const;

  int GetStrategyParamDealyOpenAfterSeconds(
      const std::string& instrument,
      const std::string& product_code) const;

  int GetStrategyParamWaitOptimalOpenPriceFillSeconds(
      const std::string& instrument,
      const std::string& product_code) const;

  double GetOptimalOpenPrice(double price,
                             double price_offset,
                             OrderDirection direction) const;

  int PendingOpenQty(const std::string& instrument,
                     OrderDirection position_effect_direction);

  void DecreasePendingOpenOrderQty(const std::string& instrument,
                                   OrderDirection position_effect_direction,
                                   int qty);

  std::vector<InputOrder> GetSpecificOrdersInPendingOpenQueue(
      const std::string& instrument,
      OrderDirection direction);

  void RemoveSpecificPendingOpenOrders(
      const std::string& instrument,
      OrderDirection position_effect_direction);
  std::string GenerateOrderId();

  std::string ParseProductCodeWithInstrument(const std::string& instrument);

  std::list<InputOrder> pending_delayed_open_order_;
  std::list<OptimalOpenOrder> optimal_open_price_orders_;
  std::map<std::string, std::string> cta_to_strategy_closing_order_id_;
  SimplyPortfolio portfolio_;
  StrategyParam default_param_;
  std::unordered_map<std::string, StrategyParam> instrument_params_;
  int order_id_seq_ = 0;
  std::shared_ptr<TickData> last_tick_;
  boost::log::sources::logger* log_;
  TimeStamp last_timestamp_ = 0;
  Delegate* delegate_;
  mutable std::unordered_map<std::string, std::string> instrument_code_cache_;
};

#endif  // FOLLOW_STRATEGY_OPTIMAL_OPEN_PRICE_STRATEGY_H
