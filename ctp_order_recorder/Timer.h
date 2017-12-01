/*****************************************************************************
 * Copyright [2017] [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

/**
 * Timer for kungfu system.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   March, 2017
 * Provide basic nano time and time transformation
 */

#ifndef YIJINJING_TIMER_H
#define YIJINJING_TIMER_H
#include <boost/shared_ptr.hpp>
#include <boost/container/detail/singleton.hpp>

//#include "YJJ_DECLARE.h"

const long MILLISECONDS_PER_SECOND = 1000L;
const long MICROSECONDS_PER_MILLISECOND = 1000L;
const long NANOSECONDS_PER_MICROSECOND = 1000L;

const long MICROSECONDS_PER_SECOND =
    MICROSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND;
const long NANOSECONDS_PER_MILLISECOND =
    NANOSECONDS_PER_MICROSECOND * MICROSECONDS_PER_MILLISECOND;
const uint64_t NANOSECONDS_PER_SECOND =
    NANOSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND;

const int SECONDS_PER_MINUTE = 60L;
const int MINUTES_PER_HOUR = 60L;
const int HOURS_PER_DAY = 24L;
const int SECONDS_PER_HOUR = SECONDS_PER_MINUTE * MINUTES_PER_HOUR;

const uint64_t MILLISECONDS_PER_MINUTE =
    MILLISECONDS_PER_SECOND * SECONDS_PER_MINUTE;
const uint64_t NANOSECONDS_PER_MINUTE =
    NANOSECONDS_PER_SECOND * SECONDS_PER_MINUTE;
const uint64_t NANOSECONDS_PER_HOUR = NANOSECONDS_PER_SECOND * SECONDS_PER_HOUR;
const uint64_t NANOSECONDS_PER_DAY = NANOSECONDS_PER_HOUR * HOURS_PER_DAY;

/**
 * timer for nanosecond, main class
 */
class NanoTimer {
 public:
  /** return current nano time: unix-timestamp * 1e9 + nano-part */
  uint64_t getNano();
  /** singleton */
  static NanoTimer* getInstance();

 private:
  friend struct boost::container::container_detail::singleton_default<
      NanoTimer>;
  NanoTimer();
  /** singleton */
  // static boost::shared_ptr<NanoTimer> m_ptr;
  /** object to be updated every time called */
  long long int secDiff;
};

/**
 * util function to utilize NanoTimer
 * @return current nano time in long (unix-timestamp * 1e9 + nano-part)
 */
inline long getNanoTime() {
  return NanoTimer::getInstance()->getNano();
}

/**
 * dump long time to string with format
 * @param nano nano time in long
 * @param format eg: %Y%m%d-%H:%M:%S
 * @return string-formatted time
 */
inline std::string parseNano(time_t nano, const char* format) {
  if (nano <= 0)
    return std::string("NULL");
  nano /= NANOSECONDS_PER_SECOND;
  struct tm* dt;
  char buffer[30];
  dt = localtime(&nano);
  strftime(buffer, sizeof(buffer), format, dt);
  return std::string(buffer);
}

/**
 * dump long time to struct tm
 * @param nano nano time in long
 * @return ctime struct
 */
inline struct tm parseNano(long nano) {
  time_t sec_num = nano / NANOSECONDS_PER_SECOND;
  return *localtime(&sec_num);
}

#endif  // YIJINJING_TIMER_H