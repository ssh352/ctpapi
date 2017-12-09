#include "live_trade_environment.h"
#include <boost/assert.hpp>

void LiveTradeEnvironment::Send(const std::shared_ptr<bft::Message>& message) {
  if (actors_.find(message->TypeIndex()) != actors_.end()) {
    //anon_send(actors_.at(message->TypeIndex()), message);
  } else {
    BOOST_ASSERT(false);
  }
}
