#include "follow_trade_actor.h"
#include "geek_quant/caf_defines.h"

FollowTradeActor::FollowTradeActor(caf::actor_config& cfg)
    : caf::event_based_actor(cfg),
      ctp_(this, "follower"),
      ctp_order_dispatcher_(false) {
  trader_order_rtn_seq_ = 0;
  follower_order_rtn_seq_ = 0;
  last_check_trader_order_rtn_seq_ = -1;
  last_check_follower_order_rtn_seq_ = -1;
  wait_sync_orders_ = true;
  max_order_no_ = 0;

  wait_yesterday_trader_position_ = false;
  wait_yesterday_follower_position_ = false;
}

FollowTradeActor::~FollowTradeActor() {}

void FollowTradeActor::OnRtnOrderData(CThostFtdcOrderField* order_field) {
  OrderIdent ord_ident;
  ord_ident.order_id = order_field->OrderRef;
  ord_ident.exchange_id = order_field->ExchangeID;
  ord_ident.front_id = order_field->FrontID;
  ord_ident.order_sys_id = order_field->OrderSysID;
  ord_ident.session_id = order_field->SessionID;

  send(this, TAOrderIdentAtom::value, ord_ident);
  if (auto order = ctp_order_dispatcher_.HandleRtnOrder(*order_field)) {
    send(this, OrderRtnForFollow::value, *order);
  }
}

void FollowTradeActor::OnLogon() {
  delayed_send(this, std::chrono::seconds(1), TrySyncHistoryOrderAtom::value);
}

caf::behavior FollowTradeActor::make_behavior() {
  ctp_.LoginServer("tcp://180.168.146.187:10000", "9999", "053861",
                   "Cj12345678");
  return {
      [=](TAOrderIdentAtom, OrderIdent order_ident) {
        unfill_orders_[order_ident.order_id] = order_ident;
      },
      [=](TrySyncHistoryOrderAtom) {
        if (trader_order_rtn_seq_ != last_check_trader_order_rtn_seq_ ||
            follower_order_rtn_seq_ != last_check_follower_order_rtn_seq_) {
          last_check_trader_order_rtn_seq_ = trader_order_rtn_seq_;
          last_check_follower_order_rtn_seq_ = follower_order_rtn_seq_;
          delayed_send(this, std::chrono::seconds(1),
                       TrySyncHistoryOrderAtom::value);
        } else {
          wait_sync_orders_ = false;
          std::for_each(instrument_follow_set_.begin(),
                        instrument_follow_set_.end(),
                        [](auto& item) { item.second.SyncComplete(); });
        }
      },
      [=](YesterdayPositionForTraderAtom,
          std::vector<OrderPosition> positions) {
        trader_positions_ = positions;
        wait_yesterday_trader_position_ = false;
        TrySyncPositionIfReady();
      },
      [=](YesterdayPositionForFollowerAtom,
          std::vector<OrderPosition> positions) {
        follower_positions_ = positions;
        wait_yesterday_follower_position_ = false;
        TrySyncPositionIfReady();
      },
      [=](OrderRtnForTrader, OrderRtnData order) {
        ++trader_order_rtn_seq_;
        InstrumentFollow& instrument_follow =
            GetInstrumentFollow(order.instrument);
        EnterOrderData enter_order;
        std::vector<std::string> cancel_order_no_list;
        instrument_follow.HandleOrderRtnForTrader(order, &enter_order,
                                                  &cancel_order_no_list);
        if (!enter_order.order_no.empty()) {
          ctp_.OrderInsert(MakeCtpOrderInsert(enter_order));
        }
        for (auto order_no : cancel_order_no_list) {
          if (unfill_orders_.find(order_no) != unfill_orders_.end()) {
            ctp_.OrderAction(
                MakeCtpCancelOrderAction(unfill_orders_[order_no]));
          }
        }
      },
      [=](OrderRtnForFollow, OrderRtnData order) {
        ++follower_order_rtn_seq_;
        InstrumentFollow& instrument_follow =
            GetInstrumentFollow(order.instrument);
        EnterOrderData enter_order;
        std::vector<std::string> cancel_order_no_list;
        instrument_follow.HandleOrderRtnForFollow(order, &enter_order,
                                                  &cancel_order_no_list);
        for (auto order_no : cancel_order_no_list) {
          if (unfill_orders_.find(order_no) != unfill_orders_.end()) {
            ctp_.OrderAction(
                MakeCtpCancelOrderAction(unfill_orders_[order_no]));
          }
        }
      },
      [=](CancelOrderAtom, std::vector<std::string> cancel_orders) {

      },
  };
}

