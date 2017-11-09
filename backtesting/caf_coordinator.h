#ifndef BACKTESTING_CAF_COORDINATOR_H
#define BACKTESTING_CAF_COORDINATOR_H
#include "caf/all.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "caf_config.h"
namespace pt = boost::property_tree;
caf::behavior Coordinator(caf::event_based_actor* self,
                          pt::ptree* instrument_infos_pt,
                          pt::ptree* strategy_config_pt,
                          const config* cfg);

#endif  // BACKTESTING_CAF_COORDINATOR_H
