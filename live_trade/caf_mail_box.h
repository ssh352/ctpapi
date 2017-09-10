#ifndef HPT_CORE_CAF_MAIL_BOX_H
#define HPT_CORE_CAF_MAIL_BOX_H
#include <tuple>
#include <typeinfo>

caf::behavior MailBoxActor(caf::event_based_actor* self,
                           caf::message_handler message_handler) {
  return message_handler;
}

class CAFMailBox {
 public:
  CAFMailBox(caf::actor_system& system) : system_(system) {}
  template <typename CLASS, typename... ARG>
  void Subscribe(void (CLASS::*pfn)(ARG...), CLASS* ptr) {
    if (current_subscriber_ != NULL && current_subscriber_ != ptr) {
      auto actor = system_.spawn(MailBoxActor, message_handler_);
      for (const auto& type_index : pending_type_indexs_) {
        subscribers_.insert({type_index, actor});
      }
      message_handler_.assign([=](ARG... arg) { (ptr->*pfn)(arg...); });
      pending_type_indexs_.clear();
    } else {
      message_handler_ =
          message_handler_.or_else([=](ARG... arg) { (ptr->*pfn)(arg...); });
    }

    current_subscriber_ = ptr;
    pending_type_indexs_.push_back(typeid(std::tuple<ARG...>));
  }

  template <typename... ARG>
  void Send(ARG... arg) {
    auto range = subscribers_.equal_range(typeid(std::tuple<ARG...>));
    for (auto it = range.first; it != range.second; ++it) {
      caf::anon_send(it->second, arg...);
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

#endif  // HPT_CORE_CAF_MAIL_BOX_H
