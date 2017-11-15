#ifndef LIVE_TRADE_CTP_POSITION_RESTORER_H
#define LIVE_TRADE_CTP_POSITION_RESTORER_H
#include <memory>
#include <map>
#include <vector>
#include <tuple>
#include <boost/unordered_set.hpp>
#include "common/api_struct.h"
#include "ctp_broker/ctp_position_amount.h"

class CtpPositionRestorer {
 public:
  void AddYesterdayPosition(const std::string& instrument,
                            OrderDirection direction,
                            int qty);

  void HandleRtnOrder(const std::shared_ptr<CTPOrderField>& order);

  void HandleTraded(const std::string& order_id,
                    double trading_price,
                    int trading_qty,
                    TimeStamp timestamp);

  std::vector<CTPPositionField> Result() const;

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

  void Unfrozen(
      std::map<std::string, std::unique_ptr<CTPPositionAmount>>* container,
      const std::string& instrument,
      int qty,
      CTPPositionEffect position_effect);

  void Frozen(
      std::map<std::string, std::unique_ptr<CTPPositionAmount>>* container,
      const std::string& instrument,
      int qty,
      CTPPositionEffect position_effect);

  void OpenTraded(
      std::map<std::string, std::unique_ptr<CTPPositionAmount>>* container,
      const std::string& instrument,
      int qty);

  void CloseTraded(
      std::map<std::string, std::unique_ptr<CTPPositionAmount>>* container,
      const std::string& instrument,
      int qty,
      CTPPositionEffect position_effect);

  void MakeCTPPositionAmountIfNeeed(
      std::map<std::string, std::unique_ptr<CTPPositionAmount>>* container,
      const std::string& instrument);

  bool IsCtpOpenPositionEffect(CTPPositionEffect position_effect) const;

  using HashCTPOrderField = HashOrderField<std::unique_ptr<CTPOrderField>>;
  using CompareCTPOrderField =
      CompareOrderField<std::unique_ptr<CTPOrderField>>;
  boost::unordered_set<std::unique_ptr<CTPOrderField>,
                       HashCTPOrderField,
                       CompareCTPOrderField>
      ctp_orders_;

  std::map<std::string, std::unique_ptr<CTPPositionAmount>> long_;
  std::map<std::string, std::unique_ptr<CTPPositionAmount>> short_;
};

#endif  // LIVE_TRADE_CTP_POSITION_RESTORER_H
