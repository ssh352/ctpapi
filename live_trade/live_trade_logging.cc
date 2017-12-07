#include "live_trade_logging.h"
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
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include "common_util.h"

BOOST_LOG_ATTRIBUTE_KEYWORD(log_tag, "log_tag", std::string)

void InitLogging() {
  namespace logging = boost::log;
  namespace attrs = boost::log::attributes;
  namespace src = boost::log::sources;
  namespace sinks = boost::log::sinks;
  namespace expr = boost::log::expressions;
  namespace keywords = boost::log::keywords;
  boost::shared_ptr<logging::core> core = logging::core::get();

  //{
  //  auto console_sink = logging::add_console_log();
  //  console_sink->set_formatter(
  //      expr::format("[%1%]:%2%") %
  //      expr::attr<boost::posix_time::ptime>("TimeStamp") % expr::smessage);
  //  core->add_sink(console_sink);
  //}

  {
    boost::shared_ptr<sinks::text_multifile_backend> backend =
        boost::make_shared<sinks::text_multifile_backend>();
    // Set up the file naming pattern
    backend->set_file_name_composer(sinks::file::as_file_name_composer(
        expr::stream << "logs/" << expr::attr<std::string>("log_tag") << "_"
                     << DateTimeSubfix() << ".log"));

    // Wrap it into the frontend and register in the core.
    // The backend requires synchronization in the frontend.
    typedef sinks::asynchronous_sink<sinks::text_multifile_backend> sink_t;
    boost::shared_ptr<sink_t> sink(new sink_t(backend));
    sink->set_formatter(expr::format("[%1%]:%2%") %
                        expr::attr<boost::posix_time::ptime>("TimeStamp") %
                        expr::smessage);
    core->add_sink(sink);
  }

  boost::log::add_common_attributes();
  // core->set_logging_enabled(enable_logging);
}
