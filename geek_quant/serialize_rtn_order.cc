#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/thread.hpp>
#include <fstream>
#include <iostream>
#include "ctpapi/ThostFtdcTraderApi.h"
#include "ctp_trader.h"

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

class SerializeCtpTrader : public CThostFtdcTraderSpi {
 public:
  SerializeCtpTrader() {
    file_.open("rtn_order.txt");
    oa_ = boost::make_shared<boost::archive::text_oarchive>(file_);
    cta_api_ = CThostFtdcTraderApi::CreateFtdcTraderApi();
  }

  void LoginServer(const std::string& front_server,
                   const std::string& broker_id,
                   const std::string& user_id,
                   const std::string& password) {
    broker_id_ = broker_id;
    user_id_ = user_id;
    password_ = password;

    cta_api_->RegisterSpi(this);
    char front_server_buffer[256] = {0};
    strcpy(front_server_buffer, front_server.c_str());

    cta_api_->RegisterFront(front_server_buffer);
    // api_->SubscribePublicTopic(THOST_TERT_RESTART);
    cta_api_->SubscribePublicTopic(THOST_TERT_RESUME);
    cta_api_->SubscribePrivateTopic(THOST_TERT_RESTART);
    cta_api_->Init();
  }

  virtual void OnFrontConnected() override {
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, broker_id_.c_str());
    strcpy(req.UserID, user_id_.c_str());
    strcpy(req.Password, password_.c_str());
    // strcpy(req.UserProductInfo, "Q7");
    int iResult = cta_api_->ReqUserLogin(&req, 0);
  }

  virtual void OnFrontDisconnected(int nReason) override {}

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin,
                              CThostFtdcRspInfoField* pRspInfo,
                              int nRequestID,
                              bool bIsLast) override {
    std::cout << "Logon\n";
  }

  virtual void OnRtnOrder(CThostFtdcOrderField* pOrder) override {
    boost::lock_guard<boost::mutex> lock(mutex);
    static int i = 0;
    *oa_&* pOrder;
    std::cout << "RtnOrder:" << ++i << "\n";
  }

 private:
  CThostFtdcTraderApi* cta_api_;
  std::string broker_id_;
  std::string user_id_;
  std::string password_;
  std::ofstream fstream_;
  std::ofstream position_detail_fstream_;
  std::ofstream position_order_fstream_;
  std::ofstream file_;
  // boost::archive::text_oarchive oa(file);
  boost::mutex mutex;
  boost::shared_ptr<boost::archive::text_oarchive> oa_;
};

void LoadRtnOrder() {
  std::ifstream file("rtn_order.txt");

  std::vector<CThostFtdcOrderField> orders;
  try {
    boost::archive::text_iarchive ia(file);
    while (true) {
      CThostFtdcOrderField order;
      memset(&order, 0, sizeof(CThostFtdcOrderField));
      ia >> order;
      orders.push_back(order);
      std::cout << orders.size() << "\n";
      if (orders.size() == 18) {
        std::cout << "O\n";
      }
    }
  } catch (boost::archive::archive_exception& exp) {
    std::cout << "exception " << exp.what() << "\n";
  }

  std::cout << "load\n";
}

std::ofstream MakeFileStream() {
  std::ofstream stream("out_rtn_order.csv");
  stream << "OrderNo,"
         << "Instrument,"
         << "OrderDirection,"
         << "OrderStatus,"
         << "OrderPrice,"
         << "Volume\n";
  return stream;
}

int main(int argc, char* argv[]) {
  /*
  CtpTrader ctp;
  ctp.LoginServer("tcp://59.42.241.91:41205", "9080", "38030022", "140616");
  std::string input;
  while (std::cin >> input) {
    if (input == "exit") {
      break;
    } else if (input == "1") {
      SerializeCtpTrader* ctp = new SerializeCtpTrader();
      ctp->LoginServer("tcp://59.42.241.91:41205", "9080", "38030022",
                       "140616");
    } else if (input == "2") {
      LoadRtnOrder();
    }
  }

  */
  std::ofstream out_stream = MakeFileStream();
  std::ifstream file("rtn_order.txt");
  std::vector<CThostFtdcOrderField> orders;
  try {
    boost::archive::text_iarchive ia(file);
    while (true) {
      CThostFtdcOrderField order;
      memset(&order, 0, sizeof(CThostFtdcOrderField));
      ia >> order;
      orders.push_back(order);
    }
  } catch (boost::archive::archive_exception& exp) {
    std::cout << "exception " << exp.what() << "\n";
  }

  CtpOrderDispatcher ctp_order_dispatcher;
  for (auto order : orders) {
    if (auto order_opt = ctp_order_dispatcher.HandleRtnOrder(order)) {
      out_stream << order_opt->order_no << "," << order_opt->instrument << ","
                 << order_opt->order_direction << "," << order_opt->order_status
                 << "," << order_opt->order_price << "," << order_opt->volume
                 << "\n";
    }
  }
  return 0;
}
