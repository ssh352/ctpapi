#ifndef HDFDEMO001_TICK_SERIES_DATA_BASE_H
#define HDFDEMO001_TICK_SERIES_DATA_BASE_H
#include <vector>
#include <memory>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "hdf5.h"
#include "common/api_struct.h"

class TickSeriesDataBase {
 public:
  TickSeriesDataBase(const char* hdf_file);
  ~TickSeriesDataBase();

  std::vector<std::pair<std::unique_ptr<Tick[]>, int64_t> > ReadRange(
      const std::string& instrument_path,
      boost::posix_time::ptime start_dt,
      boost::posix_time::ptime end_dt);

 private:
  std::pair<std::unique_ptr<Tick[]>, int64_t> FetchRowsFromPartition(
      const std::string& instrument_path,
      boost::gregorian::date date,
      boost::posix_time::ptime start_dt,
      boost::posix_time::ptime end_dt) const;

  hid_t file_;
  hid_t tick_compound_;
};

#endif  // HDFDEMO001_TICK_SERIES_DATA_BASE_H
