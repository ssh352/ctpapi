#include "strategy_trader.h"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include "atom_defines.h"

using CallOnActorAtom = caf::atom_constant<caf::atom("self")>;
using RtnOrderAtom = caf::atom_constant<caf::atom("ro")>;

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::function<void(void)>)

StrategyTrader::StrategyTrader(caf::actor_config& cfg,
                               caf::group rtn_order_grp,
                               std::string server,
                               std::string broker_id,
                               std::string user_id,
                               std::string password)
    : caf::event_based_actor(cfg),
      rtn_order_grp_(rtn_order_grp),
      server_(server),
      broker_id_(broker_id),
      user_id_(user_id),
      password_(password) {
  db_ = system().registry().get(caf::atom("db"));
}

void StrategyTrader::OnRspAuthenticate(
    CThostFtdcRspAuthenticateField* pRspAuthenticateField,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {}

void StrategyTrader::OnFrontConnected() {
  CThostFtdcReqUserLoginField field{0};
  strcpy(field.UserID, user_id_.c_str());
  strcpy(field.Password, password_.c_str());
  strcpy(field.BrokerID, broker_id_.c_str());
  api_->ReqUserLogin(&field, 0);
}

void StrategyTrader::OnFrontDisconnected(int nReason) {}

void StrategyTrader::OnRspQryInvestorPosition(
    CThostFtdcInvestorPositionField* pInvestorPosition,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  InvestorPositionField position;
  CallOnActor([ =, position(std::move(position)) ]() {
    if (pRspInfo != NULL && !IsErrorRspInfo(pRspInfo)) {
      auto it = response_.find(nRequestID);
      if (it != response_.end()) {
        boost::any_cast<std::function<void(InvestorPositionField, bool)>>(
            it->second)(std::move(position), bIsLast);
      }
    }
  });
}

caf::behavior StrategyTrader::make_behavior() {
  std::string flow_path = ".\\" + user_id_ + "\\";
  //   api_ = CThostFtdcTraderApi::CreateFtdcTraderApi(flow_path.c_str());
  //   api_->RegisterSpi(this);
  //   api_->RegisterFront(const_cast<char*>(server_.c_str()));
  //   api_->SubscribePublicTopic(THOST_TERT_RESTART);
  //   api_->SubscribePrivateTopic(THOST_TERT_RESTART);
  //   api_->Init();

  auto db = caf::actor_cast<caf::actor>(db_);
  auto db_delegate = caf::make_function_view(db);

  auto r = db_delegate(QueryStrategyOrderIDMapAtom::value, user_id_);
  if (r) {
    auto tupels = r->get_as<
        std::vector<std::tuple<std::string, std::string, std::string>>>(0);
    for (const auto& t : tupels) {
      sub_order_ids_.insert(SubOrderIDBiomap::value_type(
          {std::get<0>(t), std::get<1>(t)}, std::get<2>(t)));
    }
  }
  for (auto strategy_id : {"foo", "bar"}) {
    {
      auto r = db_delegate(QueryStrategyRntOrderAtom::value, strategy_id);
      if (r) {
        auto orders = r->get_as<std::vector<boost::shared_ptr<OrderField>>>(0);
        for (auto order : orders) {
        }
      }
    }
  }

  set_down_handler([=](const caf::down_msg& msg) {
    for (auto& strategy_actors : sub_account_on_rtn_order_callbacks_) {
      auto i = std::find_if(
          strategy_actors.second.begin(), strategy_actors.second.end(),
          [&](const caf::strong_actor_ptr& a) { return a == msg.source; });
      if (i != strategy_actors.second.end()) {
        strategy_actors.second.erase(i);
      }
    }
  });
  return {[](CallOnActorAtom, std::function<void(void)> func) { func(); },
          [](RtnOrderAtom, const boost::shared_ptr<OrderField>& order) {

          },
          [=](SubscribeRtnOrderAtom, const std::string& strategy_id) {
            caf::strong_actor_ptr actor =
                caf::actor_cast<caf::strong_actor_ptr>(current_sender());
            if (sub_account_on_rtn_order_callbacks_[strategy_id]
                    .insert(actor)
                    .second) {
              monitor(actor);

              for (const auto& order : sequence_orders_) {
                send(caf::actor_cast<caf::actor>(actor), RtnOrderAtom::value,
                     order);
              }
            }
          }};
}

void StrategyTrader::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                                    CThostFtdcRspInfoField* pRspInfo,
                                    int nRequestID,
                                    bool bIsLast) {
  front_id_ = pRspUserLogin->FrontID;
  session_id_ = pRspUserLogin->SessionID;
  if (on_connect_ != NULL) {
    on_connect_(pRspUserLogin, pRspInfo);
  }
}

