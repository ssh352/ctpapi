#include "caf_coordinator.h"
#include "hdf5.h"
#include "atom_defines.h"
#include "common/api_struct.h"
#include "hpt_core/time_series_reader.h"

auto ReadTickTimeSeries(const char* hdf_file,
                        const std::string& market,
                        const std::string& instrument,
                        const std::string& datetime_from,
                        const std::string& datetime_to) {
  hid_t file = H5Fopen(hdf_file, H5F_ACC_RDONLY, H5P_DEFAULT);
  hid_t tick_compound = H5Tcreate(H5T_COMPOUND, sizeof(Tick));
  TimeSeriesReader<Tick> ts_db(file, tick_compound);

  H5Tinsert(tick_compound, "timestamp", HOFFSET(Tick, timestamp),
            H5T_NATIVE_INT64);
  H5Tinsert(tick_compound, "last_price", HOFFSET(Tick, last_price),
            H5T_NATIVE_DOUBLE);
  H5Tinsert(tick_compound, "qty", HOFFSET(Tick, qty), H5T_NATIVE_INT64);
  H5Tinsert(tick_compound, "bid_price1", HOFFSET(Tick, bid_price1),
            H5T_NATIVE_DOUBLE);
  H5Tinsert(tick_compound, "bid_qty1", HOFFSET(Tick, bid_qty1),
            H5T_NATIVE_INT64);
  H5Tinsert(tick_compound, "ask_price1", HOFFSET(Tick, ask_price1),
            H5T_NATIVE_DOUBLE);
  H5Tinsert(tick_compound, "ask_qty1", HOFFSET(Tick, ask_qty1),
            H5T_NATIVE_INT64);

  auto ts_ret =
      ts_db.ReadRange(str(boost::format("/%s/%s") % market % instrument),
                      boost::posix_time::time_from_string(datetime_from),
                      boost::posix_time::time_from_string(datetime_to));
  herr_t ret = H5Fclose(file);
  return std::move(ts_ret);
}

auto ReadCTAOrderSignalTimeSeries(const char* hdf_file,
                                  const std::string& instrument,
                                  const std::string& datetime_from,
                                  const std::string& datetime_to) {
  hid_t file = H5Fopen(hdf_file, H5F_ACC_RDONLY, H5P_DEFAULT);
  hid_t tick_compound = H5Tcreate(H5T_COMPOUND, sizeof(CTATransaction));

  H5Tinsert(tick_compound, "timestamp", HOFFSET(CTATransaction, timestamp),
            H5T_NATIVE_INT64);
  hid_t order_no_str = H5Tcopy(H5T_C_S1);
  int len = sizeof(OrderIDType) / sizeof(char);
  H5Tset_size(order_no_str, len);

  H5Tinsert(tick_compound, "order_no", HOFFSET(CTATransaction, order_id),
            order_no_str);

  H5Tinsert(tick_compound, "position_effect",
            HOFFSET(CTATransaction, position_effect), H5T_NATIVE_INT32);
  H5Tinsert(tick_compound, "direction", HOFFSET(CTATransaction, direction),
            H5T_NATIVE_INT32);
  H5Tinsert(tick_compound, "status", HOFFSET(CTATransaction, status),
            H5T_NATIVE_INT32);
  H5Tinsert(tick_compound, "price", HOFFSET(CTATransaction, price),
            H5T_NATIVE_DOUBLE);
  H5Tinsert(tick_compound, "qty", HOFFSET(CTATransaction, qty),
            H5T_NATIVE_INT32);

  H5Tinsert(tick_compound, "traded_qty", HOFFSET(CTATransaction, traded_qty),
            H5T_NATIVE_INT32);

  TimeSeriesReader<CTATransaction> ts_db(file, tick_compound);
  auto ts_ret =
      ts_db.ReadRange(str(boost::format("/%s") % instrument),
                      boost::posix_time::time_from_string(datetime_from),
                      boost::posix_time::time_from_string(datetime_to));
  herr_t ret = H5Fclose(file);
  return std::move(ts_ret);
}

