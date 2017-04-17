#include "follow_stragety_service_actor.h"
#include <boost/lexical_cast.hpp>
#include "follow_trade_server/caf_ctp_util.h"
#include "follow_trade_server/caf_defines.h"
#include "follow_trade_server/util.h"

FollowStragetyServiceActor::FollowStragetyServiceActor(
    caf::actor_config& cfg,
    const std::string& master_account_id,
    const std::string& slave_account_id,
    std::vector<OrderPosition> master_init_positions,
    std::vector<OrderData> master_history_rtn_orders,
    caf::actor cta,
    caf::actor follow,
    caf::actor monitor)
    : caf::event_based_actor(cfg),
      service_(master_account_id, slave_account_id, this, 1000),
      cta_(cta),
      follow_(follow),
      monitor_(monitor) {
  send(monitor_, master_account_id, master_init_positions);
  send(monitor_, master_history_rtn_orders);
  service_.InitPositions(master_account_id, std::move(master_init_positions));
  service_.InitRtnOrders(std::move(master_history_rtn_orders));
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
  if (auto order = service_.context().GetOrderData(service_.slave_account_id(),
                                                   order_no)) {
    send(follow_, CTPCancelOrderAtom::value, order_no, order->order_sys_id(),
         order->exchange_id());
  }
}

caf::behavior FollowStragetyServiceActor::make_behavior() {
  caf::scoped_actor block_self(system());
  if (!Logon(follow_)) {
    caf::aout(block_self) << service_.slave_account_id() << " fail!\n";
    return {};
  }

  auto init_positions = BlockRequestInitPositions(follow_);
  auto history_orders = BlockRequestHistoryOrder(follow_);
  send(monitor_, service_.slave_account_id(), init_positions);
  send(monitor_, history_orders);

  service_.InitPositions(service_.slave_account_id(),
                         std::move(init_positions));

  service_.InitRtnOrders(std::move(history_orders));

  std::this_thread::sleep_for(std::chrono::seconds(1));
  SettlementInfoConfirm(follow_);

  send(cta_, CTPSubscribeRtnOrderAtom::value);
  send(follow_, CTPSubscribeRtnOrderAtom::value);

  // caf::aout(block_self) << service_.slave_account_id() << " ready.\n";
  // auto mp =
  // service_.context().GetAccountProfolios(service_.master_account_id());
  send(monitor_, service_.slave_account_id(),
       service_.context().GetAccountProfolios(service_.master_account_id()),
       service_.context().GetAccountProfolios(service_.slave_account_id()),
       true);

  // ss << std::setw(80) << std::setfill('=') << "=" << "\n";

  // caf::aout(block_self) << ss.str() << "\n";
  return {[=](CTPRtnOrderAtom, OrderData order) {
    service_.HandleRtnOrder(order);
    send(monitor_, order);
    send(monitor_, service_.slave_account_id(),
         service_.context().GetAccountProfolios(service_.master_account_id()),
         service_.context().GetAccountProfolios(service_.slave_account_id()),
         false);
  }};
}