void StrategyTrader::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                                     CThostFtdcRspInfoField* pRspInfo,
                                     int nRequestID,
                                     bool bIsLast) {}

void StrategyTrader::OnRtnOrder(CThostFtdcOrderField* pOrder) {
  CallOnActor(boost::bind(&StrategyTrader::OnRtnOrderOnIOThread, this,
                          boost::make_shared<CThostFtdcOrderField>(*pOrder)));
}

void StrategyTrader::OnRtnOrderOnIOThread(
    boost::shared_ptr<CThostFtdcOrderField> order) {
  std::string order_id =
      MakeOrderId(order->FrontID, order->SessionID, order->OrderRef);
  orders_.insert_or_assign(order_id, order);

  auto order_field = boost::make_shared<OrderField>();
  // order_field->instrument_name = order->InstrumentName;
  order_field->instrument_id = order->InstrumentID;
  order_field->exchange_id = order->ExchangeID;
  order_field->direction = order->Direction == THOST_FTDC_D_Buy
                               ? OrderDirection::kBuy
                               : OrderDirection::kSell;
  order_field->qty = order->VolumeTotalOriginal;
  order_field->price = order->LimitPrice;
  order_field->position_effect =
      ParseTThostFtdcPositionEffect(order->CombOffsetFlag[0]);
  order_field->date = order->InsertDate;
  order_field->input_time = order->InsertTime;
  order_field->update_time = order->UpdateTime;

  order_field->status = ParseTThostFtdcOrderStatus(order);
  order_field->leaves_qty = order->VolumeTotal;
  order_field->traded_qty = order->VolumeTraded;
  order_field->error_id = 0;
  order_field->raw_error_id = 0;

  auto sub_order_id_it = sub_order_ids_.right.find(order_id);
  if (sub_order_id_it != sub_order_ids_.right.end()) {
    auto actor_it =
        sub_account_on_rtn_order_callbacks_.find(sub_order_id_it->second.first);
    if (actor_it != sub_account_on_rtn_order_callbacks_.end()) {
      order_field->account_id = sub_order_id_it->second.first;
      order_field->order_id = sub_order_id_it->second.second;
      for (const auto& ptr : actor_it->second) {
        send(caf::actor_cast<caf::actor>(ptr), RtnOrderAtom::value,
             order_field);
      }
    }
    sequence_orders_.push_back(order_field);
  }

  send(rtn_order_grp_, order_field, std::string(order->UserID), order_id);
}

std::string StrategyTrader::MakeOrderId(TThostFtdcFrontIDType front_id,
                                        TThostFtdcSessionIDType session_id,
                                        const std::string& order_ref) const {
  return str(boost::format("%d:%d:%s") % front_id % session_id % order_ref);
}

OrderStatus StrategyTrader::ParseTThostFtdcOrderStatus(
    boost::shared_ptr<CThostFtdcOrderField> order) const {
  OrderStatus os = OrderStatus::kActive;
  switch (order->OrderStatus) {
    case THOST_FTDC_OST_AllTraded:
      os = OrderStatus::kAllFilled;
      break;
    case THOST_FTDC_OST_Canceled:
      os = OrderStatus::kCanceled;
      break;
    default:
      break;
  }
  return os;
}

PositionEffect StrategyTrader::ParseTThostFtdcPositionEffect(
    TThostFtdcOffsetFlagType flag) {
  PositionEffect ps = PositionEffect::kUndefine;
  switch (flag) {
    case THOST_FTDC_OF_Open:
      ps = PositionEffect::kOpen;
      break;
    case THOST_FTDC_OF_Close:
    case THOST_FTDC_OF_ForceClose:
    case THOST_FTDC_OF_CloseYesterday:
    case THOST_FTDC_OF_ForceOff:
    case THOST_FTDC_OF_LocalForceClose:
      ps = PositionEffect::kClose;
      break;
    case THOST_FTDC_OF_CloseToday:
      ps = PositionEffect::kCloseToday;
      break;
  }
  return ps;
}

