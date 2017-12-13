#include "caf_coordinator.h"
#include <chrono>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "yaml-cpp/yaml.h"
#include "hdf5.h"

#include "common/api_struct.h"
#include "hpt_core/time_series_reader.h"
#include "backtesting_defines.h"
#include "caf_common/caf_atom_defines.h"

namespace pt = boost::property_tree;

auto ReadTickTimeSeries(const char* hdf_file,
                        const std::string& market,
                        const std::string& instrument,
                        const std::string& datetime_from,
                        const std::string& datetime_to) {
  // auto beg = hrc::now();
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
  H5Tclose(tick_compound);
  herr_t ret = H5Fclose(file);
  // std::cout << "espces:"
  //          << std::chrono::duration_cast<std::chrono::milliseconds>(
  //                 hrc::now() - beg)
  //                 .count()
  //          << "\n";
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
                          YAML::Node* cfg,
                          std::string out_dir) {
  std::string instrument_infos_file =
      (*cfg)["InstrumentInfo"].as<std::string>();
  std::string strategy_config_file = (*cfg)["StrategyConfig"].as<std::string>();
  std::string ts_tick_path = (*cfg)["TickData"].as<std::string>();
  std::string ts_cta_signal_path = (*cfg)["CtaData"].as<std::string>();
  std::string datetime_from =
      (*cfg)["BacktestingFromDateTime"].as<std::string>();
  std::string datetime_to = (*cfg)["BacktestingToDateTime"].as<std::string>();
  int force_close_before_close_market =
      (*cfg)["CancelLimitOrderWhenSwitchTradeDate"].as<bool>();

  pt::ptree ins_infos_pt;
  try {
    pt::read_json(instrument_infos_file, ins_infos_pt);
  } catch (pt::ptree_error& err) {
    std::cout << "Read Confirg File Error:" << err.what() << "\n";
    return {};
  }

  pt::ptree strategy_config_pt;
  try {
    pt::read_json(strategy_config_file, strategy_config_pt);
  } catch (pt::ptree_error& err) {
    std::cout << "Read Confirg File Error:" << err.what() << "\n";
    return {};
  }

  // std::string ts_tick_path = cfg->ts_db;
  // std::string ts_cta_signal_path = cfg->cta_transaction_db;
  // std::string datetime_from = cfg->backtesting_from_datetime;
  // std::string datetime_to = cfg->backtesting_to_datetime;
  // int force_close_before_close_market = cfg->force_close_before_close_market;

  auto instrument_strategy_params =
      std::make_shared<std::list<StrategyParam>>();

  for (YAML::const_iterator it = (*cfg)["Instruments"].as<YAML::Node>().begin();
       it != (*cfg)["Instruments"].as<YAML::Node>().end(); ++it) {
    try {
      StrategyParam param;
      param.instrument = it->as<std::string>();
      std::string instrument_code = param.instrument.substr(
          0, param.instrument.find_first_of("0123456789"));
      boost::algorithm::to_lower(instrument_code);

      param.market = ins_infos_pt.get<std::string>(instrument_code + ".Market");
      param.margin_rate =
          ins_infos_pt.get<double>(instrument_code + ".MarginRate");
      param.constract_multiple =
          ins_infos_pt.get<int>(instrument_code + ".ConstractMultiple");
      param.cost_basis.type =
          ins_infos_pt.get<std::string>(instrument_code +
                                        ".CostBasis.CommissionType") == "fix"
              ? CommissionType::kFixed
              : CommissionType::kRate;

      param.cost_basis.open_commission = ins_infos_pt.get<double>(
          instrument_code + ".CostBasis.OpenCommission");
      param.cost_basis.close_commission = ins_infos_pt.get<double>(
          instrument_code + ".CostBasis.CloseCommission");
      param.cost_basis.close_today_commission = ins_infos_pt.get<double>(
          instrument_code + ".CostBasis.CloseTodayCommission");
      instrument_strategy_params->push_back(std::move(param));

    } catch (pt::ptree_error& err) {
      std::cout << "Read Confirg File Error:" << err.what() << "\n";
      return {};
    }
  }

  return {
    [=](IdleAtom, caf::actor work) {
    do {
      if (instrument_strategy_params->empty()) {
        break;
      }
      auto instrument_info = instrument_strategy_params->front();
      instrument_strategy_params->pop_front();
      std::string instrument = instrument_info.instrument;
      std::string market = instrument_info.market;

      using hrc = std::chrono::high_resolution_clock;
      auto tick_container = ReadTickTimeSeries(
          ts_tick_path.c_str(), market, instrument, datetime_from, datetime_to);
      if (tick_container.empty()) {
        std::cout << instrument
                  << " can't find tick data from:" << datetime_from
                  << " to:" << datetime_to << "\n";
        continue;
      }
      self->send(
          work, instrument, out_dir, strategy_config_file, instrument_info,
          force_close_before_close_market, std::move(tick_container),
          ReadCTAOrderSignalTimeSeries(ts_cta_signal_path.c_str(), instrument,
                                       datetime_from, datetime_to));
      break;
    } while (true);

    if (instrument_strategy_params->empty()) {
      self->quit();
    }
  }
  };
}
