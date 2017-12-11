#ifndef BACKTESTING_RUNNER_MAIL_BOX_H
#define BACKTESTING_RUNNER_MAIL_BOX_H
#include <unordered_map>
#include <boost/any.hpp>
#include <typeindex>
#include <list>
#include <functional>
#include "bft_core/channel_delegate.h"

class BacktestingMailBox : public bft::ChannelDelegate {
 public:
  BacktestingMailBox(std::list<std::function<void(void)>>* callable_queue);

  virtual void Subscribe(bft::MessageHandler handler) override;

  virtual void Send(bft::Message message) override;

 private:
  std::unordered_multimap<std::type_index, bft::MessageHandler> subscribers_;
  std::list<std::function<void(void)>>* callable_queue_;
};

#endif  // BACKTESTING_RUNNER_MAIL_BOX_H
