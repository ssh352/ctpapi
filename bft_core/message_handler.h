#ifndef BFT_CORE_MESSAGE_HANDLER_H
#define BFT_CORE_MESSAGE_HANDLER_H
#include "bft_core/message.h"

namespace bft {
class BasedMessageHandler {
 public:
  virtual void ApplyMessage(const Message& msg) = 0;
  virtual std::type_index TypeIndex() = 0;
  virtual ~BasedMessageHandler() {}
};

template <typename... Ts>
class MessageHandler : public BasedMessageHandler {
 public:
  template <typename C, typename... Args>
  MessageHandler(void (C::*fn)(Args...), C* ptr) {
    fn_ = [ptr,fn](const Args&... args) { (ptr->*fn)(args...); };
  }

  MessageHandler(std::function<void(const Ts&...)> fn) : fn_(fn) {}
  virtual void ApplyMessage(const Message& msg) override {
    ApplyImpl(msg, std::make_index_sequence<sizeof...(Ts)>());
  }

  virtual std::type_index TypeIndex() override {
    return typeid(std::tuple<Ts...>);
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

template <typename C, typename... Args>
std::unique_ptr<BasedMessageHandler> MakeMessageHandler(void (C::*fn)(Args...),
                                                        C* ptr) {
  return std::make_unique<MessageHandler<Args...>>(fn, ptr);
}

}  // namespace bft

#endif  // BFT_CORE_MESSAGE_HANDLER_H
