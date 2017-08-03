#ifndef FOLLOW_TRADE_DEFINES_H
#define FOLLOW_TRADE_DEFINES_H

enum class OrderEventType {
  kIgnore,
  kNewOpen,
  kNewClose,
  kOpenTraded,
  kCloseTraded,
  kCanceled,
};
#endif  // FOLLOW_TRADE_DEFINES_H
