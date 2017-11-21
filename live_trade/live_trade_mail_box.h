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
    auto range = observers_.equal_range(typeid(std::tuple<std::decay_t<Ts>...>));
    for (auto it = range.first; it != range.second; ++it) {
      caf::anon_send(it->second, args...);
    }
  }

 private:
  std::unordered_multimap<std::type_index, caf::actor> observers_;
};

#endif // LIVE_TRADE_LIVE_TRADE_MAIL_BOX_H
