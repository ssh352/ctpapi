#include "db_store.h"
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include "atom_defines.h"
#include "common/api_struct.h"

DBStore::DBStore(caf::actor_config& cfg) : caf::event_based_actor(cfg) {
  /* Open database */
  int rc = sqlite3_open("test.db", &db_);
  if (rc) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db_));
  } else {
    fprintf(stdout, "Opened database successfully\n");
  }

  CreateOrdersTableIfNotExists();
  CreateStrategyRtnOrderIfNotExists();
  CreateStrategyOrderIDTableIfNotExists();
}

void DBStore::CreateOrdersTableIfNotExists() {
  /* Create SQL statement */
  const char* sql =
      R"(
  CREATE
      TABLE IF NOT EXISTS Orders
      (
          AccountID VARCHAR(50) NOT NULL,
          InstrumentID VARCHAR(50) NOT NULL,
          OrderID VARCHAR(50) NOT NULL,
          Status INT NOT NULL,
          Price DOUBLE NULL,
          AvgPrice DOUBLE NULL,
          Qty INT NOT NULL,
          LeavesQty INT NOT NULL,
          TradedQty INT NULL,
          ExchangeID VARCHAR(50) NOT NULL,
          Date VARCHAR(50) NOT NULL,
          InputTime VARCHAR(50) NOT NULL,
          UpdateTime VARCHAR(50) NOT NULL,
          ErrorID INT NULL,
          RawErrorID INT NULL,
          RawErrorMessage VARCHAR(100) NOT NULL
      );
  )";

  char* zErrMsg = 0;
  /* Execute SQL statement */
  int rc = sqlite3_exec(db_, sql, 0, 0, &zErrMsg);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
}

void DBStore::CreateStrategyRtnOrderIfNotExists() {
  const char* sql =
      R"(
   CREATE
       TABLE IF NOT EXISTS StrategyRtnOrder
       (
           StrategyID VARCHAR(50) NOT NULL,
           InstrumentID VARCHAR(50) NOT NULL,
           OrderID VARCHAR(50) NOT NULL,
           Status INT NOT NULL,
           Price DOUBLE NULL,
           AvgPrice DOUBLE NULL,
           Qty INT NOT NULL,
           LeavesQty INT NOT NULL,
           TradedQty INT NULL,
           ExchangeID VARCHAR(50) NOT NULL,
           Date VARCHAR(50) NOT NULL,
           InputTime VARCHAR(50) NOT NULL,
           UpdateTime VARCHAR(50) NOT NULL,
           ErrorID INT NULL,
           RawErrorID INT NULL,
           RawErrorMessage VARCHAR(100) NOT NULL
       );
   )";

  char* zErrMsg = 0;
  /* Execute SQL statement */
  int rc = sqlite3_exec(db_, sql, 0, 0, &zErrMsg);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
}

void DBStore::CreateStrategyOrderIDTableIfNotExists() {
  const char* sql =
      R"(
   CREATE
       TABLE IF NOT EXISTS StrategyOrderID
       (
           StrategyID VARCHAR(50) NOT NULL,
           StrategyOrderID VARCHAR(50) NOT NULL,
           OrderID VARCHAR(50) NOT NULL,
           AccountID VARCHAR(50) NOT NULL
       );
   )";

  char* zErrMsg = 0;
  /* Execute SQL statement */
  int rc = sqlite3_exec(db_, sql, 0, 0, &zErrMsg);

  if (rc != SQLITE_OK) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
}

