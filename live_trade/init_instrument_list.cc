#include "init_instrument_list.h"
#include "ctp_trader_api.h"
#include "sqlite/sqlite3.h"

static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
  int i;
  for (i = 0; i < argc; i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

class CtpInitActor : public caf::event_based_actor,
                     public CTPTraderApi::Delegate {
 public:
  CtpInitActor(caf::actor_config& cfg,
               std::string server,
               std::string broker_id,
               std::string user_id,
               std::string password)
      : event_based_actor(cfg), api_(this, "init") {
    api_.Connect(server, broker_id, user_id, password);
    sqlite3_open("test.db", &db_);
    CreateInstrumentTableIfNotExist();
    CreateMarginRateTableIfNotExist();
    CreateInstrumentCommissionRateTableIfNotExist();
    //sqlite3_close(db_);
  }

  virtual void HandleCtpLogon(int front_id, int session_id) override {
     api_.RequestInstrumentList();
    // api_.ReqQryInstrumentMarginRateList();
    // api_.ReqQryInstrumentCommissionRateList();
  }

  virtual void HandleCTPRtnOrder(
      const std::shared_ptr<CTPOrderField>& order) override {}

  virtual void HandleCTPTradeOrder(const std::string& instrument,
                                   const std::string& order_id,
                                   double trading_price,
                                   int trading_qty,
                                   TimeStamp timestamp) override {}

  virtual void HandleRspYesterdayPosition(
      std::vector<OrderPosition> yesterday_positions) override {}

  virtual caf::behavior make_behavior() override { return {}; }

 private:
  void CreateInstrumentTableIfNotExist() {
    const char* sql = R"(
      CREATE TABLE IF NOT EXISTS instrument(
        instrument_id CHAR(31) PRIMARY KEY NOT NULL,
        exchange_id CHAR(9),
        instrument_name NCHAR(21),
        exchange_inst_id CHAR(31),
        product_id CHAR(31),
        product_class CHAR(1),
        delivery_year INT,
        delivery_month INT,
        max_market_order_volume INT,
        min_market_order_volume INT,
        max_limit_order_volume INT,
        min_limit_order_volume INT,
        volume_multiple INT,
        price_tick DOUBLE,
        create_date CHAR(9),
        open_date CHAR(9),
        expire_date CHAR(9),
        start_delive_date CHAR(9),
        end_delive_date CHAR(9),
        inst_live_phase CHAR(1),
        is_trading CHAR(1),
        position_type CHAR(1),
        position_date_type CHAR(1),
        long_margin_ratio DOUBLE,
        short_margin_ratio DOUBLE,
        max_margin_side_algorithm CHAR(1),
        underlying_instr_id CHAR(31),
        strike_price DOUBLE,
        options_type CHAR(1),
        underlying_multiple DOUBLE,
        combination_type CHAR(1))
   )";
    /* Execute SQL statement */

    char* zErrMsg = 0;
    int rc = sqlite3_exec(db_, sql, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    } else {
      fprintf(stdout, "Table created successfully\n");
    }
  }

  void CreateMarginRateTableIfNotExist() {
    const char* sql = R"(
      CREATE TABLE IF NOT EXISTS margin_rate(
        instrument_id CHAR(31) PRIMARY KEY NOT NULL,
        broker_id CHAR(11),
        hedge_flag CHAR(1),
        exchange_id CHAR(9),
        long_margin_ratio_by_money DOUBLE,
        long_margin_ratio_by_volume DOUBLE,
        short_margin_ratio_by_money DOUBLE,
        short_margin_ratio_by_volume DOUBLE)
   )";

    char* zErrMsg = 0;
    int rc = sqlite3_exec(db_, sql, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    } else {
      fprintf(stdout, "Table created successfully\n");
    }
  }

  CTPTraderApi api_;
  sqlite3* db_;

  void CreateInstrumentCommissionRateTableIfNotExist() {
    const char* sql = R"(
      CREATE TABLE IF NOT EXISTS instrument_commission_rate(
        instrument_id CHAR(31) PRIMARY KEY NOT NULL,
        investor_range CHAR(1),
        broker_id CHAR(11),
        investor_id CHAR(13),
        open_ratio_by_money DOUBLE,
        open_ratio_by_volume DOUBLE,
        close_ratio_by_money DOUBLE,
        close_ratio_by_volume DOUBLE,
        closeToday_ratio_by_money DOUBLE,
        closeToday_ratio_by_volume DOUBLE)
   )";

    char* zErrMsg = 0;
    int rc = sqlite3_exec(db_, sql, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
    } else {
      fprintf(stdout, "Table created successfully\n");
    }
  }


};

bool InitInstrumenList(caf::actor_system& system,
                       const std::string& server,
                       const std::string& broker_id,
                       const std::string& user_id,
                       const std::string& password) {
  auto actor = system.spawn<CtpInitActor>(server, broker_id, user_id, password);
  std::string input;
  while (std::cin >> input) {
    if (input == "exit")
      break;
  }

  return true;
}
