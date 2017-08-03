#include "cta_signal_trader.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/common.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/make_shared.hpp>
#include "atom_defines.h"
#include "logging.h"
CTASignalTrader::CTASignalTrader(caf::actor_config& cfg,
                                 std::string server,
                                 std::string broker_id,
                                 std::string user_id,
                                 std::string password)
    : caf::event_based_actor(cfg),
      server_(server),
      broker_id_(broker_id),
      user_id_(user_id),
      password_(password) {
  //  db_ = system().registry().get(caf::atom("db"));
  boost::filesystem::path dir(".\\" + user_id);

  boost::filesystem::directory_iterator end_iter;
  for (boost::filesystem::directory_iterator it(dir); it != end_iter; ++it) {
    boost::filesystem::remove_all(it->path());
  }
}

CTASignalTrader::~CTASignalTrader() {}

void CTASignalTrader::OnFrontConnected() {
  CThostFtdcReqUserLoginField field{0};
  strcpy(field.UserID, user_id_.c_str());
  strcpy(field.Password, password_.c_str());
  strcpy(field.BrokerID, broker_id_.c_str());
  api_->ReqUserLogin(&field, 0);
}

void CTASignalTrader::OnFrontDisconnected(int nReason) {}

void CTASignalTrader::OnRspQryInvestorPosition(
    CThostFtdcInvestorPositionField* pInvestorPosition,
    CThostFtdcRspInfoField* pRspInfo,
    int nRequestID,
    bool bIsLast) {
  if (!IsErrorRspInfo(pRspInfo) && pInvestorPosition) {
    OrderPosition position{
        pInvestorPosition->InstrumentID,
        pInvestorPosition->PosiDirection == THOST_FTDC_PD_Long
            ? OrderDirection::kBuy
            : OrderDirection::kSell,
        pInvestorPosition->YdPosition};

    CallOnActor([ =, position(std::move(position)) ] {
      if (position.quantity != 0) {
        yesterday_positions_.push_back(std::move(position));
      }
    });
  }

  if (bIsLast) {
    auto& lg = g_logger::get();
    BOOST_LOG(lg) << "CTA Restart rtn order.";
    CallOnActor([=]() {
      delayed_send(this, std::chrono::seconds(1),
                   CheckHistoryRtnOrderIsDoneAtom::value,
                   sequence_orders_.size());
    });
  }
}

void CTASignalTrader::OnRspError(CThostFtdcRspInfoField* pRspInfo,
                                 int nRequestID,
                                 bool bIsLast) {}

void CTASignalTrader::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                                     CThostFtdcRspInfoField* pRspInfo,
                                     int nRequestID,
                                     bool bIsLast) {
  auto& lg = g_logger::get();
  if (pRspInfo != nullptr && pRspInfo->ErrorID == 0) {
    BOOST_LOG(lg) << "CTA trader login sccuess.";
  } else {
    BOOST_LOG(lg) << "CTA trader login is fail:" << pRspInfo->ErrorID << ":"
                  << pRspInfo->ErrorMsg;
  }

  send(this, ConnectAtom::value,
       std::make_shared<CThostFtdcRspUserLoginField>(*pRspUserLogin),
       std::make_shared<CThostFtdcRspInfoField>(*pRspInfo));
}

void CTASignalTrader::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout,
                                      CThostFtdcRspInfoField* pRspInfo,
                                      int nRequestID,
                                      bool bIsLast) {}

void CTASignalTrader::OnRtnOrder(CThostFtdcOrderField* pOrder) {
  CallOnActor(boost::bind(&CTASignalTrader::OnRtnOrderOnIOThread, this,
                          boost::make_shared<CThostFtdcOrderField>(*pOrder)));
}

void CTASignalTrader::OnRtnOrderOnIOThread(
    boost::shared_ptr<CThostFtdcOrderField> order) {
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

  order_field->strategy_id = order->InvestorID;
  order_field->order_id =
      MakeOrderId(order->FrontID, order->SessionID, order->OrderRef);

  for (const auto& o : rtn_order_observers_) {
    send(caf::actor_cast<caf::actor>(o), RtnOrderAtom::value, order_field);
  }
  sequence_orders_.push_back(std::move(order_field));
}

std::string CTASignalTrader::MakeOrderId(TThostFtdcFrontIDType front_id,
                                         TThostFtdcSessionIDType session_id,
                                         const std::string& order_ref) const {
  return str(boost::format("%d:%d:%s") % front_id % session_id % order_ref);
}

