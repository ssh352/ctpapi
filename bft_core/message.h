#ifndef BFT_CORE_MESSAGE_H
#define BFT_CORE_MESSAGE_H
#include <typeindex>

namespace bft {
class Message {
 public:
  virtual const void* Get(size_t pos) const noexcept = 0;

  virtual std::type_index TypeIndex() const noexcept = 0;
};
}  // namespace bft
#endif  // BFT_CORE_MESSAGE_H
