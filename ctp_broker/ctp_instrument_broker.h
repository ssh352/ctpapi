#ifndef CTP_BROKER_CTP_INSTRUMENT_BROKER_H
#define CTP_BROKER_CTP_INSTRUMENT_BROKER_H
#include <memory>
#include <boost/bimap.hpp>
#include <unordered_set>
#include <unordered_map>
#include <boost/unordered_set.hpp>
#include "common/api_struct.h"
#include "ctp_broker/ctp_order_delegate.h"
#include "ctp_position_effect_flag_strategy.h"
#include "ctp_position_effect_strategy.h"

class CTPInstrumentBroker : public CTPPositionEffectStrategyDelegate {
 public:
  CTPInstrumentBroker(CTPOrderDelegate* delegate,
                      std::string instrument,
                      bool close_today_aware,
                      std::function<std::string(void)> generate_order_id_func);

  void HandleRtnOrder(const std::shared_ptr<CTPOrderField>& order);

  void HandleInputOrder(const InputOrder& order);

  void HandleCancel(const CancelOrderSignal& cancel);

  void HandleTrader(const std::string& order_id,
                    double trading_price,
                    int trading_qty,
                    TimeStamp timestamp);

      void InitPosition(std::pair<int, int> long_pos,
                        std::pair<int, int> short_pos);

  virtual void PosstionEffectStrategyHandleInputOrder(
      const std::string& input_order_id,
      CTPPositionEffect position_effect,
      OrderDirection direction,
      double price,
      int qty) override;

  template <typename T, typename S>
  void SetPositionEffectStrategy() {
    position_effect_strategy_ = std::make_unique<T>(new S(this));
  }

 protected:
  CTPOrderDelegate* order_delegate_;

 private:
  template <typename T>
  struct HashOrderField {
    size_t operator()(const T& order) const {
      return std::hash<std::string>()(order->order_id);
    }
    size_t operator()(const std::string& order_id) const {
      return std::hash<std::string>()(order_id);
    }
  };

  template <typename T>
  struct CompareOrderField {
    bool operator()(const T& l, const T& r) const {
      return l->order_id == r->order_id;
    }
    bool operator()(const std::string& l, const T& r) const {
      return l == r->order_id;
    }
  };

  void InsertOrderField(const std::string& instrument,
                        const std::string& order_id,
                        OrderDirection direciton,
                        PositionEffect position_effect,
                        double price,
                        int qty);

  void BindOrderId(const std::string& order_id,
                   const std::string& ctp_order_id);

  bool IsCtpOpenPositionEffect(CTPPositionEffect position_effect) const;

  std::unordered_multimap<std::string, std::string> ctp_order_id_to_order_id_;

  std::unordered_multimap<std::string, std::string> order_id_to_ctp_order_id_;

  using HashInnerOrderField = HashOrderField<std::unique_ptr<OrderField>>;
  using CompareInnerOrderField = CompareOrderField<std::unique_ptr<OrderField>>;
  boost::unordered_set<std::unique_ptr<OrderField>,
                       HashInnerOrderField,
                       CompareInnerOrderField>
      orders_;

  using HashCTPOrderField = HashOrderField<std::unique_ptr<CTPOrderField>>;
  using CompareCTPOrderField =
      CompareOrderField<std::unique_ptr<CTPOrderField>>;
  boost::unordered_set<std::unique_ptr<CTPOrderField>,
                       HashCTPOrderField,
                       CompareCTPOrderField>
      ctp_orders_;

  std::function<std::string(void)> generate_order_id_func_;

  std::unique_ptr<CTPPositionAmount> long_;
  std::unique_ptr<CTPPositionAmount> short_;
  std::string instrument_;
  std::unique_ptr<CTPPositionEffectStrategy> position_effect_strategy_;
};

#endif  // CTP_BROKER_CTP_INSTRUMENT_BROKER_H
