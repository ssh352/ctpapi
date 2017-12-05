#include "sub_account_broker.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "caf_common/caf_atom_defines.h"

CAFSubAccountBroker::CAFSubAccountBroker(
    caf::actor_config& cfg,
    LiveTradeMailBox* inner_mail_box,
    LiveTradeMailBox* common_mail_box,

    std::unordered_set<std::string> close_today_cost_of_product_codes,
    std::string account_id)
    : caf::event_based_actor(cfg),
      inner_mail_box_(inner_mail_box),
      common_mail_box_(common_mail_box),
      close_today_cost_of_product_codes_(close_today_cost_of_product_codes),
      account_id_(std::move(account_id)) {
  // inner_mail_box_->Subscrdibe(
  //    typeid(std::tuple<std::shared_ptr<CTPOrderField>>), this);
  inner_mail_box_->Subscribe(typeid(std::tuple<InputOrder>), this);
  inner_mail_box_->Subscribe(typeid(std::tuple<CancelOrder>), this);
  inner_mail_box_->Subscribe(typeid(std::tuple<OrderAction>), this);
}

std::string CAFSubAccountBroker::GenerateOrderId() {
  return boost::lexical_cast<std::string>(order_seq_++);
}

void CAFSubAccountBroker::ReturnOrderField(
    const std::shared_ptr<OrderField>& order) {
  inner_mail_box_->Send(order);
}

void CAFSubAccountBroker::HandleCancelOrder(
    const CTPCancelOrder& cancel_order) {
  common_mail_box_->Send(account_id_, cancel_order);
}

void CAFSubAccountBroker::HandleEnterOrder(const CTPEnterOrder& enter_order) {
  common_mail_box_->Send(account_id_, enter_order);
}

void CAFSubAccountBroker::MakeCtpInstrumentBrokerIfNeed(
    const std::string& instrument) {
  if (instrument_brokers_.find(instrument) != instrument_brokers_.end()) {
    return;
  }
  auto broker = std::make_unique<CTPInstrumentBroker>(
      this, instrument, false,
      std::bind(&CAFSubAccountBroker::GenerateOrderId, this));
  std::string instrument_code =
      instrument.substr(0, instrument.find_first_of("0123456789"));
  boost::algorithm::to_lower(instrument_code);
  bool close_today_cost = false;
  bool close_today_aware = false;
  auto product_info = product_info_mananger_->GetProductInfo(instrument_code);
  if (product_info && product_info->exchange == "sc") {
    close_today_aware = true;
  }

  if (close_today_cost_of_product_codes_.find(instrument_code) !=
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
  return {
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
        inner_mail_box_->Send(std::move(strategy_positions));
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
      },
      [=](const OrderAction& action_order) {
        auto it = instrument_brokers_.find(action_order.instrument);
        BOOST_ASSERT(it != instrument_brokers_.end());
        if (it != instrument_brokers_.end()) {
          it->second->HandleOrderAction(action_order);
        }
      },
      [=](const CancelOrder& cancel) {
        auto it = instrument_brokers_.find(cancel.instrument);
        BOOST_ASSERT(it != instrument_brokers_.end());
        if (it != instrument_brokers_.end()) {
          it->second->HandleCancel(cancel);
        }
      },
  };
}
