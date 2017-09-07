#ifndef BACKTESTING_RUNNER_MAIL_BOX_H
#define BACKTESTING_RUNNER_MAIL_BOX_H
#include <unordered_map>
#include <boost/any.hpp>
#include <typeindex>
#include <list>
#include <functional>
#include "event.h"

class BacktestingMailBox {
 public:
  BacktestingMailBox(std::list<std::function<void(void)>>* callable_queue)
      : callable_queue_(callable_queue) {}
  template <typename CLASS, typename ARG>
  void Subscribe(void (CLASS::*pfn)(ARG), CLASS* c) {
    std::function<void(ARG)> fn = std::bind(pfn, c, std::placeholders::_1);
    subscribers_.insert({typeid(ARG), std::move(fn)});
  }

  template <typename ARG>
  void Send(const ARG& arg) {
    auto range = subscribers_.equal_range(typeid(arg));
    for (auto it = range.first; it != range.second; ++it) {
      callable_queue_->push_back(std::bind(
          boost::any_cast<std::function<void(const ARG&)>>(it->second), arg));
    }
  }

 private:
  std::unordered_multimap<std::type_index, boost::any> subscribers_;
  std::list<std::function<void(void)>>* callable_queue_;
};

caf::behavior MailBoxActor(caf::event_based_actor* self,
                           caf::message_handler message_handler) {
  return message_handler;
}

class CAFMailBox {
 public:
  CAFMailBox(caf::actor_system& system) : system_(system) {}
  template <typename CLASS, typename ARG>
  void Subscribe(void (CLASS::*pfn)(ARG), CLASS* ptr) {
    if (current_subscriber_ != NULL && current_subscriber_ != ptr) {
      auto actor = system_.spawn(MailBoxActor, message_handler_);
      for (const auto& type_index : pending_type_indexs_) {
        subscribers_.insert({type_index, actor});
      }
      message_handler_.assign([=](ARG arg) { (ptr->*pfn)(arg); });
      pending_type_indexs_.clear();
    } else {
      message_handler_ =
          message_handler_.or_else([=](ARG arg) { (ptr->*pfn)(arg); });
    }

    current_subscriber_ = ptr;
    pending_type_indexs_.push_back(typeid(ARG));
  }

  template <typename ARG>
  void Send(const ARG& arg) {
    auto range = subscribers_.equal_range(typeid(arg));
    for (auto it = range.first; it != range.second; ++it) {
      caf::anon_send(it->second, arg);
    }
  }

  void FinishInitital() {
    auto actor = system_.spawn(MailBoxActor, message_handler_);
    for (const auto& type_index : pending_type_indexs_) {
      subscribers_.insert({type_index, actor});
    }
  }

 private:
  void* current_subscriber_ = NULL;
  caf::actor_system& system_;
  caf::message_handler message_handler_;
  std::vector<std::type_index> pending_type_indexs_;
  std::unordered_multimap<std::type_index, caf::actor> subscribers_;
};

#endif  // BACKTESTING_RUNNER_MAIL_BOX_H
