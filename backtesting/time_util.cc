#include "time_util.h"

timestamp_t ptime_to_timestamp(boost::posix_time::ptime timestamp) {
  using namespace boost::posix_time;
  using namespace boost::gregorian;
  ptime epoch(date(1970, 1, 1));
  return ((timestamp - epoch).ticks()) /
         (time_duration::ticks_per_second() / 1000);
}
