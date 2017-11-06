#include "ctp_position_amount.h"
#include <algorithm>
#include <boost/assert.hpp>

void CTPPositionAmount::Frozen(int qty, CTPPositionEffect /*position_effect*/) {
  BOOST_ASSERT(Closeable() >= qty);
  int yesterday = std::min<int>(YesterdayCloseable(), qty);
  if (yesterday > 0) {
    int leaves_frozen = YesterdayCloseable() - yesterday;
    frozen_yesterday_ += yesterday;
    if (leaves_frozen > 0) {
      BOOST_ASSERT(TodayCloseable() >= leaves_frozen);
      frozen_today_ += leaves_frozen;
    }
  } else {
    BOOST_ASSERT(TodayCloseable() >= qty);
    frozen_today_ += qty;
  }
}

void CTPPositionAmount::Unfrozen(int qty,
                                 CTPPositionEffect /*position_effect*/) {
  BOOST_ASSERT(qty <= frozen_today_ + frozen_yesterday_);
  int yesterday = std::min<int>(frozen_yesterday_, qty);
  if (yesterday > 0) {
    int leaves_unfrozen = frozen_yesterday_ - yesterday;
    if (leaves_unfrozen > 0) {
      BOOST_ASSERT(frozen_today_ >= leaves_unfrozen);
      frozen_today_ -= leaves_unfrozen;
    }
  } else {
    BOOST_ASSERT(frozen_today_ >= qty);
    frozen_today_ -= qty;
  }
}

void CTPPositionAmount::CloseTraded(int qty,
                                    CTPPositionEffect position_effect) {
  int remove_qty_yesterday = std::min<int>(yesterday_, qty);
  yesterday_ -= remove_qty_yesterday;
  int leaves_qty = qty - remove_qty_yesterday;
  today_ -= leaves_qty;
  frozen_today_ -= qty;
}

void CTPPositionAmount::OpenTraded(int qty) {
  today_ += qty;
}

int CTPPositionAmount::Total() const {
  return yesterday_ + today_;
}

int CTPPositionAmount::Closeable() const {
  return yesterday_ + today_ - (frozen_yesterday_ + frozen_today_);
}

int CTPPositionAmount::YesterdayCloseable() const {
  return yesterday_ - frozen_yesterday_;
}

int CTPPositionAmount::TodayCloseable() const {
  return today_ - frozen_today_;
}

void CTPPositionAmount::Init(int yesterday,
                            int frozen_yesterday,
                            int today,
                            int frozen_today) {
  yesterday_ = yesterday;
  frozen_yesterday_ = frozen_yesterday;
  today_ = today;
  frozen_today_ = frozen_today;
}

void CloseTodayAwareCTPPositionAmount::CloseTraded(
    int qty,
    CTPPositionEffect position_effect) {
  if (position_effect == CTPPositionEffect::kCloseToday) {
    BOOST_ASSERT(TodayCloseable() > qty);
    today_ -= qty;
    frozen_today_ -= qty;
  } else if (position_effect == CTPPositionEffect::kClose) {
    BOOST_ASSERT(YesterdayCloseable() > qty);
    yesterday_ -= qty;
    frozen_yesterday_ -= qty;
  } else {
    BOOST_ASSERT(false);
  }
}

void CloseTodayAwareCTPPositionAmount::Frozen(
    int qty,
    CTPPositionEffect position_effect) {
  if (position_effect == CTPPositionEffect::kCloseToday) {
    BOOST_ASSERT(TodayCloseable() >= qty);
    frozen_today_ += qty;
  } else if (position_effect == CTPPositionEffect::kClose) {
    BOOST_ASSERT(YesterdayCloseable() >= qty);
    frozen_yesterday_ += qty;
  } else {
    BOOST_ASSERT(false);
  }
}

void CloseTodayAwareCTPPositionAmount::Unfrozen(
    int qty,
    CTPPositionEffect position_effect) {
  if (position_effect == CTPPositionEffect::kCloseToday) {
    BOOST_ASSERT(frozen_today_ >= qty);
    frozen_today_ -= qty;
  } else if (position_effect == CTPPositionEffect::kClose) {
    BOOST_ASSERT(frozen_yesterday_ >= qty);
    frozen_yesterday_ -= qty;
  } else {
    BOOST_ASSERT(false);
  }
}
