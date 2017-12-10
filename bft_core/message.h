#ifndef BFT_CORE_MESSAGE_H
#define BFT_CORE_MESSAGE_H
#include <typeindex>
#include "caf/all.hpp"

namespace bft {
class Message {
 public:
  template <typename... Ts>
  Message(Ts&&... args) : type_index_(typeid(std::tuple<std::decay_t<Ts>...>)) {
    message_ = caf::make_message(std::forward<Ts>(args)...);
  }

  std::type_index TypeIndex() const noexcept;

  const caf::message& caf_message() const;

 private:
  std::type_index type_index_;
  caf::message message_;
};
}  // namespace bft
#endif  // BFT_CORE_MESSAGE_H
