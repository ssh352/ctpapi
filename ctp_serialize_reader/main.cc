#include <iostream>
#include <fstream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/variant.hpp>
#include "common/serialization_util.h"
#include "json.h"

class ThostFieldVistor : public boost::static_visitor<void> {
 public:
  ThostFieldVistor(uint64_t timestamp, std::ofstream& file)
      : timestamp_(timestamp), file_(file) {}
  void operator()(CThostFtdcOrderField& order) const {
    file_ << "\n{\"OrderField\":{";
    file_ << "\"TimeStamp\": " << timestamp_ << ",";
    file_ << "\n\"CThostFtdcOrderField\":";
    ToJson(file_, order);
    file_ << "}}";
  }

  void operator()(CThostFtdcTradeField& order) const {
    file_ << "\n{\"TradeField\":{";
    file_ << "\"TimeStamp\": " << timestamp_ << ",";
    file_ << "\n\"CThostFtdcTradeField\":";
    ToJson(file_, order);
    file_ << "}}";
  }

 private:
  std::ofstream& file_;
  uint64_t timestamp_;
};

void FtdcSerializationToJson(const std::string& file_name) {
  std::ifstream file(file_name, std::ios_base::binary);
  std::ofstream out_json_file(file_name + ".json", std::ios_base::trunc);
  out_json_file << std::boolalpha;
  out_json_file << "[\n";
  boost::archive::binary_iarchive ia(file);
  boost::variant<CThostFtdcOrderField, CThostFtdcTradeField> v;
  try {
    bool fiset_item = true;
    while (true) {
      uint64_t timestamp = 0;
      ia >> timestamp;
      boost::serialization::load(ia, v, 0);
      if (!fiset_item) {
        out_json_file << ",\n";
      }
      boost::apply_visitor(ThostFieldVistor(timestamp, out_json_file), v);
      fiset_item = false;
    }
  } catch (boost::archive::archive_exception& err) {
    std::cout << "Info:" << err.what() << "\n";
  }
  out_json_file << "]\n";
}

void OrderFieldSerializationToJson(const std::string& file_name) {
  std::ifstream file(file_name, std::ios_base::binary);
  std::ofstream out_json_file(file_name + ".json", std::ios_base::trunc);
  out_json_file << std::boolalpha;
  out_json_file << "[\n";
  boost::archive::binary_iarchive ia(file);
  OrderField order;
  try {
    bool fiset_item = true;
    while (true) {
      ia >> order;
      if (!fiset_item) {
        out_json_file << ",\n";
      }
      ToJson(out_json_file, order);
      fiset_item = false;
    }
  } catch (boost::archive::archive_exception& err) {
    std::cout << "Info: " << err.what() << "\n";
  }
  out_json_file << "]\n";
}

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "ERROR! Please use ctp_serialize_reader.exe file [-ctp]";
    return 1;
  }

  if (argc == 3 && strcmp("-ctp", argv[2]) == 0) {
    FtdcSerializationToJson(argv[1]);
  } else {
    OrderFieldSerializationToJson(argv[1]);
  }
  return 0;
}
