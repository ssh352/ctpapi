#include <iostream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <chrono>
#include <iostream>
#include <thread>
#include "ctpapi/ThostFtdcUserApiStruct.h"
using CTASignalAtom = int;
#include "follow_strategy/cta_order_signal_subscriber.h"
#include "common/api_struct.h"
#include "hpt_core/time_util.h"

struct ExchangeIdOrderSysId {
  std::string exchange_id;
  std::string order_sys_id;
};

struct HashExchagneIdOrderSysId {
  size_t operator()(const ExchangeIdOrderSysId& id) const {
    size_t seed = 0;
    boost::hash_combine(seed, id.exchange_id);
    boost::hash_combine(seed, id.order_sys_id);
    return seed;
  }
};

struct CompareExchangeIdOrderSysId {
  bool operator()(const ExchangeIdOrderSysId& l,
                  const ExchangeIdOrderSysId& r) const {
    return l.exchange_id == r.exchange_id && l.order_sys_id == r.order_sys_id;
  }
};

std::unordered_map<ExchangeIdOrderSysId,
                   std::string,
                   HashExchagneIdOrderSysId,
                   CompareExchangeIdOrderSysId>
    order_sys_id_to_order_id_;

namespace boost {
namespace serialization {
template <class Archive>
void serialize(Archive& ar,
               CThostFtdcOrderField& order,
               const unsigned int version) {
  ar& order.BrokerID;
  ar& order.InvestorID;
  ar& order.InstrumentID;
  ar& order.OrderRef;
  ar& order.UserID;
  ar& order.OrderPriceType;
  ar& order.Direction;
  ar& order.CombOffsetFlag;
  ar& order.CombHedgeFlag;
  ar& order.LimitPrice;
  ar& order.VolumeTotalOriginal;
  ar& order.TimeCondition;
  ar& order.GTDDate;
  ar& order.VolumeCondition;
  ar& order.MinVolume;
  ar& order.ContingentCondition;
  ar& order.StopPrice;
  ar& order.ForceCloseReason;
  ar& order.IsAutoSuspend;
  ar& order.BusinessUnit;
  ar& order.RequestID;
  ar& order.OrderLocalID;
  ar& order.ExchangeID;
  ar& order.ParticipantID;
  ar& order.ClientID;
  ar& order.ExchangeInstID;
  ar& order.TraderID;
  ar& order.InstallID;
  ar& order.OrderSubmitStatus;
  ar& order.NotifySequence;
  ar& order.TradingDay;
  ar& order.SettlementID;
  ar& order.OrderSysID;
  ar& order.OrderSource;
  ar& order.OrderStatus;
  ar& order.OrderType;
  ar& order.VolumeTraded;
  ar& order.VolumeTotal;
  ar& order.InsertDate;
  ar& order.InsertTime;
  ar& order.ActiveTime;
  ar& order.SuspendTime;
  ar& order.UpdateTime;
  ar& order.CancelTime;
  ar& order.ActiveTraderID;
  ar& order.ClearingPartID;
  ar& order.SequenceNo;
  ar& order.FrontID;
  ar& order.SessionID;
  ar& order.UserProductInfo;
  ar& order.StatusMsg;
  ar& order.UserForceClose;
  ar& order.ActiveUserID;
  ar& order.BrokerOrderSeq;
  ar& order.RelativeOrderSysID;
  ar& order.ZCETotalTradedVolume;
  ar& order.IsSwapOrder;
  ar& order.BranchID;
  ar& order.InvestUnitID;
  ar& order.AccountID;
  ar& order.CurrencyID;
  ar& order.IPAddress;
  ar& order.MacAddress;
}

}  // namespace serialization
}  // namespace boost

std::shared_ptr<OrderField> MakeOrderField(
    const std::shared_ptr<CTPOrderField>& ctp_order,
    double trading_price,
    int trading_qty,
    TimeStamp timestamp) {
  if (ctp_order == NULL) {
    return std::shared_ptr<OrderField>();
  }
  auto order = std::make_shared<OrderField>();
  order->direction = ctp_order->direction;
  order->position_effect_direction = ctp_order->position_effect_direction;
  order->position_effect =
      ctp_order->position_effect == CTPPositionEffect::kOpen
          ? PositionEffect::kOpen
          : PositionEffect::kClose;
  order->status = ctp_order->status;
  order->qty = ctp_order->qty;
  order->leaves_qty = ctp_order->leaves_qty;
  order->error_id = ctp_order->error_id;
  order->raw_error_id = ctp_order->raw_error_id;
  order->input_price = ctp_order->input_price;
  order->avg_price = ctp_order->avg_price;
  order->input_timestamp = ctp_order->input_timestamp;
  if (timestamp != 0) {
    order->update_timestamp = timestamp;
  } else {
    order->update_timestamp = ctp_order->update_timestamp;
  }
  order->instrument_id = ctp_order->instrument;
  order->exchange_id = ctp_order->exchange_id;
  order->date = ctp_order->date;
  order->order_id = ctp_order->order_id;
  order->raw_error_message = ctp_order->raw_error_message;
  order->trading_qty = trading_qty;
  order->trading_price = trading_price;
  return order;
}

std::string MakeCtpUniqueOrderId(int front_id,
                                 int session_id,
                                 const std::string& order_ref) {
  return str(boost::format("%d:%d:%s") % front_id % session_id % order_ref);
}

