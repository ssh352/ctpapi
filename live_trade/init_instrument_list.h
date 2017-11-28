#ifndef LIVE_TRADE_INIT_INSTRUMENT_LIST_H
#define LIVE_TRADE_INIT_INSTRUMENT_LIST_H

#include "caf/all.hpp"

bool InitInstrumenList(caf::actor_system& system,
                       const std::string& server,
                       const std::string& broker_id,
                       const std::string& user_id,
                       const std::string& password);
#endif  // LIVE_TRADE_INIT_INSTRUMENT_LIST_H
