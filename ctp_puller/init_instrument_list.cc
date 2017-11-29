#include "init_instrument_list.h"
#include <locale>
#include <codecvt>
#include "ctpapi/ThostFtdcTraderApi.h"
#include "sqlite/sqlite3.h"
#include "build/build_config.h"

std::string MBCSToUtf8(const std::string& input) {
  // int nLength = MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, NULL,
  // NULL);  WCHAR* tch = new WCHAR[nLength];  nLength =
  // MultiByteToWideChar(CP_ACP, 0, input.c_str(), -1, tch, nLength);  int
  // nUTF8len = WideCharToMultiByte(CP_UTF8, 0, tch, nLength, 0, 0, 0, 0); char*
  // utf8_string = new char[nUTF8len];  WideCharToMultiByte(CP_UTF8, 0, tch,
  // nLength, utf8_string, nUTF8len, 0, 0);  delete tch;  std::string
  // out_string(utf8_string, nUTF8len);  delete utf8_string;  return out_string;

#if defined(OS_WIN)
  std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> wide_cv(
      new std::codecvt_byname<wchar_t, char, mbstate_t>(".936"));
  std::wstring w = wide_cv.from_bytes(input);
  std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_cv;
  return utf8_cv.to_bytes(w);
#endif
}

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

    sqlite3_open((user_id_ + ".db").c_str(), &db_);
    CreateInstrumentTableIfNotExist();
    CreateMarginRateTableIfNotExist();
    CreateInstrumentCommissionRateTableIfNotExist();
    // sqlite3_close(db_);
  }

  ~CtpInitActor() {
    sqlite3_close(db_);
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
    int rc = sqlite3_exec(db_, "begin;", 0, 0, 0);
    const char* sql = R"(
      INSERT INTO instrument VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
                                    ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?,
                                    ?)
    )";
    rc = sqlite3_prepare_v2(db_, sql, static_cast<int>(strlen(sql)), &stmt_, 0);

    CThostFtdcQryInstrumentField req;
    memset(&req, 0, sizeof(req));
    int result = api_->ReqQryInstrument(&req, 0);
  }

  void ReqQryInstrumentMarginRate(const std::string& instrument_id) {
    std::cout << "Request Margin Rate:" << instrument_id << "\n";
    CThostFtdcQryInstrumentMarginRateField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.InstrumentID, instrument_id.c_str());
    strcpy(req.InvestorID, user_id_.c_str());
    strcpy(req.BrokerID, broker_id_.c_str());
    req.HedgeFlag = THOST_FTDC_HF_Speculation;
    while (api_->ReqQryInstrumentMarginRate(&req, 0) != 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  }

  void ReqQryInstrumentCommissionRate(const std::string& instrument_id) {
    std::cout << "Request Commission Rate:" << instrument_id << "\n";
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
    sqlite3_bind_text(stmt_, 2, pInstrument->ExchangeID,
                      sizeof(TThostFtdcExchangeIDType), NULL);
    std::string instrument_name = MBCSToUtf8(pInstrument->InstrumentName);
    sqlite3_bind_text(stmt_, 3, instrument_name.c_str(),
                      static_cast<int>(instrument_name.length()), NULL);
    sqlite3_bind_text(stmt_, 4, pInstrument->ExchangeInstID,
                      sizeof(TThostFtdcExchangeInstIDType), NULL);
    sqlite3_bind_text(stmt_, 5, pInstrument->ProductID,
                      sizeof(TThostFtdcInstrumentIDType), NULL);
    sqlite3_bind_int(stmt_, 6, pInstrument->ProductClass);
    sqlite3_bind_int(stmt_, 7, pInstrument->DeliveryYear);
    sqlite3_bind_int(stmt_, 8, pInstrument->DeliveryMonth);
    sqlite3_bind_int(stmt_, 9, pInstrument->MaxMarketOrderVolume);
    sqlite3_bind_int(stmt_, 10, pInstrument->MinMarketOrderVolume);
    sqlite3_bind_int(stmt_, 11, pInstrument->MaxLimitOrderVolume);
    sqlite3_bind_int(stmt_, 12, pInstrument->MinLimitOrderVolume);
    sqlite3_bind_int(stmt_, 13, pInstrument->VolumeMultiple);
    sqlite3_bind_double(stmt_, 14, pInstrument->PriceTick);
    sqlite3_bind_text(stmt_, 15, pInstrument->CreateDate,
                      sizeof(TThostFtdcDateType), NULL);
    sqlite3_bind_text(stmt_, 16, pInstrument->OpenDate,
                      sizeof(TThostFtdcDateType), NULL);
    sqlite3_bind_text(stmt_, 17, pInstrument->ExpireDate,
                      sizeof(TThostFtdcDateType), NULL);
    sqlite3_bind_text(stmt_, 18, pInstrument->StartDelivDate,
                      sizeof(TThostFtdcDateType), NULL);
    sqlite3_bind_text(stmt_, 19, pInstrument->EndDelivDate,
                      sizeof(TThostFtdcDateType), NULL);
    sqlite3_bind_int(stmt_, 20, pInstrument->InstLifePhase);
    sqlite3_bind_int(stmt_, 21, pInstrument->IsTrading);
    sqlite3_bind_int(stmt_, 22, pInstrument->PositionType);
    sqlite3_bind_int(stmt_, 23, pInstrument->PositionDateType);
    sqlite3_bind_double(stmt_, 24, pInstrument->LongMarginRatio);
    sqlite3_bind_double(stmt_, 25, pInstrument->ShortMarginRatio);
    sqlite3_bind_int(stmt_, 26, pInstrument->MaxMarginSideAlgorithm);
    sqlite3_bind_text(stmt_, 27, pInstrument->UnderlyingInstrID,
                      sizeof(TThostFtdcInstrumentIDType), NULL);
    sqlite3_bind_double(stmt_, 28, pInstrument->StrikePrice);
    sqlite3_bind_int(stmt_, 29, pInstrument->OptionsType);
    sqlite3_bind_double(stmt_, 30, pInstrument->UnderlyingMultiple);
    sqlite3_bind_int(stmt_, 31, pInstrument->CombinationType);
    sqlite3_step(stmt_);

    if (pInstrument->ProductClass == THOST_FTDC_PC_Futures) {
      pending_request_margin_rate_instruments_.push_back(
          pInstrument->InstrumentID);
      pending_request_commission_instruments_.push_back(
          pInstrument->InstrumentID);
    }
    if (bIsLast) {
      sqlite3_finalize(stmt_);
      sqlite3_exec(db_, "commit;", 0, 0, 0);

      int rc = sqlite3_exec(db_, "begin;", 0, 0, 0);
      const char* sql = R"(
        INSERT INTO margin_rate VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
      )";
      rc = sqlite3_prepare_v2(db_, sql, static_cast<int>(strlen(sql)), &stmt_,
                              0);

      RequestNextInstrumentMarginRate();
    } else {
    }
  }

  void OnRspQryInstrumentMarginRate(
      CThostFtdcInstrumentMarginRateField* pInstrumentMarginRate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) {
    if (pInstrumentMarginRate != NULL &&
        (pRspInfo == NULL || pRspInfo->ErrorID == 0)) {
      sqlite3_reset(stmt_);
      sqlite3_bind_text(stmt_, 1, pInstrumentMarginRate->InstrumentID,
                        sizeof(TThostFtdcInstrumentIDType), NULL);
      sqlite3_bind_int(stmt_, 2, pInstrumentMarginRate->InvestorRange);
      sqlite3_bind_text(stmt_, 3, pInstrumentMarginRate->BrokerID,
                        sizeof(TThostFtdcBrokerIDType), NULL);
      sqlite3_bind_text(stmt_, 4, pInstrumentMarginRate->InvestorID,
                        sizeof(TThostFtdcInvestorIDType), NULL);
      sqlite3_bind_int(stmt_, 5, pInstrumentMarginRate->HedgeFlag);
      sqlite3_bind_double(stmt_, 6,
                          pInstrumentMarginRate->LongMarginRatioByMoney);
      sqlite3_bind_double(stmt_, 7,
                          pInstrumentMarginRate->LongMarginRatioByVolume);
      sqlite3_bind_double(stmt_, 8,
                          pInstrumentMarginRate->ShortMarginRatioByMoney);
      sqlite3_bind_double(stmt_, 9,
                          pInstrumentMarginRate->ShortMarginRatioByVolume);
      sqlite3_bind_int(stmt_, 10, pInstrumentMarginRate->IsRelative);
      sqlite3_step(stmt_);

    } else {
      std::cout << "ReqQryInstrument Margin Rate Error"
                << "\n";
    }
    RequestNextInstrumentMarginRate();
  }

  void OnRspQryInstrumentCommissionRate(
      CThostFtdcInstrumentCommissionRateField* pInstrumentCommissionRate,
      CThostFtdcRspInfoField* pRspInfo,
      int nRequestID,
      bool bIsLast) {
    if (pInstrumentCommissionRate != NULL &&
        (pRspInfo == NULL || pRspInfo->ErrorID == 0)) {
      sqlite3_reset(stmt_);
      sqlite3_bind_text(stmt_, 1, pInstrumentCommissionRate->InstrumentID,
                        sizeof(TThostFtdcInstrumentIDType), NULL);
      sqlite3_bind_int(stmt_, 1, pInstrumentCommissionRate->InvestorRange);
      sqlite3_bind_text(stmt_, 1, pInstrumentCommissionRate->BrokerID,
                        sizeof(TThostFtdcBrokerIDType), NULL);
      sqlite3_bind_text(stmt_, 1, pInstrumentCommissionRate->InvestorID,
                        sizeof(TThostFtdcInvestorIDType), NULL);
      sqlite3_bind_double(stmt_, 1,
                          pInstrumentCommissionRate->OpenRatioByMoney);
      sqlite3_bind_double(stmt_, 1,
                          pInstrumentCommissionRate->OpenRatioByVolume);
      sqlite3_bind_double(stmt_, 1,
                          pInstrumentCommissionRate->CloseRatioByMoney);
      sqlite3_bind_double(stmt_, 1,
                          pInstrumentCommissionRate->CloseRatioByVolume);
      sqlite3_bind_double(stmt_, 1,
                          pInstrumentCommissionRate->CloseTodayRatioByMoney);
      sqlite3_bind_double(stmt_, 1,
                          pInstrumentCommissionRate->CloseTodayRatioByVolume);
      sqlite3_step(stmt_);

      RequestNextInstrumentCommisstion();
    }
  }

  void CreateInstrumentTableIfNotExist() {
    const char* sql = R"(
      CREATE TABLE IF NOT EXISTS instrument(
        instrument_id CHAR(31) PRIMARY KEY NOT NULL,
        exchange_id CHAR(9),
        instrument_name CHAR(21),
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
        investor_id CHAR(13),
        hedge_flag INT,
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
  std::list<std::string> pending_request_margin_rate_instruments_;
  std::list<std::string> pending_request_commission_instruments_;

  void RequestNextInstrumentMarginRate() {
    if (pending_request_margin_rate_instruments_.empty()) {
      sqlite3_finalize(stmt_);
      sqlite3_exec(db_, "commit;", 0, 0, 0);

      // perpare request commisstion
      int rc = sqlite3_exec(db_, "begin;", 0, 0, 0);
      const char* sql = R"(
        INSERT INTO instrument_commission_rate VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
      )";
      rc = sqlite3_prepare_v2(db_, sql, static_cast<int>(strlen(sql)), &stmt_,
                              0);
      RequestNextInstrumentCommisstion();
      return;
    }
    ReqQryInstrumentMarginRate(
        pending_request_margin_rate_instruments_.front());
    pending_request_margin_rate_instruments_.pop_front();
  }

  void RequestNextInstrumentCommisstion() {
    if (pending_request_commission_instruments_.empty()) {
      sqlite3_finalize(stmt_);
      sqlite3_exec(db_, "commit;", 0, 0, 0);
      return;
    }

    ReqQryInstrumentCommissionRate(
        pending_request_commission_instruments_.front());
    pending_request_commission_instruments_.pop_front();
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
