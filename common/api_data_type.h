#ifndef COMMON_API_DATA_TYPE_H
#define COMMON_API_DATA_TYPE_H
#include <stdint.h>

enum class OrderEventType {
  kIgnore,
  kNewOpen,
  kNewClose,
  kOpenTraded,
  kCloseTraded,
  kCanceled,
};

enum class OrderDirection { kUndefine, kBuy, kSell };

enum class PositionEffect { kUndefine, kOpen, kClose};

enum class CTPPositionEffect { kUndefine, kOpen, kClose, kCloseToday};

enum class OrderStatus {
  kActive,
  kAllFilled,
  kCanceled,
  kInputRejected,
  kCancelRejected,
};

typedef int64_t TimeStamp;

typedef char InstrumentIDType[31];

typedef char OrderIDType[21];

enum class CommissionType { kFixed, kRate };

enum class TradingTime { kUndefined, kDay, kNight };

#endif  // COMMON_API_DATA_TYPE_H
