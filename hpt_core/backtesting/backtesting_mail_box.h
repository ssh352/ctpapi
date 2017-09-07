#ifndef BACKTESTING_RUNNER_MAIL_BOX_H
#define BACKTESTING_RUNNER_MAIL_BOX_H
#include <unordered_map>
#include <boost/any.hpp>
#include <typeindex>
#include <list>
#include <functional>

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

#endif  // BACKTESTING_RUNNER_MAIL_BOX_H
