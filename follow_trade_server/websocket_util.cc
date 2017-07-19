#include "websocket_util.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

std::string MakePortfoilioJson(const std::string& master_account_id,
                               std::vector<AccountPortfolio> master_portfolios,
                               const std::string& slave_account_id,
                               std::vector<AccountPortfolio> slave_portfolios) {
  //   std::string instrument;
  //   OrderDirection direction;
  //   int closeable;
  //   int open;
  //   int close;
  boost::property_tree::ptree root;

  boost::property_tree::ptree master_root;

  master_root.put("account_id", master_account_id);
  {
    boost::property_tree::ptree arr;
    for (auto p : master_portfolios) {
      boost::property_tree::ptree pt;
      pt.put("instrument", p.instrument);
      pt.put("OrderDirection", p.direction == OrderDirection::kBuy ? "B" : "S");
      pt.put("closeable", p.closeable);
      pt.put("open", p.open);
      pt.put("close", p.close);
      arr.push_back(std::make_pair("", pt));
    }
    master_root.add_child("portfolios", arr);
  }

  root.add_child("master", master_root);

  boost::property_tree::ptree slave_root;

  slave_root.put("account_id", slave_account_id);

  {
    boost::property_tree::ptree arr;
    for (auto p : slave_portfolios) {
      boost::property_tree::ptree pt;
      pt.put("instrument", p.instrument);
      pt.put("OrderDirection", p.direction == OrderDirection::kBuy ? "B" : "S");
      pt.put("closeable", p.closeable);
      pt.put("open", p.open);
      pt.put("close", p.close);
      arr.push_back(std::make_pair("", pt));
    }

    slave_root.add_child("portfolios", arr);
  }

  root.add_child("slave", slave_root);

  std::stringstream s;
  boost::property_tree::write_json(s, root, false);
  return s.str();
}
