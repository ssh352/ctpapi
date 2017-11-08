#include "support_sub_account_broker.h"
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include "caf_atom_defines.h"

SupportSubAccountBroker::SupportSubAccountBroker(
    caf::actor_config& cfg,
    LiveTradeMailBox* mail_box,
    const std::vector<std::pair<std::string, caf::actor> >& sub_accounts)
    : caf::event_based_actor(cfg), trader_api_(this), mail_box_(mail_box) {
  mail_box_->Subscribe(typeid(std::tuple<std::string, CTPEnterOrder>), this);
  mail_box_->Subscribe(typeid(std::tuple<std::string, std::string>), this);

  for (const auto& sub_account : sub_accounts) {
    sub_actors_.insert({sub_account.first, sub_account.second});
  }
}

caf::behavior SupportSubAccountBroker::make_behavior() {
  return {
      [=](CtpConnectAtom, const std::string& server,
          const std::string& broker_id, const std::string& user_id,
          const std::string& password) {
        trader_api_.Connect(server, broker_id, user_id, password);
      },
      [=](const std::shared_ptr<CTPOrderField>& order) {
        auto it = order_id_bimap_.right.find(order->order_id);
        if (it != order_id_bimap_.right.end()) {
          order->order_id = it->second.order_id;
          auto actor_it = sub_actors_.find(it->second.sub_account_id);
          if (actor_it != sub_actors_.end()) {
            send(actor_it->second, order);
          }
        }
      },
      [=](const std::string& account_id, const CTPEnterOrder& enter_order) {
        std::string order_ref = GenerateOrderRef();
        order_id_bimap_.insert(OrderIdBimap::value_type(
            SubAccountOrderId{account_id, enter_order.order_id},
            trader_api_.MakeCtpUniqueOrderId(order_ref)));
        trader_api_.InputOrder(enter_order, order_ref);
      },
      [=](const std::string& account_id, const CTPCancelOrder& cancel) {
        auto it = order_id_bimap_.left.find(
            SubAccountOrderId{account_id, cancel.order_id});
        BOOST_ASSERT(it != order_id_bimap_.left.end());
        CThostFtdcInputOrderActionField order = {0};
        order.ActionFlag = THOST_FTDC_AF_Delete;
        order.FrontID = cancel.front_id;
        order.SessionID = cancel.session_id;
        strcpy(order.OrderRef, cancel.order_ref.c_str());
        strcpy(order.ExchangeID, cancel.exchange_id.c_str());
        strcpy(order.OrderSysID, cancel.order_sys_id.c_str());
        trader_api_.CancelOrder(std::move(order));
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

void SupportSubAccountBroker::HandleCTPTradeOrder(const std::string& order_id,
                                                  double trading_price,
                                                  int trading_qty,
                                                  TimeStamp timestamp) {}
