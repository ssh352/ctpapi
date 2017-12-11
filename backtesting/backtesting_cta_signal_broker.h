#ifndef BACKTESTING_BACKTESTING_CTA_SIGNAL_BROKER_H
#define BACKTESTING_BACKTESTING_CTA_SIGNAL_BROKER_H
#include "hpt_core/order_util.h"
#include "bft_core/channel_delegate.h"
#include "hpt_core/portfolio.h"
#include "caf_common/caf_atom_defines.h"

class BacktestingCTASignalBroker {
 public:
  BacktestingCTASignalBroker(
      bft::ChannelDelegate* mail_box,
      std::vector<std::pair<std::shared_ptr<CTATransaction>, int64_t>>
          cta_signal_container,
      std::string instrument);

  // struct OrderPosition {
  //   std::string instrument;
  //   OrderDirection order_direction;
  //   int quantity;
  // };

  void BeforeTrading(BeforeTradingAtom,
                     const TradingTime& trading_time);

  void HandleTick(const std::shared_ptr<TickData>& tick);

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

  PositionEffect ParseThostFtdcPosition(int position_effect) const;

  OrderDirection ParseThostFtdcOrderDirection(int order_direction) const;

  void SendOrder(std::shared_ptr<OrderField> order);

  bft::ChannelDelegate* mail_box_;
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
