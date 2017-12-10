#include "sub_account_broker.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/common.hpp>
#include <boost/log/attributes.hpp>
#include "caf_common/caf_atom_defines.h"
#include "bft_core/make_message.h"

CAFSubAccountBroker::CAFSubAccountBroker(
    caf::actor_config& cfg,
    LiveTradeSystem* live_trade_system,
    int env_id,
    ProductInfoMananger* product_info_mananger,
    std::unordered_set<std::string> close_today_cost_of_product_codes,
    std::string broker_id,
    std::string account_id)
    : caf::event_based_actor(cfg),
      live_trade_system_(live_trade_system),
      env_id_(env_id),
      product_info_mananger_(product_info_mananger),
      close_today_cost_of_product_codes_(close_today_cost_of_product_codes),
      broker_id_(std::move(broker_id)),
      account_id_(std::move(account_id)) {
  log_.add_attribute(
      "log_tag", boost::log::attributes::constant<std::string>(account_id_));
  InitMakeBehavior();
  for (const auto& type_index : message_handler_.TypeIndexs()) {
    live_trade_system_->Subscribe(env_id_, type_index, this);
  }
}

std::string CAFSubAccountBroker::GenerateOrderId() {
  return boost::lexical_cast<std::string>(order_seq_++);
}

void CAFSubAccountBroker::ReturnOrderField(
    const std::shared_ptr<OrderField>& order) {
  live_trade_system_->Send(env_id_, bft::MakeMessage(order));
}

void CAFSubAccountBroker::HandleCancelOrder(
    const CTPCancelOrder& cancel_order) {
  live_trade_system_->SendToNamed(broker_id_,
                                  bft::MakeMessage(account_id_, cancel_order));
  BOOST_LOG(log_) << "[SEND Cancel Order]"
                  << "(Id)" << cancel_order.order_id << ", (SysId)"
                  << cancel_order.order_sys_id;
}

void CAFSubAccountBroker::HandleEnterOrder(const CTPEnterOrder& enter_order) {
  live_trade_system_->SendToNamed(broker_id_,
                                  bft::MakeMessage(account_id_, enter_order));
  BOOST_LOG(log_) << "[SEND Enter Order]"
                  << "(Id)" << enter_order.order_id << ", (I)"
                  << enter_order.instrument << ", (BS)"
                  << static_cast<int>(enter_order.direction) << ", (OC)"
                  << static_cast<int>(enter_order.position_effect) << ", (P)"
                  << enter_order.price << ", (Q)" << enter_order.qty;
}

void CAFSubAccountBroker::MakeCtpInstrumentBrokerIfNeed(
    const std::string& instrument) {
  if (instrument_brokers_.find(instrument) != instrument_brokers_.end()) {
    return;
  }
  auto broker = std::make_unique<CTPInstrumentBroker>(
      this, instrument, false,
      std::bind(&CAFSubAccountBroker::GenerateOrderId, this));
  std::string instrument_lower = instrument;
  boost::algorithm::to_lower(instrument_lower);
  std::string instrument_code =
      instrument_lower.substr(0, instrument_lower.find_first_of("0123456789"));
  bool close_today_cost = false;
  bool close_today_aware = false;
  auto product_info = product_info_mananger_->GetProductInfo(instrument_code);
  if (product_info && product_info->exchange == "sc") {
    close_today_aware = true;
  }

  if (close_today_cost_of_product_codes_.find(instrument_lower) !=
          close_today_cost_of_product_codes_.end() ||
      close_today_cost_of_product_codes_.find(instrument_code) !=
          close_today_cost_of_product_codes_.end()) {
    close_today_cost = true;
  }

  if (close_today_cost && close_today_aware) {
    broker->SetPositionEffectStrategy<
        CloseTodayCostCTPPositionEffectStrategy,
        CloseTodayAwareCTPPositionEffectFlagStrategy>();
  } else if (!close_today_cost && close_today_aware) {
    broker->SetPositionEffectStrategy<
        GenericCTPPositionEffectStrategy,
        CloseTodayAwareCTPPositionEffectFlagStrategy>();
  } else if (close_today_cost && !close_today_aware) {
    broker->SetPositionEffectStrategy<CloseTodayCostCTPPositionEffectStrategy,
                                      GenericCTPPositionEffectFlagStrategy>();
  } else {
    broker->SetPositionEffectStrategy<GenericCTPPositionEffectStrategy,
                                      GenericCTPPositionEffectFlagStrategy>();
  }
  instrument_brokers_.insert({instrument, std::move(broker)});
}

