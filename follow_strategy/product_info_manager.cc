#include "product_info_manager.h"
#include <iostream>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;
ProductInfoMananger::ProductInfoMananger() {}
void ProductInfoMananger::LoadFile(const std::string& file) {
  try {
    pt::ptree product_info_config;
    pt::read_json(file, product_info_config);
    for (const auto& pt : product_info_config) {
      container_.insert_or_assign(
          pt.first, ProductInfo{pt.second.get<double>("TickSize"),
                                pt.second.get<std::string>("Exchange")});
    }
  } catch (pt::ptree_error& err) {
    std::cout << err.what() << "\n";
    return;
  }
}

void ProductInfoMananger::Load(const boost::property_tree::ptree& pt) {
  try {
    for (const auto& child : pt) {
      container_.insert_or_assign(
          child.first, ProductInfo{child.second.get<double>("TickSize"),
                                   child.second.get<std::string>("Exchange")});
    }
  } catch (pt::ptree_error& err) {
    std::cout << err.what() << "\n";
    return;
  }
}

boost::optional<ProductInfo> ProductInfoMananger::GetProductInfo(
    const std::string& product_code) const {
  if (container_.find(product_code) == container_.end()) {
    return boost::optional<ProductInfo>();
  }
  return boost::optional<ProductInfo>(container_.at(product_code));
}
