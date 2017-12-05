#ifndef STRATEGYS_PRODUCT_INFO_MANAGER_H
#define STRATEGYS_PRODUCT_INFO_MANAGER_H
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/container/detail/singleton.hpp>
#include <boost/property_tree/ptree.hpp>
#include "common/api_struct.h"

class ProductInfoMananger {
 public:
  ProductInfoMananger();

  
  void LoadFile(const std::string& file);

  void Load(const boost::property_tree::ptree& pt);
  
  boost::optional<ProductInfo> GetProductInfo(
      const std::string& product_code) const;

 private:
  friend struct boost::container::container_detail::singleton_default<
      ProductInfoMananger>;
  std::unordered_map<std::string, ProductInfo> container_;
};

#endif  // STRATEGYS_PRODUCT_INFO_MANAGER_H
