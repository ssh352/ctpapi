#ifndef HPT_CORE_TIME_SERIES_READER_H
#define HPT_CORE_TIME_SERIES_READER_H
#include "hpt_core/time_util.h"

template <typename T>
class TimeSeriesReader {
 public:
  TimeSeriesReader(hid_t file, hid_t tick_compound)
      : file_(file), tick_compound_(tick_compound) {}

  std::vector<std::pair<std::shared_ptr<T>, int64_t> > ReadRange(
      const std::string& instrument_path,
      boost::posix_time::ptime start_dt,
      boost::posix_time::ptime end_dt) {
    std::vector<std::pair<std::shared_ptr<T>, int64_t> > result;

    // std::vector<boost::gregorian::date> keys;
    for (boost::gregorian::day_iterator it(start_dt.date());
         it <= end_dt.date(); ++it) {
      // keys.push_back(*it);
      auto partition_result =
          FetchRowsFromPartition(instrument_path, *it, start_dt, end_dt);
      if (partition_result.second != 0) {
        result.push_back(std::move(partition_result));
      }
    }

    return result;
  }

  std::pair<std::shared_ptr<T>, int64_t> FetchRowsFromPartition(
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
      return {std::shared_ptr<T>(), 0};
    }

    hid_t dataset = H5Dopen2(file_, path.c_str(), H5P_DEFAULT);
    std::pair<std::shared_ptr<T>, int64_t> ret = {std::shared_ptr<T>(), 0};
    hid_t attr = H5Aopen(dataset, "NROWS", H5P_DEFAULT);
    int64_t rows = 0;
    err = H5Aread(attr, H5T_NATIVE_INT64, &rows);
    H5Aclose(attr);
    if (date == start_dt.date() && date == end_dt.date()) {
      std::shared_ptr<T> pre_ticks(new T[rows], std::default_delete<T[]>());
      herr_t err = H5Dread(dataset, tick_compound_, H5S_ALL, H5S_ALL,
                           H5P_DEFAULT, pre_ticks.get());

      T start_tick{0};
      start_tick.timestamp = ptime_to_timestamp(start_dt);
      auto start_it = std::lower_bound(
          pre_ticks.get(), pre_ticks.get() + rows, start_tick,
          [](const T& l, const T& r) { return l.timestamp < r.timestamp; });

      T end_tick{0};
      end_tick.timestamp = ptime_to_timestamp(end_dt);
      auto end_it = std::upper_bound(
          pre_ticks.get(), pre_ticks.get() + rows, end_tick,
          [](const T& l, const T& r) { return l.timestamp < r.timestamp; });

      auto request_row = end_it - start_it;
      std::shared_ptr<T> ticks(new T[request_row], std::default_delete<T[]>());
      memcpy(ticks.get(), start_it, sizeof(T) * (request_row));
      ret = {std::move(ticks), request_row};
    } else if (date == start_dt.date()) {
      std::shared_ptr<T> pre_ticks(new T[rows], std::default_delete<T[]>());
      herr_t err = H5Dread(dataset, tick_compound_, H5S_ALL, H5S_ALL,
                           H5P_DEFAULT, pre_ticks.get());

      T start_tick{0};
      start_tick.timestamp = ptime_to_timestamp(start_dt);
      auto start_it = std::lower_bound(
          pre_ticks.get(), pre_ticks.get() + rows, start_tick,
          [](const T& l, const T& r) { return l.timestamp < r.timestamp; });

      auto request_row = pre_ticks.get() + rows - start_it;
      std::shared_ptr<T> ticks(new T[request_row], std::default_delete<T[]>());
      memcpy(ticks.get(), start_it, sizeof(T) * (request_row));
      ret = {std::move(ticks), request_row};
    } else if (date == end_dt.date()) {
      std::shared_ptr<T> pre_ticks(new T[rows], std::default_delete<T[]>());
      herr_t err = H5Dread(dataset, tick_compound_, H5S_ALL, H5S_ALL,
                           H5P_DEFAULT, pre_ticks.get());

      T end_tick{0};
      end_tick.timestamp = ptime_to_timestamp(end_dt);
      auto end_it = std::upper_bound(
          pre_ticks.get(), pre_ticks.get() + rows, end_tick,
          [](const T& l, const T& r) { return l.timestamp < r.timestamp; });

      auto request_row = end_it - pre_ticks.get();
      std::shared_ptr<T> ticks(new T[request_row], std::default_delete<T[]>());
      memcpy(ticks.get(), pre_ticks.get(), sizeof(T) * (request_row));
      ret = {std::move(ticks), request_row};
    } else {
      std::shared_ptr<T> ticks(new T[rows], std::default_delete<T[]>());
      herr_t err = H5Dread(dataset, tick_compound_, H5S_ALL, H5S_ALL,
                           H5P_DEFAULT, ticks.get());
      ret = {std::move(ticks), rows};
    }

    herr_t h5_ret = H5Dclose(dataset);
    return ret;
  }

 private:
  hid_t file_;
  hid_t tick_compound_;
};

#endif  // HPT_CORE_TIME_SERIES_READER_H
