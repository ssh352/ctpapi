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
  wait_sync_position_ = true;
  max_order_no_ = 0;

  wait_yesterday_trader_position_ = true;
  wait_yesterday_follower_position_ = true;
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
  delayed_send(this, std::chrono::seconds(1), SettlementInfoConfirmAtom::value);
}

void FollowTradeActor::OnPositions(std::vector<OrderPosition> positions) {
  send(this, YesterdayPositionForFollowerAtom::value, std::move(positions));
}

void FollowTradeActor::OnSettlementInfoConfirm() {
  delayed_send(this, std::chrono::seconds(1), QryInvestorPositionsAtom::value);
}

caf::behavior FollowTradeActor::make_behavior() {
  // ctp_.LoginServer("tcp://180.168.146.187:10000", "9999", "053861",
  //                  "Cj12345678");
  ctp_.LoginServer("tcp://ctp1-front3.citicsf.com:41205", "66666", "120350655",
                   "140616");
  return {
      [=](TAOrderIdentAtom, OrderIdent order_ident) {
        unfill_orders_[order_ident.order_id] = order_ident;
      },
      [=](SettlementInfoConfirmAtom) { ctp_.SettlementInfoConfirm(); },
      [=](TrySyncHistoryOrderAtom) {
        if (wait_sync_position_ ||
            trader_order_rtn_seq_ != last_check_trader_order_rtn_seq_ ||
            follower_order_rtn_seq_ != last_check_follower_order_rtn_seq_) {
          last_check_trader_order_rtn_seq_ = trader_order_rtn_seq_;
          last_check_follower_order_rtn_seq_ = follower_order_rtn_seq_;
          delayed_send(this, std::chrono::seconds(1),
                       TrySyncHistoryOrderAtom::value);
        } else {
          std::cout << "Sync Complete\n";
          wait_sync_orders_ = false;
          std::for_each(instrument_follow_set_.begin(),
                        instrument_follow_set_.end(),
                        [](auto& item) { item.second.SyncComplete(); });
        }
      },
      [=](QryInvestorPositionsAtom) { ctp_.QryInvestorPosition(); },
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
        if (wait_sync_position_) {
          pending_trader_rtn_orders_.push_back(order);
        } else {
          order.order_no =
              RemapOrderNo(order.order_no, order.session_id, order.request_by);
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
        }
      },
      [=](OrderRtnForFollow, OrderRtnData order) {
        ++follower_order_rtn_seq_;
        if (wait_sync_position_) {
          pending_follower_rtn_orders_.push_back(order);
        } else {
          order.order_no =
              RemapOrderNo(order.order_no, order.session_id, order.request_by);
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
      order.action == kEOAOpen
          ? THOST_FTDC_OF_Open
          : order.today ? THOST_FTDC_OF_CloseToday : THOST_FTDC_OF_Close;
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

  wait_sync_position_ = false;
  std::map<std::pair<std::string, OrderDirection>, int> position_type_map_;
  for (auto pos : trader_positions_) {
    auto key = std::make_pair(pos.instrument, pos.order_direction);
    if (position_type_map_.find(key) == position_type_map_.end()) {
      position_type_map_.insert(
          {key, static_cast<int>(position_type_map_.size())});
    }
  }

  for (auto pos : follower_positions_) {
    auto key = std::make_pair(pos.instrument, pos.order_direction);
    if (position_type_map_.find(key) == position_type_map_.end()) {
      position_type_map_.insert(
          {key, static_cast<int>(position_type_map_.size())});
    }
  }

  max_order_no_ = static_cast<int>(position_type_map_.size());

  for (auto pos : trader_positions_) {
    InstrumentFollow& instrument = GetInstrumentFollow(pos.instrument);
    instrument.AddPositionToTrader(
        boost::lexical_cast<std::string>(
            position_type_map_[{pos.instrument, pos.order_direction}]),
        pos.order_direction, pos.volume);
  }

  for (auto pos : follower_positions_) {
    InstrumentFollow& instrument = GetInstrumentFollow(pos.instrument);
    instrument.AddPositionToFollower(
        boost::lexical_cast<std::string>(
            position_type_map_[{pos.instrument, pos.order_direction}]),
        pos.order_direction, pos.volume);
  }

  for (auto order : pending_trader_rtn_orders_) {
    order.order_no =
        RemapOrderNo(order.order_no, order.session_id, order.request_by);
    InstrumentFollow& instrument = GetInstrumentFollow(order.instrument);
    EnterOrderData dummy_enter_order;
    std::vector<std::string> dummy_cancel_order_no_list;
    instrument.HandleOrderRtnForTrader(order, &dummy_enter_order,
                                       &dummy_cancel_order_no_list);
  }
  pending_trader_rtn_orders_.clear();

  for (auto order : pending_follower_rtn_orders_) {
    order.order_no =
        RemapOrderNo(order.order_no, order.session_id, order.request_by);
    InstrumentFollow& instrument = GetInstrumentFollow(order.instrument);
    EnterOrderData dummy_enter_order;
    std::vector<std::string> dummy_cancel_order_no_list;
    instrument.HandleOrderRtnForFollow(order, &dummy_enter_order,
                                       &dummy_cancel_order_no_list);
  }
  pending_follower_rtn_orders_.clear();
}

std::string FollowTradeActor::RemapOrderNo(const std::string& order_no,
                                           int session_id,
                                           RequestBy request_by) {
  if (request_by == RequestBy::kStrategy) {
    return order_no;
  }

  std::string ret_order_no;
  auto key = std::make_pair(order_no, session_id);
  if (remap_order_ref_.find(key) != remap_order_ref_.end()) {
    ret_order_no = boost::lexical_cast<std::string>(remap_order_ref_[key]);
  } else {
    remap_order_ref_.insert(
        {{order_no, session_id},
         static_cast<int>(remap_order_ref_.size()) + max_order_no_});
    ret_order_no = boost::lexical_cast<std::string>(remap_order_ref_[key]);
  }
  return ret_order_no;
}
