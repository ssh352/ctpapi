#ifndef STRATEGY_UNITTEST_STRATEGY_FIXTURE_H
#define STRATEGY_UNITTEST_STRATEGY_FIXTURE_H
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <boost/any.hpp>
#include <boost/optional.hpp>
#include "gtest/gtest.h"
#include "common/api_struct.h"

struct CTASignalAtom {
  static const CTASignalAtom value;
};
struct BeforeTradingAtom {
  static const BeforeTradingAtom value;
};
struct BeforeCloseMarketAtom {
  static const BeforeCloseMarketAtom value;
};

struct CloseMarketNearAtom {
  static const CloseMarketNearAtom value;
};

class UnittestMailBox {
 public:
  UnittestMailBox() {}
  template <typename CLASS, typename... ARG>
  void Subscribe(void (CLASS::*pfn)(ARG...), CLASS* c) {
    std::function<void(ARG...)> fn = [=](ARG... arg) { (c->*pfn)(arg...); };
    subscribers_.insert({typeid(std::tuple<ARG...>), std::move(fn)});
  }

  template <typename... ARG>
  void Send(const ARG&... arg) {
    auto range = subscribers_.equal_range(typeid(std::tuple<const ARG&...>));
    for (auto it = range.first; it != range.second; ++it) {
      boost::any_cast<std::function<void(const ARG&...)>>(it->second)(arg...);
    }
  }

 private:
  std::unordered_multimap<std::type_index, boost::any> subscribers_;
  // std::list<std::function<void(void)>>* callable_queue_;
};

class StrategyFixture : public testing::Test {
 public:
  StrategyFixture() {
    mail_box_.Subscribe(&StrategyFixture::HandleInputOrder, this);
    mail_box_.Subscribe(&StrategyFixture::HandleCancelOrder, this);
  }

  template <typename T, typename... Args>
  void CreateStrategy(Args... args) {
    // strategy_ = std::make_shared<T>(&mail_box_,
    // std::forward<Args...>(args...));
    strategy_ = std::make_shared<T>(&mail_box_, args...);
  }

  template <typename... Args>
  void Send(const Args&... args) {
    mail_box_.Send(args...);
  }

  void Clear() { event_queues_.clear(); }

  template <typename T>
  boost::optional<T> PopupRntOrder() {
    if (event_queues_.empty()) {
      return boost::optional<T>();
    }
    auto event = event_queues_.front();
    event_queues_.pop_front();
    return boost::any_cast<T>(event);
  }

  template <typename... Ts>
  std::enable_if_t<sizeof...(Ts) >= 2, boost::optional<std::tuple<Ts...>>>
  PopupRntOrder() {
    if (event_queues_.empty()) {
      return boost::optional<std::tuple<Ts...>>();
    }
    auto event = event_queues_.front();
    event_queues_.pop_front();
    return boost::any_cast<std::tuple<Ts...>>(event);
  }

  void ElapseSeconds(int seconds) { now_timestamp_ += seconds * 1000; }
  void ElapseMillisecond(int millsec) { now_timestamp_ += millsec; }

  void HandleInputOrder(const InputOrder& input_order);

  void HandleCancelOrder(const CancelOrderSignal& signal);



  std::shared_ptr<OrderField> MakeOrderField(const std::string& account_id,
                                             const std::string& order_id,
                                             const std::string& instrument,
                                             PositionEffect position_effect,
                                             OrderDirection direction,
                                             OrderStatus status,
                                             double price,
                                             double leaves_qty,
                                             double traded_qty,
                                             double qty,
                                             TimeStamp input_timestamp,
                                             TimeStamp update_timestamp);

  std::shared_ptr<OrderField> MakeNewOrder(const std::string& account_id,
                                           const std::string& order_id,
                                           const std::string& instrument,
                                           PositionEffect position_effect,
                                           OrderDirection direction,
                                           double price,
                                           double qty);

  std::shared_ptr<OrderField> MakeTradedOrder(const std::string& account_id,
                                              const std::string& order_id,
                                              double traded_qty);

  std::shared_ptr<OrderField> MakeTradedOrder(const std::string& account_id,
                                              const std::string& order_id,
                                              double trading_price,
                                              double traded_qty);

  std::shared_ptr<OrderField> MakeNewOpenOrder(const std::string& account_id,
                                               const std::string& order_id,
                                               const std::string& instrument,
                                               OrderDirection direction,
                                               double price,
                                               double qty);

  std::shared_ptr<OrderField> MakeNewCloseOrder(
      const std::string& account_id,
      const std::string& order_id,
      const std::string& instrument,
      OrderDirection direction,
      double price,
      double qty,
      PositionEffect position_effect = PositionEffect::kClose);

  std::shared_ptr<OrderField> MakeCanceledOrder(const std::string& account_id,
                                                const std::string& order_id);

 protected:
  std::unordered_map<std::string, std::shared_ptr<OrderField>>
      order_containter_;
  UnittestMailBox mail_box_;
  boost::any strategy_;
  mutable std::list<boost::any> event_queues_;
  int now_timestamp_ = 0;
};

#endif  // STRATEGY_UNITTEST_STRATEGY_FIXTURE_H
