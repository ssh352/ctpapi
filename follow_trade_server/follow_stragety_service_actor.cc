#include "follow_stragety_service_actor.h"
#include <boost/lexical_cast.hpp>
#include "follow_trade_server/caf_ctp_util.h"
#include "follow_trade_server/caf_defines.h"
#include "follow_trade_server/ctp_util.h"
#include "follow_trade_server/util.h"

static const int kAdultAge = 5;

using DisplayPortfolioAtom = caf::atom_constant<caf::atom("dp")>;

FollowStragetyServiceActor::FollowStragetyServiceActor(
    caf::actor_config& cfg,
    Server* websocket_server,
    const std::string& master_account_id,
    const std::string& slave_account_id,
    std::vector<OrderPosition> master_init_positions,
    std::vector<OrderData> master_history_rtn_orders,
    caf::actor cta,
    caf::actor follow,
    caf::actor monitor)
    : caf::event_based_actor(cfg),
      websocket_server_(websocket_server),
      cta_(cta),
      follow_(follow),
      monitor_(monitor),
      master_account_id_(master_account_id),
      slave_account_id_(slave_account_id) {
  send(monitor_, master_account_id, master_init_positions);
  send(monitor_, master_history_rtn_orders);
  master_init_positions_ = std::move(master_init_positions);
  master_history_rtn_orders_ = std::move(master_history_rtn_orders);
  portfolio_age_ = 0;
  service_.SubscribeEnterOrderObserver(this);
}

void FollowStragetyServiceActor::OpenOrder(const std::string& instrument,
                                           const std::string& order_no,
                                           OrderDirection direction,
                                           OrderPriceType price_type,
                                           double price,
                                           int quantity) {
  send(follow_, CTPReqOpenOrderAtom::value, instrument, order_no, direction,
       price_type, price, quantity);
}

void FollowStragetyServiceActor::CloseOrder(const std::string& instrument,
                                            const std::string& order_no,
                                            OrderDirection direction,
                                            PositionEffect position_effect,
                                            OrderPriceType price_type,
                                            double price,
                                            int quantity) {
  send(follow_, CTPReqCloseOrderAtom::value, instrument, order_no, direction,
       position_effect, price_type, price, quantity);
}

void FollowStragetyServiceActor::CancelOrder(const std::string& order_no) {
  send(follow_, CTPCancelOrderAtom::value, order_no);
}

caf::behavior FollowStragetyServiceActor::make_behavior() {
  caf::scoped_actor block_self(system());
  if (!Logon(follow_)) {
    caf::aout(block_self) << slave_account_id_ << " fail!\n";
    return {};
  }

  // portfolio_.InitYesterdayPosition(init_positions);

  std::this_thread::sleep_for(std::chrono::seconds(1));
  SettlementInfoConfirm(follow_);

  send(cta_, CTPSubscribeRtnOrderAtom::value);
  send(follow_, CTPSubscribeRtnOrderAtom::value);

  for (int i = 0; i < 10; ++i) {
    master_context_ = std::make_shared<OrdersContext>(master_account_id_);
    auto slave_context = std::make_shared<OrdersContext>(slave_account_id_);

    auto cta_strategy = std::make_shared<CTAGenericStrategy>(
        boost::lexical_cast<std::string>(i));
    cta_strategy->Subscribe(&service_);
    auto signal = std::make_shared<CTASignal>();
    signal->SetOrdersContext(master_context_, slave_context);
    auto signal_dispatch = std::make_shared<CTASignalDispatch>(signal);
    signal_dispatch->SubscribeEnterOrderObserver(cta_strategy);
    signal_dispatch->SetOrdersContext(master_context_, slave_context);

    // WARRNING:probably cann't quit program
    signal_dispatch->SubscribePortfolioObserver(
        std::make_shared<PortfolioProxy<DisplayPortfolioAtom>>(
            caf::actor_cast<caf::actor>(this),
            boost::lexical_cast<std::string>(i)));

    service_.SubscribeRtnOrderObserver(boost::lexical_cast<std::string>(i),
                                       signal_dispatch);
    signal_dispatchs_.push_back(signal_dispatch);
  }

  return {[=](CTPRtnOrderAtom, CThostFtdcOrderField field) {
            OrderData order = MakeOrderData(field);
            if (order.account_id() == master_account_id_) {
              std::pair<TThostFtdcSessionIDType, std::string> key =
                  std::make_pair(field.SessionID, field.OrderRef);
              if (master_adjust_order_ids_.find(key) !=
                  master_adjust_order_ids_.end()) {
                order.order_id_ = master_adjust_order_ids_[key];
              } else {
                order.order_id_ = boost::lexical_cast<std::string>(
                    master_adjust_order_ids_.size());
                master_adjust_order_ids_.insert({key, order.order_id_});
              }

              std::for_each(signal_dispatchs_.begin(), signal_dispatchs_.end(),
                            std::bind(&CTASignalDispatch::RtnOrder,
                                      std::placeholders::_1, order));

            } else {
              service_.RtnOrder(order);
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
            websocket_server_->send(hdl_, "hello",
                                    websocketpp::frame::opcode::text);
          },
          [=](StragetyPortfilioAtom, connection_hdl hdl) { 
            hdl_ = hdl; 

          }};
}
