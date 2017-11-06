#ifndef CTP_BROKER_CTP_POSITION_AMOUNT_H
#define CTP_BROKER_CTP_POSITION_AMOUNT_H
#include "common/api_data_type.h"

class CTPPositionAmount {
 public:
  void Init(int yesterday, int frozen_yesterday, int today, int frozen_today);

  int Closeable() const;

  int YesterdayCloseable() const;

  int TodayCloseable() const;

  int Total() const;

  void OpenTraded(int qty);

  virtual void CloseTraded(int qty, CTPPositionEffect position_effect);

  virtual void Frozen(int qty, CTPPositionEffect position_effect);

  virtual void Unfrozen(int qty, CTPPositionEffect position_effect);

 protected:
  int yesterday_ = 0;
  int today_ = 0;
  int frozen_today_ = 0;
  int frozen_yesterday_;
  int opening = 0;
};

class CloseTodayAwareCTPPositionAmount : public CTPPositionAmount {
 public:
  virtual void CloseTraded(int qty, CTPPositionEffect position_effect) override;

  virtual void Frozen(int qty, CTPPositionEffect position_effect) override;

  virtual void Unfrozen(int qty, CTPPositionEffect position_effect) override;
};

#endif  // CTP_BROKER_CTP_POSITION_AMOUNT_H