caf::behavior Coordinator(caf::event_based_actor* self,
                          pt::ptree* instrument_infos_pt,
                          pt::ptree* strategy_config_pt,
                          const config* cfg) {
  std::string ts_tick_path = cfg->ts_db;
  std::string ts_cta_signal_path = cfg->cta_transaction_db;
  std::string datetime_from = cfg->backtesting_from_datetime;
  std::string datetime_to = cfg->backtesting_to_datetime;
  int force_close_before_close_market = cfg->force_close_before_close_market;
  std::string out_dir = cfg->out_dir;
  struct StrategyParam {
    std::string instrument;
    std::string market;
    double margin_rate;
    int constract_multiple;
    CostBasis cost_basis;
    int delay_open_order_after_seconds;
    double price_offset;
  };

  auto instrument_strategy_params =
      std::make_shared<std::list<StrategyParam>>();

  for (const auto& pt : strategy_config_pt->get_child("backtesting")) {
    try {
      StrategyParam param;
      param.instrument = pt.second.get_value<std::string>();

      std::string instrument_code = param.instrument.substr(
          0, param.instrument.find_first_of("0123456789"));
      boost::algorithm::to_lower(instrument_code);

      param.market =
          instrument_infos_pt->get<std::string>(param.instrument + ".Market");
      param.margin_rate =
          instrument_infos_pt->get<double>(param.instrument + ".MarginRate");
      param.constract_multiple =
          instrument_infos_pt->get<int>(param.instrument + ".ConstractMultiple");
      param.cost_basis.type =
          instrument_infos_pt->get<std::string>(
              param.instrument + ".CostBasis.CommissionType") == "fix"
              ? CommissionType::kFixed
              : CommissionType::kRate;

      param.cost_basis.open_commission = instrument_infos_pt->get<double>(
          param.instrument + ".CostBasis.OpenCommission");
      param.cost_basis.close_commission = instrument_infos_pt->get<double>(
          param.instrument + ".CostBasis.CloseCommission");
      param.cost_basis.close_today_commission =
          instrument_infos_pt->get<double>(param.instrument +
                                           ".CostBasis.CloseTodayCommission");
      param.delay_open_order_after_seconds = strategy_config_pt->get<int>(
          instrument_code + ".DelayOpenOrderAfterSeconds");
      param.price_offset =
          strategy_config_pt->get<double>(instrument_code + ".PriceOffset");
      instrument_strategy_params->push_back(std::move(param));

    } catch (pt::ptree_error& err) {
      std::cout << "Read Confirg File Error:" << pt.first << ":" << err.what()
                << "\n";
      return {};
    }
  }

  return {[=](IdleAtom, caf::actor work) {
    // std::string ts_tick_path = "d:/ts_futures.h5";
    // std::string ts_cta_signal_path =
    //    "d:/WorkSpace/backtesing_cta/cta_tstable.h5";

    auto instrument_with_market = instrument_strategy_params->front();
    instrument_strategy_params->pop_front();
    std::string market = instrument_with_market.market;
    std::string instrument = instrument_with_market.instrument;
    self->send(
        work, market, instrument, out_dir,
        instrument_with_market.delay_open_order_after_seconds,
        force_close_before_close_market, instrument_with_market.price_offset,
        instrument_with_market.margin_rate,
        instrument_with_market.constract_multiple,
        instrument_with_market.cost_basis,
        ReadTickTimeSeries(ts_tick_path.c_str(), market, instrument,
                           datetime_from, datetime_to),
        ReadCTAOrderSignalTimeSeries(ts_cta_signal_path.c_str(), instrument,
                                     datetime_from, datetime_to));

    if (instrument_strategy_params->empty()) {
      self->quit();
    }
  }};
}
