#ifndef BFT_CORE_MAKE_MESSAGE_H
#define BFT_CORE_MAKE_MESSAGE_H
#include "caf/all.hpp"
#include "bft_core/message.h"
#include "bft_core/detail/message_impl.h"

namespace bft {
template <typename... Ts>
Message MakeMessage(Ts&&... args) {
  return Message(std::forward<Ts>(args)...);
}
}  // namespace bft

#endif  // BFT_CORE_MAKE_MESSAGE_H
