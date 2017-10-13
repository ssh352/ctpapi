#include "time_util.h"

TimeStamp ptime_to_timestamp(boost::posix_time::ptime timestamp) {
  using namespace boost::posix_time;
  using namespace boost::gregorian;
  ptime epoch(date(1970, 1, 1));
  return ((timestamp - epoch).ticks()) /
         (time_duration::ticks_per_second() / 1000);
}

boost::posix_time::ptime TimeStampToPtime(TimeStamp timestamp)
{
  return boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1),
    boost::posix_time::milliseconds(timestamp));
}
