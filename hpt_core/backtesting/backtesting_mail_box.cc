#include "backtesting_mail_box.h"

void BacktestingMailBox::Send(bft::Message message) {
  auto range = subscribers_.equal_range(message.TypeIndex());
  for (auto it = range.first; it != range.second; ++it) {
    callable_queue_->push_back(
        std::bind(it->second.message_handler(), message.caf_message()));
  }
}

void BacktestingMailBox::Subscribe(bft::MessageHandler handler) {
  for (auto type_index : handler.TypeIndexs()) {
    subscribers_.insert({type_index, handler});
  }
}

BacktestingMailBox::BacktestingMailBox(
    std::list<std::function<void(void)>>* callable_queue)
    : callable_queue_(callable_queue) {}
