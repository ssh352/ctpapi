#ifndef BFT_CORE_MAKE_MESSAGE_H
#define BFT_CORE_MAKE_MESSAGE_H
#include "bft_core/message.h"
#include "bft_core/detail/message_impl.h"

namespace bft {
template <typename... Ts>
std::shared_ptr<Message> MakeMessage(Ts&&... args) {
  return std::make_shared<detail::MessageImpl<std::decay_t<Ts>...>>(
      std::forward<Ts>(args)...);
}
}  // namespace bft

#endif  // BFT_CORE_MAKE_MESSAGE_H
