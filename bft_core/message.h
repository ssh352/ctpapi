#ifndef BFT_CORE_MESSAGE_H
#define BFT_CORE_MESSAGE_H
#include <typeindex>
#include "caf/all.hpp"
#include "bft_core/core_export.h"

namespace bft {
class CORE_EXPORT Message {
 public:
  Message(Message&&) = default;

  template <typename... Ts>
  Message(Ts&&... args)
      : type_index_(typeid(std::tuple<std::decay_t<Ts>...>)),
        message_(caf::make_message(std::forward<Ts>(args)...)) {

  }

  ~Message();

  std::type_index TypeIndex() const noexcept;

  const caf::message& caf_message() const;

 private:
  class CORE_EXPORT std::type_index;
  class CORE_EXPORT caf::message;
  std::type_index type_index_;
  caf::message message_;
};
}  // namespace bft
#endif  // BFT_CORE_MESSAGE_H
