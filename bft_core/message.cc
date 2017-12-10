#include "message.h"

namespace bft {

Message::~Message()
{

}
std::type_index bft::Message::TypeIndex() const noexcept {
  return type_index_;
}

const caf::message& Message::caf_message() const {
  return message_;
}

}  // namespace bft
