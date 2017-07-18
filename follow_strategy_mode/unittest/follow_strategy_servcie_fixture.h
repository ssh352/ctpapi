#ifndef FOLLOW_TRADE_UNITTEST_FOLLOW_STRATEGY_SERVCIE_FIXTURE_H
#define FOLLOW_TRADE_UNITTEST_FOLLOW_STRATEGY_SERVCIE_FIXTURE_H
#include <boost/exception/all.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/common.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/support/exception.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

#include "follow_strategy_mode/cta_generic_strategy.h"
#include "follow_strategy_mode/cta_signal.h"
#include "follow_strategy_mode/cta_signal_dispatch.h"
#include "follow_strategy_mode/logging_defines.h"
#include "follow_strategy_mode/strategy_order_dispatch.h"
#include "follow_strategy_mode/string_util.h"
#include "gtest/gtest.h"

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

extern const char kMasterAccountID[];
extern const char kSlaveAccountID[];

struct OrderInsertForTest {
  std::string instrument;
  std::string order_no;
  OrderDirection direction;
  PositionEffect position_effect;
  OrderPriceType price_type;
  double price;
  int quantity;
};

class FollowStragetyServiceFixture : public testing::Test,
                                     public EnterOrderObserver {
 public:
  typedef std::tuple<OrderInsertForTest, std::vector<std::string> > TestRetType;
  FollowStragetyServiceFixture();

  static void SetUpTestCase() {
    //     logging::add_file_log(
    //       keywords::file_name = "unittest_%N.log",
    //       keywords::format = "[%strategy_id%]:%Message%"
    //     );

    boost::shared_ptr<logging::core> core = logging::core::get();

    boost::shared_ptr<sinks::text_multifile_backend> backend =
        boost::make_shared<sinks::text_multifile_backend>();
    // Set up the file naming pattern
    backend->set_file_name_composer(sinks::file::as_file_name_composer(
        expr::stream << "logs/"
                     << "unittest_%N_" << expr::attr<std::string>("strategy_id")
                     << ".log"));

    // Wrap it into the frontend and register in the core.
    // The backend requires synchronization in the frontend.
    typedef sinks::synchronous_sink<sinks::text_multifile_backend> sink_t;
    boost::shared_ptr<sink_t> sink(new sink_t(backend));
    sink->set_formatter(expr::stream << "["
                                     << expr::attr<std::string>("strategy_id")
                                     << "]: " << expr::smessage);

    core->add_sink(sink);
  }

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          OrderPriceType price_type,
                          double price,
                          int quantity) override;

  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         OrderPriceType price_type,
                         double price,
                         int quantity) override;

  virtual void CancelOrder(const std::string& order_no) override;

 protected:
  void InitDefaultOrderExchangeId(std::string exchange_id);

  OrderInsertForTest PopOrderInsert();

  OrderData MakeMasterOrderData(const std::string& order_no,
                                OrderDirection order_direction,
                                PositionEffect position_effect,
                                OrderStatus status,
                                int filled_quantity = 0,
                                int quantity = 10,
                                double order_price = 1234.1,
                                const std::string& instrument = "abc",
                                const std::string& user_product_info = "Q7");

  OrderData MakeSlaveOrderData(
      const std::string& order_no,
      OrderDirection order_direction,
      PositionEffect position_effect,
      OrderStatus status,
      int filled_quantity = 0,
      int quantity = 10,
      double order_price = 1234.1,
      const std::string& instrument = "abc",
      const std::string& user_product_info = kStrategyUserProductInfo);

  TestRetType PushOpenOrderForMaster(
      const std::string& order_id,
      int quantity = 10,
      OrderDirection direction = OrderDirection::kBuy);

  TestRetType PushOpenOrderForSlave(
      const std::string& order_id,
      int quantity = 10,
      OrderDirection direction = OrderDirection::kBuy);

  void PushOpenOrder(const std::string& order_id,
                     int quantity = 10,
                     OrderDirection direction = OrderDirection::kBuy);

  void OpenAndFilledOrder(const std::string& order_id,
                          int quantity = 10,
                          int master_filled_quantity = 10,
                          int slave_filled_quantity = 10,
                          OrderDirection direction = OrderDirection::kBuy);

  TestRetType PushNewOpenOrderForMaster(
      const std::string& order_no = "0001",
      OrderDirection direction = OrderDirection::kBuy,
      int quantity = 10);

  TestRetType PushNewCloseOrderForMaster(
      const std::string& order_id = "0002",
      OrderDirection direction = OrderDirection::kSell,
      int quantity = 10,
      double price = 1234.1,
      PositionEffect position_effect = PositionEffect::kClose);

  TestRetType PushCloseOrderForMaster(
      const std::string& order_id = "0002",
      OrderDirection direction = OrderDirection::kSell,
      int filled_quantity = 10,
      int quantity = 10,
      double price = 1234.1,
      PositionEffect position_effect = PositionEffect::kClose);

  TestRetType PushCancelOrderForMaster(
      const std::string& order_no = "0001",
      OrderDirection direction = OrderDirection::kBuy,
      PositionEffect position_effect = PositionEffect::kOpen,
      int fill_quantity = 0,
      int quantity = 10);

  TestRetType PushNewCloseOrderForSlave(
      const std::string& order_id = "0002",
      OrderDirection direction = OrderDirection::kSell,
      int quantity = 10,
      PositionEffect position_effect = PositionEffect::kClose);

  TestRetType PushCloseOrderForSlave(
      const std::string& order_id = "0002",
      OrderDirection direction = OrderDirection::kSell,
      int filled_quantity = 10,
      int quantity = 10,
      double price = 1234.1,
      PositionEffect position_effect = PositionEffect::kClose);

  TestRetType PushCancelOrderForSlave(
      const std::string& order_no = "0001",
      OrderDirection direction = OrderDirection::kBuy,
      PositionEffect position_effect = PositionEffect::kOpen,
      int fill_quantity = 0,
      int quantity = 10);

  FollowStragetyServiceFixture::TestRetType PushOrderForMaster(
      const std::string& order_no,
      OrderDirection direction,
      PositionEffect position_effect,
      OrderStatus status,
      int filled_quantity,
      int quantity,
      double price);

  FollowStragetyServiceFixture::TestRetType PushOrderForSlave(
      const std::string& order_no,
      OrderDirection direction,
      PositionEffect position_effect,
      OrderStatus status,
      int filled_quantity,
      int quantity,
      double price);

  TestRetType PopOrderEffectForTest();

  virtual void SetUp() override;

  std::deque<OrderInsertForTest> order_inserts;
  std::vector<std::string> cancel_orders;
  std::string default_order_exchange_id_;
  std::shared_ptr<OrdersContext> master_context_;
  std::shared_ptr<OrdersContext> slave_context_;
  std::shared_ptr<CTASignal> signal_;
  std::shared_ptr<CTAGenericStrategy> cta_strategy_;
  std::shared_ptr<CTASignalDispatch> signal_dispatch_;
  StrategyOrderDispatch strategy_dispatch_;
};

#endif  // FOLLOW_TRADE_UNITTEST_FOLLOW_STRATEGY_SERVCIE_FIXTURE_H
