#include "tick_series_data_base.h"

#include <boost/format.hpp>


typedef int64_t timestamp_t;

timestamp_t ptime_to_timestamp(boost::posix_time::ptime timestamp) {
  using namespace boost::posix_time;
  using namespace boost::gregorian;
  ptime epoch(date(1970, 1, 1));
  return ((timestamp - epoch).ticks()) /
         (time_duration::ticks_per_second() / 1000);
}


TickSeriesDataBase::TickSeriesDataBase(const char* hdf_file) {
  file_ = H5Fopen(hdf_file, H5F_ACC_RDONLY, H5P_DEFAULT);

  tick_compound_ = H5Tcreate(H5T_COMPOUND, sizeof(Tick));

  H5Tinsert(tick_compound_, "timestamp", HOFFSET(Tick, timestamp),
            H5T_NATIVE_INT64);
  H5Tinsert(tick_compound_, "last_price", HOFFSET(Tick, last_price),
            H5T_NATIVE_DOUBLE);
  H5Tinsert(tick_compound_, "qty", HOFFSET(Tick, qty), H5T_NATIVE_INT64);
  H5Tinsert(tick_compound_, "bid_price1", HOFFSET(Tick, bid_price1),
            H5T_NATIVE_DOUBLE);
  H5Tinsert(tick_compound_, "bid_qty1", HOFFSET(Tick, bid_qty1),
            H5T_NATIVE_INT64);
  H5Tinsert(tick_compound_, "ask_price1", HOFFSET(Tick, ask_price1),
            H5T_NATIVE_DOUBLE);
  H5Tinsert(tick_compound_, "ask_qty1", HOFFSET(Tick, ask_qty1),
            H5T_NATIVE_INT64);
}

TickSeriesDataBase::~TickSeriesDataBase() {
  herr_t ret = H5Fclose(file_);
}

std::vector<std::pair<std::unique_ptr<Tick[]>, int64_t> >
TickSeriesDataBase::ReadRange(const std::string& instrument_path,
                              boost::posix_time::ptime start_dt,
                              boost::posix_time::ptime end_dt) {
  std::vector<std::pair<std::unique_ptr<Tick[]>, int64_t> > result;

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

std::pair<std::unique_ptr<Tick[]>, int64_t>
TickSeriesDataBase::FetchRowsFromPartition(
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
    return {std::unique_ptr<Tick[]>(), 0};
  }

  hid_t dataset = H5Dopen2(file_, path.c_str(), H5P_DEFAULT);

  if (date == start_dt.date() && date == end_dt.date()) {
    hid_t attr = H5Aopen(dataset, "NROWS", H5P_DEFAULT);
    int64_t rows = 0;
    herr_t ret = H5Aread(attr, H5T_NATIVE_INT64, &rows);
    std::unique_ptr<Tick[]> pre_ticks(new Tick[rows]);
    ret = H5Dread(dataset, tick_compound_, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                  pre_ticks.get());

    Tick start_tick{0};
    start_tick.timestamp = ptime_to_timestamp(start_dt);
    auto start_it = std::lower_bound(
        pre_ticks.get(), pre_ticks.get() + rows, start_tick,
        [](const Tick& l, const Tick& r) { return l.timestamp < r.timestamp; });

    Tick end_tick{0};
    end_tick.timestamp = ptime_to_timestamp(end_dt);
    auto end_it = std::upper_bound(
        pre_ticks.get(), pre_ticks.get() + rows, end_tick,
        [](const Tick& l, const Tick& r) { return l.timestamp < r.timestamp; });

    auto request_row = end_it - start_it;
    std::unique_ptr<Tick[]> ticks(new Tick[request_row]);
    memcpy(ticks.get(), start_it, sizeof(Tick) * (request_row));
    return {std::move(ticks), request_row};
  } else if (date == start_dt.date()) {
    hid_t attr = H5Aopen(dataset, "NROWS", H5P_DEFAULT);
    int64_t rows = 0;
    herr_t ret = H5Aread(attr, H5T_NATIVE_INT64, &rows);
    std::unique_ptr<Tick[]> pre_ticks(new Tick[rows]);
    ret = H5Dread(dataset, tick_compound_, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                  pre_ticks.get());

    Tick start_tick{0};
    start_tick.timestamp = ptime_to_timestamp(start_dt);
    auto start_it = std::lower_bound(
        pre_ticks.get(), pre_ticks.get() + rows, start_tick,
        [](const Tick& l, const Tick& r) { return l.timestamp < r.timestamp; });

    auto request_row = pre_ticks.get() + rows - start_it;
    std::unique_ptr<Tick[]> ticks(new Tick[request_row]);
    memcpy(ticks.get(), start_it, sizeof(Tick) * (request_row));
    return {std::move(ticks), request_row};

  } else if (date == end_dt.date()) {
    hid_t attr = H5Aopen(dataset, "NROWS", H5P_DEFAULT);
    int64_t rows = 0;
    herr_t ret = H5Aread(attr, H5T_NATIVE_INT64, &rows);
    std::unique_ptr<Tick[]> pre_ticks(new Tick[rows]);
    ret = H5Dread(dataset, tick_compound_, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                  pre_ticks.get());

    Tick end_tick{0};
    end_tick.timestamp = ptime_to_timestamp(end_dt);
    auto end_it = std::upper_bound(
        pre_ticks.get(), pre_ticks.get() + rows, end_tick,
        [](const Tick& l, const Tick& r) { return l.timestamp < r.timestamp; });

    auto request_row = end_it - pre_ticks.get();
    std::unique_ptr<Tick[]> ticks(new Tick[request_row]);
    memcpy(ticks.get(), pre_ticks.get(), sizeof(Tick) * (request_row));
    return {std::move(ticks), request_row};
  } else {
    hid_t attr = H5Aopen(dataset, "NROWS", H5P_DEFAULT);
    int64_t rows = 0;
    herr_t ret = H5Aread(attr, H5T_NATIVE_INT64, &rows);
    std::unique_ptr<Tick[]> ticks(new Tick[rows]);
    ret = H5Dread(dataset, tick_compound_, H5S_ALL, H5S_ALL, H5P_DEFAULT,
                  ticks.get());
    return {std::move(ticks), rows};
  }

  herr_t ret = H5Dclose(dataset);
  return {std::unique_ptr<Tick[]>(), 0};
}
