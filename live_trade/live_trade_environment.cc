#include "live_trade_environment.h"
#include <boost/assert.hpp>

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<bft::Message>)

void LiveTradeEnvironment::Subscribe(std::type_index type_index,
                                     caf::actor actor) {
  actors_.insert({type_index, actor});
}

void LiveTradeEnvironment::Send(const std::shared_ptr<bft::Message>& message) {
  if (actors_.find(message->TypeIndex()) != actors_.end()) {
    anon_send(actors_.at(message->TypeIndex()), message);
  } else {
    BOOST_ASSERT(false);
  }
}