CThostFtdcInputOrderActionField FollowTradeActor::MakeCtpCancelOrderAction(
    const OrderIdent& order_ident) const {
  CThostFtdcInputOrderActionField order = {0};
  order.ActionFlag = THOST_FTDC_AF_Delete;
  order.FrontID = order_ident.front_id;
  order.SessionID = order_ident.session_id;
  strcpy(order.OrderRef, order_ident.order_id.c_str());
  strcpy(order.ExchangeID, order_ident.exchange_id.c_str());
  strcpy(order.OrderSysID, order_ident.order_sys_id.c_str());
  return order;
}

CThostFtdcInputOrderField FollowTradeActor::MakeCtpOrderInsert(
    const EnterOrderData& order) const {
  CThostFtdcInputOrderField field = {0};
  // strcpy(filed.BrokerID, "");
  // strcpy(filed.InvestorID, "");
  strcpy(field.InstrumentID, order.instrument.c_str());
  strcpy(field.OrderRef, order.order_no.c_str());
  // strcpy(filed.UserID, );
  field.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
  field.Direction =
      order.order_direction == kODBuy ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;
  field.CombOffsetFlag[0] =
      order.action == kEOAOpen ? THOST_FTDC_OF_Open : THOST_FTDC_OF_CloseToday;
  strcpy(field.CombHedgeFlag, "1");
  field.LimitPrice = order.order_price;
  field.VolumeTotalOriginal = order.volume;
  field.TimeCondition = THOST_FTDC_TC_GFD;
  strcpy(field.GTDDate, "");
  field.VolumeCondition = THOST_FTDC_VC_AV;
  field.MinVolume = 1;
  field.ContingentCondition = THOST_FTDC_CC_Immediately;
  field.StopPrice = 0;
  field.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
  field.IsAutoSuspend = 0;
  field.UserForceClose = 0;
  return field;
}

InstrumentFollow& FollowTradeActor::GetInstrumentFollow(
    const std::string& instrument) {
  if (instrument_follow_set_.find(instrument) == instrument_follow_set_.end()) {
    instrument_follow_set_.insert(
        std::make_pair(instrument, InstrumentFollow(wait_sync_orders_)));
  }
  return instrument_follow_set_[instrument];
}

void FollowTradeActor::TrySyncPositionIfReady() {
  if (wait_yesterday_trader_position_ || wait_yesterday_follower_position_) {
    return;
  }

  std::map<std::pair<std::string, OrderDirection>, int> position_type_set_;
  for (auto pos : trader_positions_) {
    auto key = std::make_pair(pos.instrument, pos.order_direction);
    if (position_type_set_.find(key) == position_type_set_.end()) {
      position_type_set_.insert(
          {key, static_cast<int>(position_type_set_.size() + 1)});
    }
  }

  for (auto pos : follower_positions_) {
    auto key = std::make_pair(pos.instrument, pos.order_direction);
    if (position_type_set_.find(key) == position_type_set_.end()) {
      position_type_set_.insert(
          {key, static_cast<int>(position_type_set_.size() + 1)});
    }
  }

  max_order_no_ = static_cast<int>(position_type_set_.size());

  for (auto pos : trader_positions_) {
    instrument_follow_set_[pos.instrument].AddPositionToTrader(
        boost::lexical_cast<std::string>(
            position_type_set_[{pos.instrument, pos.order_direction}]),
        pos.order_direction, pos.volume);
  }

  for (auto pos : follower_positions_) {
    instrument_follow_set_[pos.instrument].AddPositionToFollower(
        boost::lexical_cast<std::string>(
            position_type_set_[{pos.instrument, pos.order_direction}]),
        pos.order_direction, pos.volume);
  }
}
