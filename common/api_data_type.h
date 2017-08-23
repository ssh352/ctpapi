﻿#ifndef COMMON_API_DATA_TYPE_H
#define COMMON_API_DATA_TYPE_H

enum class OrderEventType {
  kIgnore,
  kNewOpen,
  kNewClose,
  kOpenTraded,
  kCloseTraded,
  kCanceled,
};

enum class OrderDirection { kUndefine, kBuy, kSell };

enum class PositionEffect { kUndefine, kOpen, kClose, kCloseToday };

enum class OrderStatus {
  kActive,
  kAllFilled,
  kCanceled,
  kInputRejected,
  kCancelRejected,
};

typedef int64_t timestamp_t;

typedef char InstrumentIDType[31];

#endif  // COMMON_API_DATA_TYPE_H
