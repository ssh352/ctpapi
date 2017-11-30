#include <iostream>
#include <fstream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/variant.hpp>
#include "caf/all.hpp"
#include "ctp_order_serialization.h"
#include "thost_field_fusion_adapt.h"
#include "json.h"

class config : public caf::actor_system_config {
 public:
  std::string server;
  std::string broker_id;
  std::string user_id;
  std::string password;
  config() {
    opt_group{custom_options_, "ctp"}
        .add(server, "server", "ctp front server")
        .add(broker_id, "broker_id", "broker id")
        .add(user_id, "user_id", "user id")
        .add(password, "password", "password");
  }
};

class ThostFieldVistor : public boost::static_visitor<void> {
 public:
  ThostFieldVistor(std::ofstream& file) : file_(file) {}
  void operator()(CThostFtdcOrderField& order) const {
    file_ << "\n{\"CThostFtdcOrderField\":";
    ToJson(file_, order);
    file_ << "}";
  }

  void operator()(CThostFtdcTradeField& order) const {
    file_ << "\n{\"CThostFtdcTradeField\":";
    ToJson(file_, order);
    file_ << "}";
  }

 private:
  std::ofstream& file_;
};

void load(const std::string& user_id) {
  std::ifstream file(user_id + ".bin", std::ios_base::binary);
  std::ofstream out_json_file(user_id + ".json", std::ios_base::trunc);
  out_json_file << std::boolalpha;
  out_json_file << "[\n";
  boost::archive::binary_iarchive ia(file);
  boost::variant<CThostFtdcOrderField, CThostFtdcTradeField> v;
  try {
    bool fiset_item = true;
    while (true) {
      boost::serialization::load(ia, v, 0);
      if (!fiset_item) {
        out_json_file << ",\n";
      }
      boost::apply_visitor(ThostFieldVistor(out_json_file), v);
      fiset_item = false;
    }
  } catch (boost::archive::archive_exception& err) {
    std::cout << "Done" << err.what() << "\n";
  }
  out_json_file << "]\n";
}

int caf_main(caf::actor_system& system, const config& cfg) {
  auto actor = system.spawn<CtpOrderSerialization>(cfg.server, cfg.broker_id,
                                                   cfg.user_id, cfg.password);
  auto f = caf::make_function_view(actor);
  f(RequestTodayFlowAtom::value);
  load(cfg.user_id);

  return 0;
}

CAF_MAIN()