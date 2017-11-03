#include "support_sub_account_broker.h"
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<CTPOrderField>);

SupportSubAccountBroker::SupportSubAccountBroker(caf::actor_config& cfg)
    : caf::event_based_actor(cfg), trader_api_(this) {}

caf::behavior SupportSubAccountBroker::make_behavior() {
  return {
      [=](const std::shared_ptr<CTPOrderField>& order) {
        auto it = order_id_bimap_.right.find(trader_api_.MakeOrderId(
            order->front_id, order->session_id, order->order_ref));
        if (it != order_id_bimap_.right.end()) {
          order->order_id = it->second.order_id;
          auto actor_it = sub_actors_.find(it->second.sub_account_id);
          if (actor_it != sub_actors_.end()) {
            send(actor_it->second, order);
          }
        }
      },
      [=](const CTPEnterOrder& enter_order) {
        std::string order_ref = GenerateOrderRef();
        order_id_bimap_.insert(OrderIdBimap::value_type(
            SubAccountOrderId{enter_order.strategy_id, enter_order.order_id},
            trader_api_.MakeOrderId(order_ref)));
        trader_api_.HandleInputOrder(enter_order, order_ref);
      },
  };
}

void SupportSubAccountBroker::HandleCTPRtnOrder(
    const std::shared_ptr<CTPOrderField>& order) {
  send(this, order);
}

std::string SupportSubAccountBroker::GenerateOrderRef() {
  return boost::lexical_cast<std::string>(order_seq_++);
}

void SupportSubAccountBroker::Connect(const std::string& server,
                                      const std::string& broker_id,
                                      const std::string& user_id,
                                      const std::string& password) {
  trader_api_.Connect(server, broker_id, user_id, password);
}
