#include "follow_trade_actor.h"
#include "geek_quant/caf_defines.h"

FollowTradeActor::FollowTradeActor(caf::actor_config& cfg)
    : caf::event_based_actor(cfg), ctp_(this, "follower") {
  instrument_follow_set_.insert_or_assign("MA705", InstrumentFollow());
  instrument_follow_set_.insert_or_assign("c1709", InstrumentFollow());
  instrument_follow_set_.insert_or_assign("cs1709", InstrumentFollow());
  instrument_follow_set_.insert_or_assign("bu1706", InstrumentFollow());
  instrument_follow_set_.insert_or_assign("FG705", InstrumentFollow());
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
  delayed_send(this, std::chrono::seconds(3), TrySyncHistoryOrderAtom::value);
}

caf::behavior FollowTradeActor::make_behavior() {
  ctp_.LoginServer("tcp://180.168.146.187:10000", "9999", "053861",
                   "Cj12345678");
  return {
      [=](TAOrderIdentAtom, OrderIdent order_ident) {
        unfill_orders_[order_ident.order_id] = order_ident;
      },
      [=](TrySyncHistoryOrderAtom) {
        bool sync_finish = true;
        for (auto& item : instrument_follow_set_) {
          if (!item.second.TryCompleteSyncOrders()) {
            sync_finish = false;
          }
        }
        if (!sync_finish) {
          delayed_send(this, std::chrono::seconds(3),
                       TrySyncHistoryOrderAtom::value);
        }
      },
      [=](OrderRtnForTrader, OrderRtnData order) {
        auto it = instrument_follow_set_.find(order.instrument);
        if (it != instrument_follow_set_.end()) {
          EnterOrderData enter_order;
          std::vector<std::string> cancel_order_no_list;
          it->second.HandleOrderRtnForTrader(order, &enter_order,
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
        auto it = instrument_follow_set_.find(order.instrument);
        if (it != instrument_follow_set_.end()) {
          EnterOrderData enter_order;
          std::vector<std::string> cancel_order_no_list;
          it->second.HandleOrderRtnForFollow(order, &enter_order,
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
      order.action == kEOAOpen ? THOST_FTDC_OF_Open : THOST_FTDC_OF_Close;
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
