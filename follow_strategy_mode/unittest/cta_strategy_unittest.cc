
#include "follow_strategy_mode/cta_generic_strategy.h"
#include "follow_strategy_mode/cta_signal.h"
#include "follow_strategy_mode/cta_signal_dispatch.h"
#include "follow_strategy_mode/strategy_order_dispatch.h"
#include "gtest/gtest.h"

class TestDispatchEnterOrder : public EnterOrderObserver {
 public:
  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_no,
                         OrderDirection direction,
                         OrderPriceType price_type,
                         double price,
                         int quantity) override {
  }

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_no,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          OrderPriceType price_type,
                          double price,
                          int quantity) override {
  }

  virtual void CancelOrder(const std::string& order_no) override {
  }
};

TEST(CTAStrategyTest, FirstTest) {
  CTASignal signal;
  CTASignalDispatch signal_dispath(&signal);
  CTAGenericStrategy cta_strategy;
  StrategyOrderDispatch strategy_dispatch;
  cta_strategy.Subscribe(&strategy_dispatch);
  signal_dispath.SubscribeEnterOrderObserver(&cta_strategy);
  TestDispatchEnterOrder test_dispatch_enter_order;
  strategy_dispatch.SubscribeEnterOrderObserver(&test_dispatch_enter_order);
}
