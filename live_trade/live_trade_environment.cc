#include "live_trade_environment.h"
#include <boost/assert.hpp>

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(bft::Message)

void LiveTradeEnvironment::Subscribe(std::type_index type_index,
                                     caf::actor actor) {
  actors_.insert({type_index, actor});
}

void LiveTradeEnvironment::Send(bft::Message message) {
  auto range = actors_.equal_range(message.TypeIndex());
  if (range.first != range.second) {
    for (auto it = range.first; it != range.second; ++it) {
      caf::anon_send(it->second, message.caf_message());
    }
  } else {
    BOOST_ASSERT(false);
  }
}
