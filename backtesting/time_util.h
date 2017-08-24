#ifndef BACKTESTING_TIME_UTIL_H
#define BACKTESTING_TIME_UTIL_H
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "common/api_data_type.h"

TimeStamp ptime_to_timestamp(boost::posix_time::ptime timestamp);

#endif  // BACKTESTING_TIME_UTIL_H