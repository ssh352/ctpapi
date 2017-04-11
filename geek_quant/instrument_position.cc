#include "instrument_position.h"

std::vector<OrderQuantity> InstrumentPosition::GetQuantitysWithOrderIds(
    std::vector<std::string> orders) {
  return {{"", 0, 0}};
}

int InstrumentPosition::GetPositionCloseableQuantity(OrderDirection direction) {
  std::vector<Position>& positions =
      direction == OrderDirection::kBuy ? buy_positions_ : sell_positions_;

  return std::accumulate(positions.begin(), positions.end(), 0,
                         [=](int val, auto position) {
                           return val + position.closeable_quantity;
                         });
}
