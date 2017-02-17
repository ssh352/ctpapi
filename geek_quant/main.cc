#include <string>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "caf/all.hpp"
#include "tradeapi/ThostFtdcTraderApi.h"

using std::endl;
using std::string;

using namespace caf;


using LoginAtom = atom_constant<atom("login")>;
using OrderPushAtom = atom_constant<atom("order_push")>;

using Broker = typed_actor<reacts_to<LoginAtom>, reacts_to<OrderPushAtom> >;

Broker::behavior_type CtpBroker(event_based_actor* self) {
  return {
    [=](LoginAtom) {
      aout(self) << "LoginAtom Message \n";
    },
    [](OrderPushAtom) {

    }
  };
}

class CTPTradingSpi : public CThostFtdcTraderSpi {
public:
  CTPTradingSpi(actor_system& system) : system_(system) {
    api_ = CThostFtdcTraderApi::CreateFtdcTraderApi();
  }
 

  void LoginServer(const std::string& user_id,
                    const std::string password) {
    broker_ = actor_cast<strong_actor_ptr>(system_.spawn(CtpBroker));
    // anon_send(system_.spawn(CtpBroker), LoginAtom::value);
    user_id_ = user_id;
    password_ = password;

    api_->RegisterSpi(this);
    api_->RegisterFront("tcp://180.168.146.187:10000");
    api_->SubscribePublicTopic(THOST_TERT_RESTART);
    api_->SubscribePrivateTopic(THOST_TERT_RESUME);
    api_->Init();
  }

  void Join() {
    api_->Join();
  }

	virtual void OnFrontConnected() override {
    std::cout << "OnFrontConnected\n";
    CThostFtdcReqUserLoginField req;
    memset(&req, 0, sizeof(req));
    strcpy(req.BrokerID, "9999");
    strcpy(req.UserID, user_id_.c_str());
    strcpy(req.Password, password_.c_str());
    int iResult = api_->ReqUserLogin(&req, 0);
  };

  virtual void OnFrontDisconnected(int nReason) override {
    
  }

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
                               CThostFtdcRspInfoField *pRspInfo,
                               int nRequestID,
                               bool bIsLast) override {

    anon_send(actor_cast<actor>(broker_), LoginAtom::value);
    if (pRspInfo->ErrorID == 1) {
      std::cout << "OnRspUserLogin\n";
    } else {
      std::cout << "User Login Error:" << pRspInfo->ErrorMsg << "\n";
    }
  }
private:
  actor_system& system_;
  CThostFtdcTraderApi* api_;
  strong_actor_ptr broker_;
  std::string user_id_;
  std::string password_;
};





behavior mirror(event_based_actor* self) {
  // return the (initial) actor behavior
  return {
    // a handler for messages containing a single string
    // that replies with a string
    [=](const string& what) -> string {
      // prints "Hello World!" via aout (thread-safe cout wrapper)
      aout(self) << what << endl;
      // reply "!dlroW olleH"
      return string(what.rbegin(), what.rend());
    }
  };
}

void hello_world(event_based_actor* self, const actor& buddy, boost::shared_ptr<CTPTradingSpi> ctp_broker) {
  self->request(buddy, std::chrono::seconds(10), "Hello World!").then(
    // ... wait up to 10s for a response ...
    [=](const string& what) {
      // ... and print it
      aout(self) << what << endl;
    }
  );
}

int main() {
  // our CAF environment
  actor_system_config cfg;
  actor_system system{cfg};

  auto ctp_broker = boost::make_shared<CTPTradingSpi>(system);
  ctp_broker->LoginServer("053867", "8661188");
  // system will wait until both actors are destroyed before leaving main
  std::string input;
  while (std::cin >> input) {
    if (input == "exit") {
      break;
    }
  }
}