OrderStatus CTASignalTrader::ParseTThostFtdcOrderStatus(
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

PositionEffect CTASignalTrader::ParseTThostFtdcPositionEffect(
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

bool CTASignalTrader::IsErrorRspInfo(CThostFtdcRspInfoField* pRspInfo) const {
  return (pRspInfo != NULL) && (pRspInfo->ErrorID != 0);
}

TThostFtdcDirectionType CTASignalTrader::OrderDirectionToTThostOrderDireciton(
    OrderDirection direction) {
  return direction == OrderDirection::kBuy ? THOST_FTDC_D_Buy
                                           : THOST_FTDC_D_Sell;
}

TThostFtdcOffsetFlagType CTASignalTrader::PositionEffectToTThostOffsetFlag(
    PositionEffect position_effect) {
  return position_effect == PositionEffect::kOpen
             ? THOST_FTDC_OF_Open
             : (position_effect == PositionEffect::kCloseToday
                    ? THOST_FTDC_OF_CloseToday
                    : THOST_FTDC_OF_Close);
}

void CTASignalTrader::CallOnActor(std::function<void(void)> func) {
  caf::anon_send(this, CallOnActorAtom::value, func);
}

caf::behavior CTASignalTrader::make_behavior() {
  auto& lg = g_logger::get();

  BOOST_LOG(lg) << "Start CTA Signal";

  std::string flow_path = ".\\" + user_id_ + "\\";
  api_ = CThostFtdcTraderApi::CreateFtdcTraderApi(flow_path.c_str());
  api_->RegisterSpi(this);
  api_->RegisterFront(const_cast<char*>(server_.c_str()));
  api_->SubscribePublicTopic(THOST_TERT_RESUME);
  api_->SubscribePrivateTopic(THOST_TERT_RESUME);
  api_->Init();

  set_down_handler([=](const caf::down_msg& msg) {
    auto i = std::find_if(
        rtn_order_observers_.begin(), rtn_order_observers_.end(),
        [&](const caf::strong_actor_ptr& a) { return a == msg.source; });
    if (i != rtn_order_observers_.end()) {
      rtn_order_observers_.erase(i);
    }
  });

  set_default_handler(caf::skip());

  delayed_send(this, std::chrono::seconds(5), ConnectTimeOutAtom::value);
  caf::behavior behavior = {

      [](CallOnActorAtom, std::function<void(void)> func) { func(); },
      [=](SubscribeRtnOrderAtom, const caf::strong_actor_ptr& actor)
          -> std::list<boost::shared_ptr<OrderField>> {
        if (rtn_order_observers_.insert(actor).second) {
          monitor(actor);
          return sequence_orders_;
        }
        return std::list<boost::shared_ptr<OrderField>>();
      },
      [=](QueryInverstorPositionAtom) { return yesterday_positions_; },
  };

  return {
      [=](ConnectAtom, const std::shared_ptr<CThostFtdcRspUserLoginField>& rsp,
          const std::shared_ptr<CThostFtdcRspInfoField>& rsp_info) {
        if (!IsErrorRspInfo(rsp_info.get())) {
          front_id_ = rsp->FrontID;
          session_id_ = rsp->SessionID;
          CThostFtdcQryInvestorPositionField field{0};
          strcpy(field.BrokerID, broker_id_.c_str());
          strcpy(field.InvestorID, user_id_.c_str());

          auto& lg = g_logger::get();
          BOOST_LOG(lg) << "CTA request investor position.";
          if (api_->ReqQryInvestorPosition(&field, request_id_++) != 0) {
            quit();
          }
        } else {
          quit();
        }
      },
      [=](CheckHistoryRtnOrderIsDoneAtom, size_t last_check_size) {
        if (last_check_size != sequence_orders_.size()) {
          // maybe still on receive
          delayed_send(this, std::chrono::milliseconds(500),
                       CheckHistoryRtnOrderIsDoneAtom::value,
                       sequence_orders_.size());
        } else {
          auto& lg = g_logger::get();
          BOOST_LOG(lg) << "CTA RtnOrder:" << sequence_orders_.size();

          become(behavior);
        }
      },
      [](CallOnActorAtom, std::function<void(void)> func) { func(); },
      [=](ConnectTimeOutAtom) {
        if (session_id_ == 0) {
          api_->Release();
          quit();
        }
      }};
}
