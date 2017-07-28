#include "follow_stragety_service_actor.h"
#include <boost/lexical_cast.hpp>
#include "follow_trade_server/caf_ctp_util.h"
#include "follow_trade_server/caf_defines.h"
#include "follow_trade_server/util.h"
#include "websocket_util.h"

static const int kAdultAge = 5;

using DisplayPortfolioAtom = caf::atom_constant<caf::atom("dp")>;
using RtnOrderAtom = caf::atom_constant<caf::atom("ro")>;
using CTARtnOrderAtom = caf::atom_constant<caf::atom("cta_ro")>;
using TraderConnectAtom = caf::atom_constant<caf::atom("connect")>;

FollowStragetyServiceActor::FollowStragetyServiceActor(caf::actor_config& cfg,
                                                       ctp_bind::Trader* trader,
                                                       std::string account_id)
    : caf::event_based_actor(cfg),
      trader_(trader),
      master_account_id_(std::move(account_id)) {
  strategy_server_.SubscribeEnterOrderObserver(this);
}

void FollowStragetyServiceActor::OpenOrder(const std::string& strategy_id,
                                           const std::string& instrument,
                                           const std::string& order_id,
                                           OrderDirection direction,
                                           double price,
                                           int quantity) {
  trader_->LimitOrder(strategy_id, order_id, instrument, PositionEffect::kOpen,
                      direction, price, quantity);
}

void FollowStragetyServiceActor::CloseOrder(const std::string& strategy_id,
                                            const std::string& instrument,
                                            const std::string& order_id,
                                            OrderDirection direction,
                                            PositionEffect position_effect,
                                            double price,
                                            int quantity) {
  trader_->LimitOrder(strategy_id, order_id, instrument, position_effect,
                      direction, price, quantity);
}

void FollowStragetyServiceActor::CancelOrder(const std::string& strategy_id,
                                             const std::string& order_id) {
  trader_->CancelOrder(strategy_id, order_id);
}

caf::behavior FollowStragetyServiceActor::make_behavior() {

  auto stragetys = {"Foo", "Bar"};

  for (auto s : stragetys) {
    auto master_context_ = std::make_shared<OrdersContext>(master_account_id_);
    auto slave_context = std::make_shared<OrdersContext>(s);

    auto cta_strategy = std::make_shared<CTAGenericStrategy>(s);
    cta_strategy->Subscribe(&strategy_server_);
    
    auto signal = std::make_shared<CTASignal>();
    signal->SetOrdersContext(master_context_, slave_context);
    auto signal_dispatch = std::make_shared<CTASignalDispatch>(signal);
    signal_dispatch->SubscribeEnterOrderObserver(cta_strategy);
    signal_dispatch->SetOrdersContext(master_context_, slave_context);

    strategy_server_.SubscribeRtnOrderObserver(s, signal_dispatch);

    // WARRNING:probably cann't quit program
    // signal_dispatch->SubscribePortfolioObserver(
    //     std::make_shared<PortfolioProxy<DisplayPortfolioAtom>>(
    //         caf::actor_cast<caf::actor>(this),
    //         boost::lexical_cast<std::string>(s)));

    trader_->SubscribeRtnOrder(s, [=](boost::shared_ptr<OrderField> order) {
      send(this, CTARtnOrderAtom::value, order);
    });
  }

  return {
      [=](CTASignalInitAtom) {
        become(caf::keep_behavior,
               [=](CTASignalRtnOrderAtom, boost::shared_ptr<OrderField> order) {
                 return caf::skip();
               },
               [=](CTASignalRtnOrderAtom,
                   const boost::shared_ptr<OrderField>& order) { caf::skip(); },
               [=](TraderConnectAtom, bool sccuess) { unbecome(); });

        trader_->Connect([=](CThostFtdcRspUserLoginField* rsp_field,
                             CThostFtdcRspInfoField* rsp_info) {
          if (rsp_info != NULL && rsp_info->ErrorID == 0) {
            send(this, TraderConnectAtom::value, true);
          }
        });
      },
      [=](CTASignalRtnOrderAtom, const boost::shared_ptr<OrderField>& order) {
        strategy_server_.RtnOrder(order);
      },
      [=](CTARtnOrderAtom, const boost::shared_ptr<OrderField>& order) {
        strategy_server_.RtnOrder(order);
      }};
}

//           [=](CTARtnOrderAtom, boost::shared_ptr<OrderField> order) {
//             std::for_each(signal_dispatchs_.begin(),
//             signal_dispatchs_.end(),
//                           std::bind(&CTASignalDispatch::RtnOrder,
//                                     std::placeholders::_1, order));
//           },
//           [=](RtnOrderAtom, boost::shared_ptr<OrderField> order) {
//             if (order.account_id() == master_account_id_) {
//             } else {
//               // service_.RtnOrder(order);
//               portfolio_.OnRtnOrder(std::move(field));
//             }
//             send(monitor_, std::move(order));
//             /*
//             if (portfolio_age_ != 0) {
//               portfolio_age_ = 0;
//             } else {
//               ++portfolio_age_;
//               delayed_send(this, std::chrono::milliseconds(100),
//                            DisplayPortfolioAtom::value);
//             }
//             */
//           },
//           [=](DisplayPortfolioAtom, std::string stragety_id,
//               std::vector<AccountPortfolio> portfolio) {
//             //             send(monitor_, slave_account_id_ + ":" +
//             stragety_id,
//             // master_context_->GetAccountPortfolios(),
//             //                  portfolio, true);
//
//             if (auto hdl = hdl_.lock()) {
//               websocket_server_->send(
//                   hdl,
//                   MakePortfoilioJson(master_account_id_,
//                                      master_context_->GetAccountPortfolios(),
//                                      slave_account_id_,
//                                      std::move(portfolio)),
//                   websocketpp::frame::opcode::text);
//             }
//           },
//           [=](StragetyPortfilioAtom, connection_hdl hdl) {
//             hdl_ = hdl;
//
//           }
