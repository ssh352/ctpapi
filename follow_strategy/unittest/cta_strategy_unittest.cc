
#include "follow_strategy/cta_generic_strategy.h"
#include "follow_strategy/cta_signal.h"
#include "follow_strategy/cta_signal_dispatch.h"
#include "follow_strategy/strategy_order_dispatch.h"
#include "gtest/gtest.h"

class TestDispatchEnterOrder : public EnterOrderObserver {
 public:
  virtual void OpenOrder(const std::string& instrument,
                         const std::string& order_id,
                         OrderDirection direction,
                         double price,
                         int quantity) override {}

  virtual void CloseOrder(const std::string& instrument,
                          const std::string& order_id,
                          OrderDirection direction,
                          PositionEffect position_effect,
                          double price,
                          int quantity) override {}

  virtual void CancelOrder(const std::string& order_id) override {}
};

TEST(CTAStrategyTest, FirstTest) {}
