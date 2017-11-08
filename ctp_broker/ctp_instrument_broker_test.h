#ifndef CTP_BROKER_UNITTEST_CTP_INSTRUMENT_BROKER_TEST_H
#define CTP_BROKER_UNITTEST_CTP_INSTRUMENT_BROKER_TEST_H

#include <boost/optional.hpp>
#include <boost/any.hpp>
#include "gtest/gtest.h"
#include "ctp_instrument_broker.h"

class CTPInstrumentBrokerTest : public testing::Test, public CTPOrderDelegate {
 public:
  CTPInstrumentBrokerTest();
  virtual void EnterOrder(const CTPEnterOrder& enter_order) override;

  virtual void CancelOrder(const CTPCancelOrder& cancel_order) override;

  virtual void ReturnOrderField(
      const std::shared_ptr<OrderField>& order) override;

  template <typename T>
  boost::optional<T> PopupOrder();

  template <typename... Ts>
  std::enable_if_t<sizeof...(Ts) >= 2, boost::optional<std::tuple<Ts...>>>
  PopupOrder() {
    if (event_queues_.empty()) {
      return boost::optional<std::tuple<Ts...>>();
    }
    auto event = event_queues_.front();
    event_queues_.pop_front();
    return boost::any_cast<std::tuple<Ts...>>(event);
  }

  void Clear();

 protected:
  CTPInstrumentBroker broker_;
  std::shared_ptr<CTPOrderField> MakeCTPOrderField(
      const std::string& order_id,
      const std::string& instrument,
      CTPPositionEffect position_effect,
      OrderDirection direction,
      OrderStatus status,
      double price,
      double leaves_qty,
      double traded_qty,
      double qty,
      TimeStamp input_timestamp,
      TimeStamp update_timestamp);

  void MakeOpenOrderRequest(const std::string& order_id,
                            OrderDirection direction,
                            double price,
                            int qty);

  void MakeCloseOrderRequest(const std::string& order_id,
                             OrderDirection direction,
                             double price,
                             int qty);

  void SimulateCTPNewOpenOrderField(const std::string& order_id,
                                    OrderDirection direction,
                                    double price,
                                    int qty);

  void SimulateCTPNewCloseOrderField(
      const std::string& order_id,
      OrderDirection direction,
      double price,
      int qty,
      CTPPositionEffect position_effect = CTPPositionEffect::kClose);

  void SimulateCTPTradedOrderField(const std::string& order_id, int qty);

  void SimulateCTPTradedOrderFieldWithPrice(const std::string& order_id,
                                            double trading_price,
                                            int qty);

 private:
  struct HashCTPOrderField {
    size_t operator()(const std::shared_ptr<CTPOrderField>& order) const;
    size_t operator()(const std::string& order_id) const;
  };

  struct CompareCTPOrderField {
    bool operator()(const std::shared_ptr<CTPOrderField>& l,
                    const std::shared_ptr<CTPOrderField>& r) const;
    bool operator()(const std::string& l,
                    const std::shared_ptr<CTPOrderField>& r) const;
  };
  std::string GenerateOrderId();
  std::list<boost::any> event_queues_;

  boost::unordered_set<std::shared_ptr<CTPOrderField>,
                       HashCTPOrderField,
                       CompareCTPOrderField>
      orders_;
  int order_seq_ = 0;
};

template <typename T>
boost::optional<T> CTPInstrumentBrokerTest::PopupOrder() {
  if (event_queues_.empty()) {
    return boost::optional<T>();
  }
  auto event = event_queues_.front();
  event_queues_.pop_front();
  return boost::any_cast<T>(event);
}

#endif  // CTP_BROKER_UNITTEST_CTP_INSTRUMENT_BROKER_TEST_H
