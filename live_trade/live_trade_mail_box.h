#ifndef LIVE_TRADE_LIVE_TRADE_MAIL_BOX_H
#define LIVE_TRADE_LIVE_TRADE_MAIL_BOX_H
#include <unordered_map>
#include <typeindex>

class LiveTradeMailBox {
 public:
  void Subscribe(std::type_index type_id, caf::actor actor) {
    observers_.insert({type_id, actor});
  }

  template <typename... Ts>
  void Send(Ts&&... args) {
    auto it = observers_.find(typeid(std::tuple<std::decay_t<Ts>...>));
    if (it != observers_.end()) {
      caf::anon_send(it->second, std::forward<Ts>(args)...);
    } else {
      BOOST_ASSERT(false);
    }
  }

 private:
  std::unordered_map<std::type_index, caf::actor> observers_;
};

#endif // LIVE_TRADE_LIVE_TRADE_MAIL_BOX_H
