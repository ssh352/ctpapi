#ifndef STRATEGY_UNITTEST_STRATEGY_FIXTURE_H
#define STRATEGY_UNITTEST_STRATEGY_FIXTURE_H
#include <memory>
#include <unordered_map>
#include <typeindex>
#include <boost/any.hpp>
#include <boost/optional.hpp>
#include <boost/log/sources/logger.hpp>
#include "gtest/gtest.h"
#include "common/api_struct.h"
#include "bft_core/channel_delegate.h"
#include "bft_core/make_message.h"
#include "caf_common/caf_atom_defines.h"

class UnittestMailBox : public bft::ChannelDelegate {
 public:
  UnittestMailBox() {}

  virtual void Subscribe(bft::MessageHandler handler) {
    handler_ = handler_.or_else(handler.message_handler());
  }

  virtual void Send(bft::Message message) {
    handler_(message.caf_message());
  }
 private:
   caf::message_handler handler_;
};

class StrategyFixture : public testing::Test {
 public:
  StrategyFixture(std::string account_id) : account_id_(std::move(account_id)) {
    bft::MessageHandler handler;
    handler.Subscribe(&StrategyFixture::HandleInputOrder, this);
    handler.Subscribe(&StrategyFixture::HandleCancelOrder, this);
    handler.Subscribe(&StrategyFixture::HandleActionOrder, this);
    mail_box_.Subscribe(std::move(handler));
  }

  template <typename T, typename... Args>
  void CreateStrategy(Args... args) {
    // strategy_ = std::make_shared<T>(&mail_box_,
    // std::forward<Args...>(args...));
    strategy_ = std::make_shared<T>(&mail_box_, args...);
  }

  template <typename... Args>
  void Send(Args&&... args) {
    mail_box_.Send(bft::MakeMessage(std::forward<Args>(args)...));
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

  void HandleCancelOrder(const CancelOrder& signal);

  void HandleActionOrder(const OrderAction& action_order);

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

  void SendAndClearPendingReplyRtnOrder();

 protected:
  std::unordered_map<std::string, std::shared_ptr<OrderField>>
      order_containter_;
  UnittestMailBox mail_box_;
  boost::any strategy_;
  mutable std::list<boost::any> event_queues_;
  int now_timestamp_ = 0;
  bool auto_reply_new_rtn_order = true;
  std::list<std::shared_ptr<OrderField>> pending_reply_new_rtn_orders_;
  std::string account_id_;
  boost::log::sources::logger log_;
};

#endif  // STRATEGY_UNITTEST_STRATEGY_FIXTURE_H
