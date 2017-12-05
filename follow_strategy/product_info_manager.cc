#include "product_info_manager.h"
#include <iostream>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;
ProductInfoMananger::ProductInfoMananger() {
  try {
    pt::ptree product_info_config;
    pt::read_json("product_info.json", product_info_config);
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

const ProductInfoMananger* ProductInfoMananger::GetInstance() {
  return &boost::container::container_detail::singleton_default<
      ProductInfoMananger>::instance();
}

boost::optional<ProductInfo> ProductInfoMananger::GetProductInfo(
    const std::string& product_code) const {
  if (container_.find(product_code) == container_.end()) {
    return boost::optional<ProductInfo>();
  }
  return boost::optional<ProductInfo>(container_.at(product_code));
}
