#ifndef BACKTESTING_EVENT_H
#define BACKTESTING_EVENT_H

class AbstractEvent {
 public:
  virtual void Do() = 0;
};

#endif // BACKTESTING_EVENT_H



