#ifndef BACKTESTING_CAF_COORDINATOR_H
#define BACKTESTING_CAF_COORDINATOR_H
#include "caf/all.hpp"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include "caf_config.h"
#include "yaml-cpp/yaml.h"
namespace pt = boost::property_tree;
caf::behavior Coordinator(caf::event_based_actor* self,
                          YAML::Node* cfg,
                          std::string out_dir);

#endif  // BACKTESTING_CAF_COORDINATOR_H