CTPPositionEffect ParseTThostFtdcPositionEffect(TThostFtdcOffsetFlagType flag) {
  CTPPositionEffect ps = CTPPositionEffect::kUndefine;
  switch (flag) {
    case THOST_FTDC_OF_Open:
      ps = CTPPositionEffect::kOpen;
      break;
    case THOST_FTDC_OF_Close:
    case THOST_FTDC_OF_ForceClose:
    case THOST_FTDC_OF_CloseYesterday:
    case THOST_FTDC_OF_ForceOff:
    case THOST_FTDC_OF_LocalForceClose:
      ps = CTPPositionEffect::kClose;
      break;
    case THOST_FTDC_OF_CloseToday:
      ps = CTPPositionEffect::kCloseToday;
      break;
  }
  return ps;
}

OrderStatus ParseTThostFtdcOrderStatus(CThostFtdcOrderField* order) {
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

std::shared_ptr<CTPOrderField> MakeCtpOrderField(CThostFtdcOrderField* pOrder) {
  if (strlen(pOrder->OrderSysID) == 0) {
    return std::shared_ptr<CTPOrderField>();
  }

  std::string order_id = MakeCtpUniqueOrderId(
      pOrder->FrontID, pOrder->SessionID, pOrder->OrderRef);

  if (order_sys_id_to_order_id_.find(
          {pOrder->ExchangeID, pOrder->OrderSysID}) ==
      order_sys_id_to_order_id_.end()) {
    order_sys_id_to_order_id_.insert(
        {{pOrder->ExchangeID, pOrder->OrderSysID}, order_id});
  }

  auto order_field = std::make_shared<CTPOrderField>();
  // order_field->instrument_name = order->InstrumentName;
  order_field->instrument = pOrder->InstrumentID;
  order_field->exchange_id = pOrder->ExchangeID;

  order_field->qty = pOrder->VolumeTotalOriginal;
  order_field->input_price = pOrder->LimitPrice;
  order_field->position_effect =
      ParseTThostFtdcPositionEffect(pOrder->CombOffsetFlag[0]);

  if (order_field->position_effect == CTPPositionEffect::kOpen) {
    order_field->position_effect_direction =
        pOrder->Direction == THOST_FTDC_D_Buy ? OrderDirection::kBuy
                                              : OrderDirection::kSell;
  } else {
    order_field->position_effect_direction =
        pOrder->Direction == THOST_FTDC_D_Buy ? OrderDirection::kSell
                                              : OrderDirection::kBuy;
  }

  order_field->direction = pOrder->Direction == THOST_FTDC_D_Buy
                               ? OrderDirection::kBuy
                               : OrderDirection::kSell;
  order_field->date = pOrder->InsertDate;
  // order_field->input_timestamp = pOrder->InsertTime;
  order_field->update_timestamp =
      ptime_to_timestamp(boost::posix_time::microsec_clock::local_time());

  order_field->status = ParseTThostFtdcOrderStatus(pOrder);
  order_field->leaves_qty = pOrder->VolumeTotal;
  order_field->order_ref = pOrder->OrderRef;
  order_field->order_sys_id = pOrder->OrderSysID;
  order_field->order_id = order_id;
  order_field->trading_qty = 0;
  order_field->error_id = 0;
  order_field->raw_error_id = 0;
  order_field->front_id = pOrder->FrontID;
  order_field->session_id = pOrder->SessionID;
  return order_field;
}

class OutputMailBox {
 public:
  template <typename... Ts>
  void Subscribe(Ts&&... args) {}

  template <typename... Ts>
  void Send(Ts&&... args) {}
};

void BackRoolRtnOrder() {
  OutputMailBox mail_box;
  CTAOrderSignalSubscriber<OutputMailBox> cta_signal_subscriber(&mail_box);

  // auto& it =
  //    ctp_orders_.find(ctp_order->order_id, HashCTPOrderField(),
  //                     CompareCTPOrderField());
  // if (it == ctp_orders_.end()) {
  //  signal_subscriber_.HandleCTASignalOrder(
  //      CTASignalAtom::value, MakeOrderField(ctp_order, 0.0, 0));
  //  ctp_orders_.insert(ctp_order);
  //} else if (ctp_order->status == OrderStatus::kCanceled) {
  //  signal_subscriber_.HandleCTASignalOrder(
  //      CTASignalAtom::value, MakeOrderField(ctp_order, 0.0, 0));
  //  ctp_orders_.erase(it);
  //  ctp_orders_.insert(ctp_order);
  //} else {
  //}

  std::vector<std::shared_ptr<OrderField>> view_orders;
  try {
    std::ifstream file("c:\\Users\\yjqpro\\Desktop\\181006-20171113-114116.bin",
                       std::ios_base::binary);
    boost::archive::binary_iarchive ia(file);

    while (true) {
      CThostFtdcOrderField ftdc_order;
      ia >> ftdc_order;

      auto order = MakeOrderField(MakeCtpOrderField(&ftdc_order), 0.0, 0, 0);
      if (order != nullptr) {
        cta_signal_subscriber.HandleCTASignalOrder(CTASignalAtom(),
                                                   std::move(order));
      }

      // signal_subscriber_.HandleCTASignalOrder(
      // CTASignalAtom::value, MakeOrderField(ctp_order, 0.0, 0));
    }
  } catch (boost::archive::archive_exception& err) {
    std::cout << "Done" << err.what() << "\n";
  }
}
int main(int argc, char* argv[]) {
  BackRoolRtnOrder();
  return 0;
}
