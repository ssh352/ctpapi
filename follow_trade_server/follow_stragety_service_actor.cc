#include "follow_stragety_service_actor.h"
#include <boost/lexical_cast.hpp>
#include "follow_trade_server/atom_defines.h"
#include "follow_trade_server/caf_ctp_util.h"
#include "follow_trade_server/util.h"
#include "websocket_util.h"

static const int kAdultAge = 5;

FollowStragetyServiceActor::FollowStragetyServiceActor(caf::actor_config& cfg,
                                                       caf::actor trader,
                                                       caf::actor cta_signal,
                                                       std::string account_id)
    : caf::event_based_actor(cfg),
      trader_(trader),
      cta_signal_(cta_signal),
      master_account_id_(std::move(account_id)) {
  strategy_server_.SubscribeEnterOrderObserver(this);
  db_ = system().registry().get(caf::atom("db"));
}

void FollowStragetyServiceActor::OpenOrder(const std::string& strategy_id,
                                           const std::string& instrument,
                                           const std::string& order_id,
                                           OrderDirection direction,
                                           double price,
                                           int quantity) {
  send(trader_, strategy_id, order_id, instrument, PositionEffect::kOpen,
       direction, price, quantity);
}

void FollowStragetyServiceActor::CloseOrder(const std::string& strategy_id,
                                            const std::string& instrument,
                                            const std::string& order_id,
                                            OrderDirection direction,
                                            PositionEffect position_effect,
                                            double price,
                                            int quantity) {
  send(trader_, strategy_id, order_id, instrument, position_effect, direction,
       price, quantity);
}

void FollowStragetyServiceActor::CancelOrder(const std::string& strategy_id,
                                             const std::string& order_id) {
  send(trader_, strategy_id, order_id);
}

caf::behavior FollowStragetyServiceActor::make_behavior() {
  auto block_trader = caf::make_function_view(trader_);
  auto block_cta_signal = caf::make_function_view(cta_signal_);

  auto r = block_cta_signal(SubscribeRtnOrderAtom::value,
                            caf::actor_cast<caf::strong_actor_ptr>(this));
  std::list<boost::shared_ptr<OrderField>> cta_rtn_orders;
  if (r) {
    cta_rtn_orders = r->get_as<std::list<boost::shared_ptr<OrderField>>>(0);
  }

  r = block_cta_signal(QueryInverstorPositionAtom::value);
  std::vector<OrderPosition> cta_inverstor_positions;
  if (r) {
    cta_inverstor_positions = r->get_as<std::vector<OrderPosition>>(0);
  }

  auto stragetys = {"Foo", "Bar"};
  for (auto s : stragetys) {
    auto master_context_ = std::make_shared<OrdersContext>(master_account_id_);
    for (auto o : cta_rtn_orders) {
      master_context_->HandleRtnOrder(o);
    }

    master_context_->InitPositions(cta_inverstor_positions);

    auto slave_context = std::make_shared<OrdersContext>(s);

    auto r = block_trader(SubscribeRtnOrderAtom::value, s,
                          caf::actor_cast<caf::strong_actor_ptr>(this));
    if (r) {
      auto orders = r->get_as<std::list<boost::shared_ptr<OrderField>>>(0);
      for (auto order : orders) {
        (void)slave_context->HandleRtnOrder(order);
      }
    }
    r = block_trader(QueryInverstorPositionAtom::value, s);
    if (r) {
      slave_context->InitPositions(r->get_as<std::vector<OrderPosition>>(0));
      
    }

    auto cta_strategy = std::make_shared<CTAGenericStrategy>(s);
    cta_strategy->Subscribe(&strategy_server_);

    auto signal = std::make_shared<CTASignal>();
    signal->SetOrdersContext(master_context_, slave_context);
    auto signal_dispatch = std::make_shared<CTASignalDispatch>(signal);
    signal_dispatch->SubscribeEnterOrderObserver(cta_strategy);
    signal_dispatch->SetOrdersContext(master_context_, slave_context);

    strategy_server_.SubscribeRtnOrderObserver(s, signal_dispatch);

  }

  return {[=](RtnOrderAtom, const boost::shared_ptr<OrderField>& order) {
    strategy_server_.RtnOrder(order);
  }};
}