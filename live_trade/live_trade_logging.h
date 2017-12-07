#ifndef LIVE_TRADE_LIVE_TRADE_LOGGING_H
#define LIVE_TRADE_LIVE_TRADE_LOGGING_H
#include <boost/log/common.hpp>
#include <boost/log/sources/severity_logger.hpp>

enum class SeverityLevel {
  kInfo,
  kWarning,
  kError,
};

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(
    BLog,
    boost::log::sources::severity_logger<SeverityLevel>)


void InitLogging();

#endif  // LIVE_TRADE_LIVE_TRADE_LOGGING_H
