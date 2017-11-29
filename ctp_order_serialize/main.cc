#include <iostream>
#include <fstream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/variant.hpp>
#include "caf/all.hpp"
#include "ctp_order_serialization.h"
#include "thost_field_fusion_adapt.h"

class config : public caf::actor_system_config {
 public:
  config() {}
};

class ThostFieldVistor : public boost::static_visitor<void> {
 public:
  void operator()(CThostFtdcOrderField& order) const { 
    std::cout << "Order:" << order.ActiveTime << "\n"; 
  }

  void operator()(CThostFtdcTradeField& order) const { 
    //std::cout << "Trade:" << order.TradeTime << "\n"; 
  }
};

void load() {
  std::ifstream file("181006.bin", std::ios_base::binary);
  //std::ifstream file("120301760.bin", std::ios_base::binary);

  boost::archive::binary_iarchive ia(file);
  boost::variant<CThostFtdcOrderField, CThostFtdcTradeField> v;
  try {
    while (true) {
      boost::serialization::load(ia, v, 0);
      boost::apply_visitor(ThostFieldVistor(), v);
    }
  } catch (boost::archive::archive_exception& err) {
    std::cout << "Done" << err.what() << "\n";
  }
}

int caf_main(caf::actor_system& system, const config& cfg) {
  // auto actor =
  // system.spawn<CtpOrderSerialization>("tcp://ctp1-front3.citicsf.com:41205",
  //                                    "66666", "120301760", "140616");
  // auto actor =
  // system.spawn<CtpOrderSerialization>("tcp://101.231.3.125:41205",
  //                                                 "8888", "181006",
  //                                                 "140616");
  load();

  std::string input;
  while (std::cin >> input) {
    if (input == "exit")
      break;
  }
  return 0;
}

CAF_MAIN()