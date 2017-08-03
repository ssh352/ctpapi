#ifndef FOLLOW_TRADE_SERVER_DB_STORE_H
#define FOLLOW_TRADE_SERVER_DB_STORE_H
#include "caf/all.hpp"
#include "sqlite/sqlite3.h"

class DBStore : public caf::event_based_actor {
 public:
  DBStore(caf::actor_config& cfg);

  virtual caf::behavior make_behavior() override;

 private:
  void CreateOrdersTableIfNotExists();
  void CreateStrategyRtnOrderIfNotExists();
  void CreateStrategyOrderIDTableIfNotExists();
  void CreateStrategyPositionIfNotExists();

  sqlite3* db_;
};

#endif  // FOLLOW_TRADE_SERVER_DB_STORE_H
