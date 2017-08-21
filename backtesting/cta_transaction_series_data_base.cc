#include "cta_transaction_series_data_base.h"
#include <boost/format.hpp>
#include "time_util.h"

CTATransactionSeriesDataBase::CTATransactionSeriesDataBase(
    const char* hdf_file) {
  file_ = H5Fopen(hdf_file, H5F_ACC_RDONLY, H5P_DEFAULT);
  tick_compound_ = H5Tcreate(H5T_COMPOUND, sizeof(CTATransaction));

  H5Tinsert(tick_compound_, "timestamp", HOFFSET(CTATransaction, timestamp),
            H5T_NATIVE_INT64);
  H5Tinsert(tick_compound_, "position_effect",
            HOFFSET(CTATransaction, position_effect), H5T_NATIVE_INT32);
  H5Tinsert(tick_compound_, "direction", HOFFSET(CTATransaction, direction),
            H5T_NATIVE_INT32);
  H5Tinsert(tick_compound_, "price", HOFFSET(CTATransaction, price),
            H5T_NATIVE_DOUBLE);
  H5Tinsert(tick_compound_, "qty", HOFFSET(CTATransaction, qty),
            H5T_NATIVE_INT32);
}

CTATransactionSeriesDataBase::~CTATransactionSeriesDataBase() {
  herr_t ret = H5Fclose(file_);
}

std::vector<std::pair<std::unique_ptr<CTATransaction[]>, int64_t> >
CTATransactionSeriesDataBase::ReadRange(const std::string& instrument_path,
                                        boost::posix_time::ptime start_dt,
                                        boost::posix_time::ptime end_dt) {
  std::vector<std::pair<std::unique_ptr<CTATransaction[]>, int64_t> > result;

  // std::vector<boost::gregorian::date> keys;
  for (boost::gregorian::day_iterator it(start_dt.date()); it <= end_dt.date();
       ++it) {
    // keys.push_back(*it);
    auto partition_result =
        FetchRowsFromPartition(instrument_path, *it, start_dt, end_dt);
    if (partition_result.second != 0) {
      result.push_back(std::move(partition_result));
    }
  }

  return result;
}

std::pair<std::unique_ptr<CTATransaction[]>, int64_t>
CTATransactionSeriesDataBase::FetchRowsFromPartition(
    const std::string& instrument_path,
    boost::gregorian::date date,
    boost::posix_time::ptime start_dt,
    boost::posix_time::ptime end_dt) const {
  H5Eset_auto(H5E_DEFAULT, NULL, NULL);
  std::string path =
      str(boost::format("%s/y%04d/m%02d/d%02d/ts_data") % instrument_path %
          date.year() % static_cast<int>(date.month()) % date.day());

  herr_t err = H5Lget_info(file_, path.c_str(), NULL, H5P_DEFAULT);
  H5Eset_auto(H5E_DEFAULT, (H5E_auto2_t)H5Eprint, stderr);
  if (err != 0) {
    return {std::unique_ptr<CTATransaction[]>(), 0};
  }

  hid_t dataset = H5Dopen2(file_, path.c_str(), H5P_DEFAULT);

  if (date == start_dt.date() && date == end_dt.date()) {
    hid_t attr = H5Aopen(dataset, "NROWS", H5P_DEFAULT);
    int64_t rows = 0;
    herr_t ret = H5Aread(attr, H5T_NATIVE_INT64, &rows);
    std::unique_ptr<CTATransaction[]> pre_ticks(new CTATransaction[rows]);
    ret = H5Dread(dataset, tick_compound_, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                  pre_ticks.get());

    CTATransaction start_tick{0};
    start_tick.timestamp = ptime_to_timestamp(start_dt);
    auto start_it =
        std::lower_bound(pre_ticks.get(), pre_ticks.get() + rows, start_tick,
                         [](const CTATransaction& l, const CTATransaction& r) {
                           return l.timestamp < r.timestamp;
                         });

    CTATransaction end_tick{0};
    end_tick.timestamp = ptime_to_timestamp(end_dt);
    auto end_it =
        std::upper_bound(pre_ticks.get(), pre_ticks.get() + rows, end_tick,
                         [](const CTATransaction& l, const CTATransaction& r) {
                           return l.timestamp < r.timestamp;
                         });

    auto request_row = end_it - start_it;
    std::unique_ptr<CTATransaction[]> ticks(new CTATransaction[request_row]);
    memcpy(ticks.get(), start_it, sizeof(CTATransaction) * (request_row));
    return {std::move(ticks), request_row};
  } else if (date == start_dt.date()) {
    hid_t attr = H5Aopen(dataset, "NROWS", H5P_DEFAULT);
    int64_t rows = 0;
    herr_t ret = H5Aread(attr, H5T_NATIVE_INT64, &rows);
    std::unique_ptr<CTATransaction[]> pre_ticks(new CTATransaction[rows]);
    ret = H5Dread(dataset, tick_compound_, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                  pre_ticks.get());

    CTATransaction start_tick{0};
    start_tick.timestamp = ptime_to_timestamp(start_dt);
    auto start_it =
        std::lower_bound(pre_ticks.get(), pre_ticks.get() + rows, start_tick,
                         [](const CTATransaction& l, const CTATransaction& r) {
                           return l.timestamp < r.timestamp;
                         });

    auto request_row = pre_ticks.get() + rows - start_it;
    std::unique_ptr<CTATransaction[]> ticks(new CTATransaction[request_row]);
    memcpy(ticks.get(), start_it, sizeof(CTATransaction) * (request_row));
    return {std::move(ticks), request_row};

  } else if (date == end_dt.date()) {
    hid_t attr = H5Aopen(dataset, "NROWS", H5P_DEFAULT);
    int64_t rows = 0;
    herr_t ret = H5Aread(attr, H5T_NATIVE_INT64, &rows);
    std::unique_ptr<CTATransaction[]> pre_ticks(new CTATransaction[rows]);
    ret = H5Dread(dataset, tick_compound_, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                  pre_ticks.get());

    CTATransaction end_tick{0};
    end_tick.timestamp = ptime_to_timestamp(end_dt);
    auto end_it =
        std::upper_bound(pre_ticks.get(), pre_ticks.get() + rows, end_tick,
                         [](const CTATransaction& l, const CTATransaction& r) {
                           return l.timestamp < r.timestamp;
                         });

    auto request_row = end_it - pre_ticks.get();
    std::unique_ptr<CTATransaction[]> ticks(new CTATransaction[request_row]);
    memcpy(ticks.get(), pre_ticks.get(),
           sizeof(CTATransaction) * (request_row));
    return {std::move(ticks), request_row};
  } else {
    hid_t attr = H5Aopen(dataset, "NROWS", H5P_DEFAULT);
    int64_t rows = 0;
    herr_t ret = H5Aread(attr, H5T_NATIVE_INT64, &rows);
    std::unique_ptr<CTATransaction[]> ticks(new CTATransaction[rows]);
    ret = H5Dread(dataset, tick_compound_, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                  ticks.get());
    return {std::move(ticks), rows};
  }

  herr_t ret = H5Dclose(dataset);
  return {std::unique_ptr<CTATransaction[]>(), 0};
}
