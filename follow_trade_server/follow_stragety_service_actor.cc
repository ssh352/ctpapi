#include "follow_stragety_service_actor.h"
#include <boost/lexical_cast.hpp>
#include "follow_trade_server/caf_ctp_util.h"
#include "follow_trade_server/caf_defines.h"
#include "follow_trade_server/ctp_util.h"
#include "follow_trade_server/util.h"
#include "websocket_util.h"

static const int kAdultAge = 5;

using DisplayPortfolioAtom = caf::atom_constant<caf::atom("dp")>;
using RtnOrderAtom = caf::atom_constant<caf::atom("ro")>;
using CTARtnOrderAtom = caf::atom_constant<caf::atom("cta_ro")>;

FollowStragetyServiceActor::FollowStragetyServiceActor(
    caf::actor_config& cfg,
    Server* websocket_server,
    const std::string& master_account_id,
    const std::string& slave_account_id,
    std::vector<OrderPosition> master_init_positions,
    std::vector<OrderData> master_history_rtn_orders,
    caf::actor cta,
    std::unique_ptr<ctp_bind::Trader> trader,
    caf::actor monitor)
    : caf::event_based_actor(cfg),
      websocket_server_(websocket_server),
      cta_(cta),
      trader_(std::move(trader)),
      monitor_(monitor),
      master_account_id_(master_account_id),
      slave_account_id_(slave_account_id) {
  send(monitor_, master_account_id, master_init_positions);
  send(monitor_, master_history_rtn_orders);
  master_init_positions_ = std::move(master_init_positions);
  master_history_rtn_orders_ = std::move(master_history_rtn_orders);
  portfolio_age_ = 0;
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
  // trader_->InitAsio();
  trader_->Connect([=](CThostFtdcRspUserLoginField* rsp_field,
                       CThostFtdcRspInfoField* rsp_info) {
    if (rsp_info != NULL && rsp_info->ErrorID == 0) {
    }
  });
  /*
  caf::scoped_actor block_self(system());
  if (!Logon(follow_)) {
    caf::aout(block_self) << slave_account_id_ << " fail!\n";
    return {};
  }

  */

  /*
  // portfolio_.InitYesterdayPosition(init_positions);

  std::this_thread::sleep_for(std::chrono::seconds(1));
  SettlementInfoConfirm(follow_);

  send(cta_, CTPSubscribeRtnOrderAtom::value);
  send(follow_, CTPSubscribeRtnOrderAtom::value);
  */

  auto stragetys = {"Foo", "Bar"};

  for (auto s : stragetys) {
    master_context_ = std::make_shared<OrdersContext>(master_account_id_);
    auto slave_context = std::make_shared<OrdersContext>(slave_account_id_);

    auto cta_strategy = std::make_shared<CTAGenericStrategy>(s);
    cta_strategy->Subscribe(this);
    auto signal = std::make_shared<CTASignal>();
    signal->SetOrdersContext(master_context_, slave_context);
    auto signal_dispatch = std::make_shared<CTASignalDispatch>(signal);
    signal_dispatch->SubscribeEnterOrderObserver(cta_strategy);
    signal_dispatch->SetOrdersContext(master_context_, slave_context);

    // WARRNING:probably cann't quit program
    signal_dispatch->SubscribePortfolioObserver(
        std::make_shared<PortfolioProxy<DisplayPortfolioAtom>>(
            caf::actor_cast<caf::actor>(this),
            boost::lexical_cast<std::string>(s)));

    signal_dispatchs_.push_back(signal_dispatch);
  }

  return {[=]() {

          },
          [=](CTARtnOrderAtom, boost::shared_ptr<OrderField> order) {
            std::for_each(signal_dispatchs_.begin(), signal_dispatchs_.end(),
                          std::bind(&CTASignalDispatch::RtnOrder,
                                    std::placeholders::_1, order));
          },
          [=](RtnOrderAtom, boost::shared_ptr<OrderField> order) {
            if (order.account_id() == master_account_id_) {
            } else {
              // service_.RtnOrder(order);
              portfolio_.OnRtnOrder(std::move(field));
            }
            send(monitor_, std::move(order));
            /*
            if (portfolio_age_ != 0) {
              portfolio_age_ = 0;
            } else {
              ++portfolio_age_;
              delayed_send(this, std::chrono::milliseconds(100),
                           DisplayPortfolioAtom::value);
            }
            */
          },
          [=](DisplayPortfolioAtom, std::string stragety_id,
              std::vector<AccountPortfolio> portfolio) {
            //             send(monitor_, slave_account_id_ + ":" + stragety_id,
            //                  master_context_->GetAccountPortfolios(),
            //                  portfolio, true);

            if (auto hdl = hdl_.lock()) {
              websocket_server_->send(
                  hdl,
                  MakePortfoilioJson(master_account_id_,
                                     master_context_->GetAccountPortfolios(),
                                     slave_account_id_, std::move(portfolio)),
                  websocketpp::frame::opcode::text);
            }
          },
          [=](StragetyPortfilioAtom, connection_hdl hdl) {
            hdl_ = hdl;

          }};
}
