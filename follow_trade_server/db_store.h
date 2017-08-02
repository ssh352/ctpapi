#ifndef FOLLOW_TRADE_SERVER_DB_STORE_H
#define FOLLOW_TRADE_SERVER_DB_STORE_H
#include "caf/all.hpp"
#include "sqlite/sqlite3.h"

class DBStore : public caf::event_based_actor {
 public:
   DBStore(caf::actor_config& cfg);

   void CreateOrdersTableIfNotExists();

   virtual caf::behavior make_behavior() override;

   void CreateStrategyRtnOrderIfNotExists();

private:
  void CreateStrategyOrderIDTableIfNotExists();
  sqlite3* db_;
};

#endif  // FOLLOW_TRADE_SERVER_DB_STORE_H