caf::behavior CAFSubAccountBroker::make_behavior() {
  return message_handler_.message_handler();
}

void CAFSubAccountBroker::InitMakeBehavior() {
  message_handler_.Assign(
      [=](const std::vector<CTPPositionField>& positions) {
        for (const auto& pos : positions) {
          MakeCtpInstrumentBrokerIfNeed(pos.instrument);
          instrument_brokers_.at(pos.instrument)
              ->InitPosition(pos.direction, {pos.yesterday, pos.today});
        }

        std::vector<OrderPosition> strategy_positions;
        for (const auto& item : instrument_brokers_) {
          if (auto pos = item.second->GetPosition()) {
            strategy_positions.push_back(*pos);
          }
        }
        boost::log::record rec = log_.open_record();
        if (rec) {
          boost::log::record_ostream s(rec);
          s << "[Init Pos]";
          bool comma = false;
          for (const auto& pos : strategy_positions) {
            if (comma) {
              s << ",";
            }
            s << "{"
              << "(I)" << pos.instrument << ", (BS)"
              << static_cast<int>(pos.order_direction) << ", (Q)"
              << pos.quantity << "}";
            comma = true;
          }
          s.flush();
          log_.push_record(std::move(rec));
        }

        live_trade_system_->Send(
            env_id_, bft::MakeMessage(std::move(strategy_positions)));
      },
      [=](const std::shared_ptr<CTPOrderField>& order) {
        auto it = instrument_brokers_.find(order->instrument);
        BOOST_ASSERT(it != instrument_brokers_.end());
        if (it != instrument_brokers_.end()) {
          it->second->HandleRtnOrder(order);
        }
      },
      [=](const std::string& instrument, const std::string& order_id,
          double trading_price, int trading_qty, TimeStamp timestamp) {
        auto it = instrument_brokers_.find(instrument);
        BOOST_ASSERT(it != instrument_brokers_.end());
        if (it != instrument_brokers_.end()) {
          it->second->HandleTraded(order_id, trading_price, trading_qty,
                                   timestamp);
        }
      },
      [=](const InputOrder& order) {
        MakeCtpInstrumentBrokerIfNeed(order.instrument);
        instrument_brokers_.at(order.instrument)->HandleInputOrder(order);
        BOOST_LOG(log_) << "[RECV Input Order]"
                        << "(Id)" << order.order_id << ",(I)"
                        << order.instrument << ", (BS)"
                        << static_cast<int>(order.direction) << ", (OC)"
                        << static_cast<int>(order.position_effect) << ", (P)"
                        << order.price << ", (Q)" << order.qty;
      },
      [=](const OrderAction& action_order) {
        BOOST_LOG(log_) << "[RECV Action Order]"
                        << "(Id)" << action_order.order_id << ", (I)"
                        << action_order.instrument << ", (NewP)"
                        << action_order.new_price << ", (OldP)"
                        << action_order.old_price << ", (NewQ)"
                        << action_order.new_qty << ", (OldQ)"
                        << action_order.old_qty;
        auto it = instrument_brokers_.find(action_order.instrument);
        BOOST_ASSERT(it != instrument_brokers_.end());
        if (it != instrument_brokers_.end()) {
          it->second->HandleOrderAction(action_order);
        }
      },
      [=](const CancelOrder& cancel) {
        BOOST_LOG(log_) << "[RECV Cancel Order]"
                        << "(Id)" << cancel.order_id << ", (I)"
                        << cancel.instrument;
        auto it = instrument_brokers_.find(cancel.instrument);
        BOOST_ASSERT(it != instrument_brokers_.end());
        if (it != instrument_brokers_.end()) {
          it->second->HandleCancel(cancel);
        }
      },
      );
}