bool StrategyTrader::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) const {
  return (pRspInfo != NULL) && (pRspInfo->ErrorID != 0);
}

TThostFtdcDirectionType StrategyTrader::OrderDirectionToTThostOrderDireciton(
    OrderDirection direction) {
  return direction == OrderDirection::kBuy ? THOST_FTDC_D_Buy
                                           : THOST_FTDC_D_Sell;
}

TThostFtdcOffsetFlagType StrategyTrader::PositionEffectToTThostOffsetFlag(
    PositionEffect position_effect) {
  return position_effect == PositionEffect::kOpen
             ? THOST_FTDC_OF_Open
             : (position_effect == PositionEffect::kCloseToday
                    ? THOST_FTDC_OF_CloseToday
                    : THOST_FTDC_OF_Close);
}

void StrategyTrader::LimitOrder(std::string sub_account_id,
                                std::string sub_order_id,
                                std::string instrument,
                                PositionEffect position_effect,
                                OrderDirection direction,
                                double price,
                                int volume) {
  std::string order_ref =
      boost::lexical_cast<std::string>(order_ref_index_.fetch_add(1));

  CallOnActor([=]() {
    std::string order_id = MakeOrderId(front_id_, session_id_, order_ref);
    sub_order_ids_.insert(
        SubOrderIDBiomap::value_type({sub_account_id, sub_order_id}, order_id));

    send(caf::actor_cast<caf::actor>(db_), InsertStrategyOrderIDAtom::value,
         sub_account_id, sub_order_id, order_id);
  });

  CThostFtdcInputOrderField field = {0};
  // strcpy(filed.BrokerID, "");
  // strcpy(filed.InvestorID, "");
  // strcpy(filed.UserID, );

  strcpy(field.InstrumentID, instrument.c_str());
  strcpy(field.OrderRef, order_ref.c_str());
  field.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
  field.Direction = OrderDirectionToTThostOrderDireciton(direction);
  field.CombOffsetFlag[0] = PositionEffectToTThostOffsetFlag(position_effect);
  strcpy(field.CombHedgeFlag, "1");
  field.LimitPrice = price;
  field.VolumeTotalOriginal = volume;
  field.TimeCondition = THOST_FTDC_TC_GFD;
  strcpy(field.GTDDate, "");
  field.VolumeCondition = THOST_FTDC_VC_AV;
  field.MinVolume = 1;
  field.ContingentCondition = THOST_FTDC_CC_Immediately;
  field.StopPrice = 0;
  field.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
  field.IsAutoSuspend = 0;
  field.UserForceClose = 0;
  strcpy(field.BrokerID, broker_id_.c_str());
  strcpy(field.UserID, user_id_.c_str());
  strcpy(field.InvestorID, user_id_.c_str());

  if (api_->ReqOrderInsert(&field, 0) != 0) {
    auto order_field = boost::make_shared<OrderField>();
    order_field->account_id = sub_account_id;
    order_field->order_id = sub_order_id;
    order_field->status = OrderStatus::kInputRejected;
    CallOnActor([ =, order_field(std::move(order_field)) ]() {
      auto cb_it = sub_account_on_rtn_order_callbacks_.find(sub_account_id);
      if (cb_it == sub_account_on_rtn_order_callbacks_.end()) {
        return;
      }

      for (const auto& ptr : cb_it->second) {
        send(caf::actor_cast<caf::actor>(ptr), RtnOrderAtom::value,
             order_field);
      }
    });
  }
}

/*
void StrategyTrader::CancelOrder(std::string order_id) {
  CallOnActor(
      boost::bind(&StrategyTrader::CancelOrderOnIOThread, this,
std::move(order_id)));
}

*/
void StrategyTrader::CancelOrder(std::string sub_accont_id,
                                 std::string sub_order_id) {
  CallOnActor(boost::bind(&StrategyTrader::CancelOrderOnIOThread, this,
                          std::move(sub_accont_id), std::move(sub_order_id)));
}

