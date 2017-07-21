#ifndef FOLLOW_STRATEGY_MODE_LOGGING_DEFINES_H
#define FOLLOW_STRATEGY_MODE_LOGGING_DEFINES_H

#include <boost/log/expressions/keyword.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/exception.hpp>
#include <boost/exception/all.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_feature.hpp>
BOOST_LOG_ATTRIBUTE_KEYWORD(strategy_id, "strategy_id", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(account_id, "account_id", std::string)

#endif // FOLLOW_STRATEGY_MODE_LOGGING_DEFINES_H



