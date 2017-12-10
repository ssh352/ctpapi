#ifndef BFT_CORE_MESSAGE_HANDLER_H
#define BFT_CORE_MESSAGE_HANDLER_H
#include "bft_core/message.h"

namespace bft {

template<typename TL>
struct TypeListToTuple;

template<typename... Ts>
struct TypeListToTuple<caf::detail::type_list<Ts...>> {
  using Type = std::tuple<std::decay_t<Ts>...>;
};

template <typename T>
struct LambdaTrait {
  using fun_trait = typename caf::detail::get_callable_trait<T>::type;
  using ArgsType =
      typename TypeListToTuple<typename fun_trait::arg_types>::Type;
};

class MessageHandler {
 public:
  MessageHandler() = default;
  ~MessageHandler() {}

  template <typename... Ts>
  void Assign(Ts&&... args) {
    type_indexs_ = {typeid(typename LambdaTrait<Ts>::ArgsType)...};
    message_handler_.assign(std::forward<Ts>(args)...);
  }

  template <typename C, typename... Args>
  void Subscribe(void (C::*fn)(Args...), C* ptr) {
    message_handler_ = message_handler_.or_else(
        [fn, ptr](const Args&... args) { (ptr->*fn)(args...); });
    type_indexs_.push_back(typeid(std::tuple<std::decay_t<Args>...>));
  }

  const std::vector<std::type_index>& TypeIndexs() const {
    return type_indexs_;
  }

  caf::message_handler message_handler() const { return message_handler_; }

 private:
  std::vector<std::type_index> type_indexs_;
  caf::message_handler message_handler_;
};
}  // namespace bft

#endif  // BFT_CORE_MESSAGE_HANDLER_H