void StrategyTrader::ReqInvestorPosition(
    std::function<void(InvestorPositionField, bool)> callback) {
  CThostFtdcQryInvestorPositionField field{0};
  strcpy(field.BrokerID, broker_id_.c_str());
  strcpy(field.InvestorID, user_id_.c_str());
  int request_id = request_id_++;
  api_->ReqQryInvestorPosition(&field, request_id);
  CallOnActor([=](void) { response_.insert({request_id, callback}); });
}

// void StrategyTrader::SubscribeRtnOrder(
//     std::string sub_account_id,
//     std::function<void(boost::shared_ptr<OrderField>)> callback) {
//   CallOnActor([
//     =, sub_account_id(std::move(sub_account_id)),
//     callback(std::move(callback))
//   ](void) {
//     if (sub_account_on_rtn_order_callbacks_.find(sub_account_id) !=
//         sub_account_on_rtn_order_callbacks_.end()) {
//       return;
//     }
//     sub_account_on_rtn_order_callbacks_.insert(
//         std::make_pair(sub_account_id, callback));
//     for (auto order : sequence_orders_) {
//       callback(order);
//     }
//   });
// }

void StrategyTrader::CallOnActor(std::function<void(void)> func) {
  caf::anon_send(this, CallOnActorAtom::value, func);
}

void StrategyTrader::CancelOrderOnIOThread(std::string sub_accont_id,
                                           std::string order_id) {
  auto it = sub_order_ids_.left.find(std::make_pair(sub_accont_id, order_id));
  if (it == sub_order_ids_.left.end()) {
    return;
  }

  if (orders_.find(it->second) != orders_.end()) {
    auto order = orders_[it->second];
    CThostFtdcInputOrderActionField field = {0};
    field.ActionFlag = THOST_FTDC_AF_Delete;
    field.FrontID = front_id_;
    field.SessionID = session_id_;
    strcpy(field.OrderRef, order->OrderRef);
    strcpy(field.ExchangeID, order->ExchangeID);
    strcpy(field.OrderSysID, order->OrderSysID);
    strcpy(field.BrokerID, order->BrokerID);
    if (api_->ReqOrderAction(&field, request_id_.fetch_add(1)) != 0) {
    }
  }
}

void StrategyTrader::OnRspOrderInsert(CThostFtdcInputOrderField* pInputOrder,
                                      CThostFtdcRspInfoField* pRspInfo,
                                      int nRequestID,
                                      bool bIsLast) {
  if (IsErrorRspInfo(pRspInfo) && pInputOrder != NULL) {
    auto order_field = boost::make_shared<OrderField>();
    std::string order_id =
        MakeOrderId(front_id_, session_id_, pInputOrder->OrderRef);
    order_field->order_id = order_id;
    order_field->status = OrderStatus::kInputRejected;
    order_field->raw_error_id = pRspInfo->ErrorID;
    order_field->raw_error_message = pRspInfo->ErrorMsg;
    CallOnActor([ =, order_field(std::move(order_field)) ](){
        //       if (on_rtn_order_ != NULL) {
        //         on_rtn_order_(std::move(order_field));
        //       }
    });
  }
}

void StrategyTrader::OnRspOrderAction(
    CThostFtdcInputOrderActionField* pInputOrderAction,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  if (IsErrorRspInfo(pRspInfo) && pInputOrderAction != NULL) {
    auto order_field = boost::make_shared<OrderField>();
    std::string order_id =
        MakeOrderId(front_id_, session_id_, pInputOrderAction->OrderRef);
    order_field->order_id = order_id;
    order_field->status = OrderStatus::kCancelRejected;
    order_field->raw_error_id = pRspInfo->ErrorID;
    order_field->raw_error_message = pRspInfo->ErrorMsg;
    CallOnActor([ =, order_field(std::move(order_field)) ](){
        //       if (on_rtn_order_ != NULL) {
        //         on_rtn_order_(std::move(order_field));
        //       }
    });
  }
}

void StrategyTrader::OnRspError(CThostFtdcRspInfoField* pRspInfo,
                                int nRequestID,
                                bool bIsLast) {}
