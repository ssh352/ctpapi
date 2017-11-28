#include "init_instrument_list.h"
#include "ctpapi/ThostFtdcTraderApi.h"
#include "sqlite/sqlite3.h"

static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
  int i;
  for (i = 0; i < argc; i++) {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

class CtpInitActor : public caf::event_based_actor, public CThostFtdcTraderSpi {
 public:
  CtpInitActor(caf::actor_config& cfg,
               std::string server,
               std::string broker_id,
               std::string user_id,
               std::string password)
      : event_based_actor(cfg),
        broker_id_(std::move(broker_id)),
        user_id_(std::move(user_id)),
        password_(std::move(password)) {
    api_ = CThostFtdcTraderApi::CreateFtdcTraderApi();
    api_->RegisterSpi(this);

    char fron_server[255] = {0};
    strcpy(fron_server, server.c_str());
    api_->RegisterFront(fron_server);
    api_->SubscribePublicTopic(THOST_TERT_QUICK);
    api_->SubscribePrivateTopic(THOST_TERT_QUICK);
    api_->Init();

    sqlite3_open("test.db", &db_);
    CreateInstrumentTableIfNotExist();
    CreateMarginRateTableIfNotExist();
    CreateInstrumentCommissionRateTableIfNotExist();
    // sqlite3_close(db_);
  }
  virtual void OnFrontConnected() override {
    CThostFtdcReqUserLoginField field{0};
    strcpy(field.UserID, user_id_.c_str());
    strcpy(field.Password, password_.c_str());
    strcpy(field.BrokerID, broker_id_.c_str());
    api_->ReqUserLogin(&field, 0);
  }

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override {
    if (pRspInfo != NULL && pRspInfo->ErrorID == 0) {
      RequestInstrumentList();
    }
  }

  virtual caf::behavior make_behavior() override { return {}; }

 private:
  void RequestInstrumentList() {
    sqlite3_exec(db_, "begin;", 0, 0, 0);
    const char* sql = R"(
      INSERT INTO instrument VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
                                    ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
                                    ?)
    )";
    sqlite3_prepare_v2(db_, sql, static_cast<int>(strlen(sql)), &stmt_, 0);

    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));
    int result = api_->ReqQryInstrument(&req, 0);
  }

  void ReqQryInstrumentMarginRate(const std::string& instrument_id) {
    CThostFtdcQryInstrumentMarginRateField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.InstrumentID, instrument_id.c_str());
    strcpy(req.InvestorID, user_id_.c_str());
    strcpy(req.BrokerID, broker_id_.c_str());
    req.HedgeFlag = THOST_FTDC_HF_Speculation;
    int result = api_->ReqQryInstrumentMarginRate(&req, 0);
  }

  void ReqQryInstrumentCommissionRate(const std::string& instrument_id) {
    CThostFtdcQryInstrumentCommissionRateField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, broker_id_.c_str());
    strcpy(req.InvestorID, user_id_.c_str());
    strcpy(req.InstrumentID, instrument_id.c_str());
    int result = api_->ReqQryInstrumentCommissionRate(&req, 0);
  }

  virtual void OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument,
                                  CThostFtdcRspInfoField* pRspInfo,
                                  int nRequestID,
                                  bool bIsLast) override {
    sqlite3_reset(stmt_);
    sqlite3_bind_text(stmt_, 1, pInstrument->InstrumentID,
                      sizeof(TThostFtdcInstrumentIDType), NULL);
    sqlite3_bind_text(stmt_, 1, pInstrument->ExchangeID,
                      sizeof(TThostFtdcExchangeIDType), NULL);
    sqlite3_bind_text(stmt_, 1, pInstrument->InstrumentName,
                      sizeof(TThostFtdcInstrumentNameType), NULL);
    sqlite3_bind_text(stmt_, 1, pInstrument->ExchangeInstID,
                      sizeof(TThostFtdcExchangeInstIDType), NULL);
    sqlite3_bind_text(stmt_, 1, pInstrument->ProductID,
                      sizeof(TThostFtdcInstrumentIDType), NULL);
    sqlite3_bind_int(stmt_, 1, pInstrument->ProductClass);
    sqlite3_bind_int(stmt_, 1, pInstrument->DeliveryYear);
    sqlite3_bind_int(stmt_, 1, pInstrument->DeliveryMonth);
    sqlite3_bind_int(stmt_, 1, pInstrument->MaxMarketOrderVolume);
    sqlite3_bind_int(stmt_, 1, pInstrument->MinMarketOrderVolume);
    sqlite3_bind_int(stmt_, 1, pInstrument->MaxLimitOrderVolume);
    sqlite3_bind_int(stmt_, 1, pInstrument->MinLimitOrderVolume);
    sqlite3_bind_int(stmt_, 1, pInstrument->VolumeMultiple);
    sqlite3_bind_double(stmt_, 1, pInstrument->PriceTick);
    sqlite3_bind_text(stmt_, 1, pInstrument->CreateDate,
                      sizeof(TThostFtdcDateType), NULL);
    sqlite3_bind_text(stmt_, 1, pInstrument->OpenDate,
                      sizeof(TThostFtdcDateType), NULL);
    sqlite3_bind_text(stmt_, 1, pInstrument->ExpireDate,
                      sizeof(TThostFtdcDateType), NULL);
    sqlite3_bind_text(stmt_, 1, pInstrument->StartDelivDate,
                      sizeof(TThostFtdcDateType), NULL);
    sqlite3_bind_text(stmt_, 1, pInstrument->EndDelivDate,
                      sizeof(TThostFtdcDateType), NULL);
    sqlite3_bind_int(stmt_, 1, pInstrument->InstLifePhase);
    sqlite3_bind_int(stmt_, 1, pInstrument->IsTrading);
    sqlite3_bind_int(stmt_, 1, pInstrument->PositionType);
    sqlite3_bind_int(stmt_, 1, pInstrument->PositionDateType);
    sqlite3_bind_double(stmt_, 1, pInstrument->LongMarginRatio);
    sqlite3_bind_double(stmt_, 1, pInstrument->ShortMarginRatio);
    sqlite3_bind_int(stmt_, 1, pInstrument->MaxMarginSideAlgorithm);
    sqlite3_bind_text(stmt_, 1, pInstrument->UnderlyingInstrID,
                      sizeof(TThostFtdcInstrumentIDType), NULL);
    sqlite3_bind_double(stmt_, 1, pInstrument->StrikePrice);
    sqlite3_bind_int(stmt_, 1, pInstrument->OptionsType);
    sqlite3_bind_double(stmt_, 1, pInstrument->UnderlyingMultiple);
    sqlite3_bind_int(stmt_, 1, pInstrument->CombinationType);
    if (bIsLast) {
      sqlite3_finalize(stmt_);
      sqlite3_exec(db_, "commit;", 0, 0, 0);
    } else {
    }
  }

  void OnRspQryInstrumentMarginRate(
      CThostFtdcInstrumentMarginRateField* pInstrumentMarginRate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) {
    std::cout << pInstrumentMarginRate->InstrumentID << "\n";
  }

  void OnRspQryInstrumentCommissionRate(
      CThostFtdcInstrumentCommissionRateField* pInstrumentCommissionRate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) {
    std::cout << pInstrumentCommissionRate->InstrumentID << "\n";
  }

  void CreateInstrumentTableIfNotExist() {
    const char* sql = R"(
      CREATE TABLE IF NOT EXISTS instrument(
        instrument_id CHAR(31) PRIMARY KEY NOT NULL,
        exchange_id CHAR(9),
        instrument_name NCHAR(21),
        exchange_inst_id CHAR(31),
        product_id CHAR(31),
        product_class INT,
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
        inst_live_phase INT,
        is_trading INT,
        position_type INT,
        position_date_type INT,
        long_margin_ratio DOUBLE,
        short_margin_ratio DOUBLE,
        max_margin_side_algorithm INT,
        underlying_instr_id CHAR(31),
        strike_price DOUBLE,
        options_type INT,
        underlying_multiple DOUBLE,
        combination_type INT)
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
        investor_range INT,
        broker_id CHAR(11),
        hedge_flag INT,
        exchange_id CHAR(9),
        long_margin_ratio_by_money DOUBLE,
        long_margin_ratio_by_volume DOUBLE,
        short_margin_ratio_by_money DOUBLE,
        short_margin_ratio_by_volume DOUBLE,
        is_relative INT)
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

  void CreateInstrumentCommissionRateTableIfNotExist() {
    const char* sql = R"(
      CREATE TABLE IF NOT EXISTS instrument_commission_rate(
        instrument_id CHAR(31) PRIMARY KEY NOT NULL,
        investor_range INT,
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

  CThostFtdcTraderApi* api_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  sqlite3* db_;
  sqlite3_stmt* stmt_;
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
