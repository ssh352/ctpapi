#ifndef BFT_CORE_MESSAGE_HANDLER_H
#define BFT_CORE_MESSAGE_HANDLER_H
#include "bft_core/message.h"

namespace bft {
class BasedMessageHandler {
 public:
  virtual void ApplyMessage(const Message& msg) = 0;
  virtual ~BasedMessageHandler() {}
};

template <typename... Ts>
class MessageHandler : public BasedMessageHandler {
 public:
  MessageHandler(std::function<void(const Ts&...)> fn) : fn_(fn) {}
  virtual void ApplyMessage(const Message& msg) override {
    ApplyImpl(msg, std::make_index_sequence<sizeof...(Ts)>());
  }

 private:
  template <size_t... Is>
  void ApplyImpl(const Message& msg, std::index_sequence<Is...>) {
    fn_(Get<Is>(msg)...);
  }
  template <size_t N>
  const auto& Get(const Message& msg) {
    using T =
        typename std::tuple_element<N, std::tuple<std::decay_t<Ts>...>>::type;
    return *reinterpret_cast<const T*>(msg.Get(N));
  }
  std::function<void(const Ts&...)> fn_;
};
}  // namespace bft

#endif  // BFT_CORE_MESSAGE_HANDLER_H
