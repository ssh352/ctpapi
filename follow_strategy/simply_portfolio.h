#ifndef FOLLOW_STRATEGY_SIMPLY_PORTFOLIO_H
#define FOLLOW_STRATEGY_SIMPLY_PORTFOLIO_H
#include <memory>
#include <unordered_map>
#include "common/api_struct.h"
#include "hpt_core/just_qty_position.h"
#include <boost/functional/hash.hpp>

class SimplyPortfolio {
 public:
  void HandleOrder(const std::shared_ptr<OrderField>& order);

  int GetPositionCloseableQty(const std::string& instrument,
                              OrderDirection direction) const;

  int GetPositionQty(const std::string& instrument,
                     OrderDirection direction) const;

  int GetFrozenQty(const std::string& instrument,
                   OrderDirection direction) const;

  std::shared_ptr<OrderField> GetOrder(const std::string& order_id) const;

  int UnfillOpenQty(const std::string& instrument,
                    OrderDirection direction) const;

  int UnfillCloseQty(const std::string& instrument,
                     OrderDirection directon) const;

  std::vector<std::shared_ptr<OrderField> > UnfillOpenOrders(
      const std::string& instrument,
      OrderDirection direction) const;

  std::vector<std::shared_ptr<OrderField> > UnfillCloseOrders(
      const std::string& instrument,
      OrderDirection direction) const;

 private:
  struct PositionKey {
    std::string instrument;
    OrderDirection direction;
  };

  struct HashPositionKey {
    size_t operator()(const PositionKey& key) const {
      size_t seed = 0;
      boost::hash_combine(seed, key.instrument);
      boost::hash_combine(seed, key.direction);
      return seed;
    }
  };

  struct ComparePositionKey {
    bool operator()(const PositionKey& l, const PositionKey& r) const {
      return l.instrument == r.instrument && l.direction == r.direction;
    }
  };
  std::unordered_map<PositionKey,
                     JustQtyPosition,
                     HashPositionKey,
                     ComparePositionKey>
      positions_;

  std::unordered_map<std::string, std::shared_ptr<OrderField> >
      order_container_;
};

#endif  // FOLLOW_STRATEGY_SIMPLY_PORTFOLIO_H