caf::behavior DBStore::make_behavior() {
  return {
      [=](InsertStrategyRtnOrder, const boost::shared_ptr<OrderField> order) {
        char* error = NULL;
        auto rc = sqlite3_exec(
            db_,
            str(boost::format("INSERT INTO StrategyRtnOrder VALUES('%s', '%s', "
                              "'%s', %d, %lf, "
                              "%lf, %d, %d, %d, '%s', '%s', '%s', '%s', %d, "
                              "%d, '%s');") %
                order->strategy_id % order->instrument_id % order->order_id %
                static_cast<int>(order->status) % order->price %
                order->avg_price % order->qty % order->leaves_qty %
                order->traded_qty % order->exchange_id % order->date %
                order->input_time % order->update_time % order->error_id %
                order->raw_error_id % order->raw_error_message)
                .c_str(),
            0, 0, &error);
        if (rc != SQLITE_OK) {
          fprintf(stderr, "SQL error: %s\n", error);
          sqlite3_free(error);
        }
      },
      [=](QueryStrategyRntOrderAtom, const std::string& strategy_id)
          -> std::vector<boost::shared_ptr<OrderField>> {
        enum class Column {
          kInstrumentID,
          kOrderID,
          kStatus,
          kPrice,
          kAvgPrice,
          kQty,
          kLeavesQty,
          kTradedQty,
          kExchangeID,
          kDATE,
          kInputTime,
          kUpdateTime,
          kErrorID,
          kRawErrorID,
          kRawErrorMessage,
        };
        sqlite3_stmt* stmt = NULL;
        int ret =
            sqlite3_prepare(db_, str(boost::format(R"(
                          SELECT
                              InstrumentID,
                              OrderID,
                              Status,
                              Price,
                              AvgPrice,
                              Qty,
                              LeavesQty,
                              TradedQty,
                              ExchangeID,
                              DATE,
                              InputTime,
                              UpdateTime,
                              ErrorID,
                              RawErrorID,
                              RawErrorMessage,
                          FROM
                              StrategyRtnOrder
                          WHERE
                              StrategyID='%s'
                )") % strategy_id).c_str(),
                            -1, &stmt, NULL);
        int seq_no = 0;
        int rows = 0;
        std::vector<boost::shared_ptr<OrderField>> orders;
        while (true) {
          ret = sqlite3_step(stmt);
          if (ret != SQLITE_ROW)
            break;
          auto order = boost::make_shared<OrderField>();
          order->strategy_id = strategy_id;
          order->instrument_id =
              reinterpret_cast<const char*>(sqlite3_column_text(
                  stmt, static_cast<int>(Column::kInstrumentID)));
          order->order_id = reinterpret_cast<const char*>(
              sqlite3_column_text(stmt, static_cast<int>(Column::kOrderID)));
          order->status = static_cast<OrderStatus>(
              sqlite3_column_int(stmt, static_cast<int>(Column::kStatus)));
          order->price =
              sqlite3_column_double(stmt, static_cast<int>(Column::kPrice));
          order->avg_price =
              sqlite3_column_double(stmt, static_cast<int>(Column::kAvgPrice));
          order->qty = sqlite3_column_int(stmt, static_cast<int>(Column::kQty));
          order->leaves_qty =
              sqlite3_column_int(stmt, static_cast<int>(Column::kLeavesQty));
          order->traded_qty =
              sqlite3_column_int(stmt, static_cast<int>(Column::kTradedQty));
          order->exchange_id = reinterpret_cast<const char*>(
              sqlite3_column_text(stmt, static_cast<int>(Column::kExchangeID)));
          order->date = reinterpret_cast<const char*>(
              sqlite3_column_text(stmt, static_cast<int>(Column::kDATE)));
          order->input_time = reinterpret_cast<const char*>(
              sqlite3_column_text(stmt, static_cast<int>(Column::kInputTime)));
          order->update_time = reinterpret_cast<const char*>(
              sqlite3_column_text(stmt, static_cast<int>(Column::kUpdateTime)));
          order->error_id =
              sqlite3_column_int(stmt, static_cast<int>(Column::kErrorID));
          order->raw_error_id =
              sqlite3_column_int(stmt, static_cast<int>(Column::kRawErrorID));
          order->raw_error_message =
              reinterpret_cast<const char*>(sqlite3_column_text(
                  stmt, static_cast<int>(Column::kRawErrorMessage)));

          orders.push_back(std::move(order));
        }
        sqlite3_finalize(stmt);
        return std::move(orders);
      },
      [=](QueryStrategyOrderIDMapAtom, const std::string& account_id)
          -> std::vector<std::tuple<std::string, std::string, std::string>> {
        sqlite3_stmt* stmt = NULL;
        int ret = sqlite3_prepare(
            db_,
            str(boost::format(
                    "SELECT * FROM StrategyOrderID WHERE StrategyID='%s'") %
                account_id)
                .c_str(),
            -1, &stmt, NULL);
        int seq_no = 0;
        int rows = 0;
        std::vector<std::tuple<std::string, std::string, std::string>> tuples;
        while (true) {
          ret = sqlite3_step(stmt);
          if (ret != SQLITE_ROW)
            break;
          tuples.push_back({(const char*)sqlite3_column_text(stmt, 0),
                            (const char*)sqlite3_column_text(stmt, 1),
                            (const char*)sqlite3_column_text(stmt, 2)});
        }
        sqlite3_finalize(stmt);
        return std::move(tuples);
      },
      [=](InsertStrategyOrderIDAtom, const std::string& strategy_id,
          const std::string& strategy_order_id, const std::string& order_id) {
        char* error = NULL;
        auto rc = sqlite3_exec(
            db_,
            str(boost::format(
                    "INSERT INTO StrategyOrderID VALUES('%s', '%s', '%s');") %
                strategy_id % strategy_order_id % order_id)
                .c_str(),
            0, 0, &error);
      },

  };
}
