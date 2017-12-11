#include "message.h"

namespace bft {

Message::~Message()
{

}
std::type_index bft::Message::TypeIndex() const noexcept {
  return type_index_;
}

caf::message& Message::caf_message() {
  return message_;
}

}  // namespace bft
