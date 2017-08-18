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
  send(trader_, LimitOrderAtom::value, strategy_id, order_id, instrument,
       PositionEffect::kOpen, direction, price, quantity);
}

void FollowStragetyServiceActor::CloseOrder(const std::string& strategy_id,
                                            const std::string& instrument,
                                            const std::string& order_id,
                                            OrderDirection direction,
                                            PositionEffect position_effect,
                                            double price,
                                            int quantity) {
  send(trader_, LimitOrderAtom::value, strategy_id, order_id, instrument,
       position_effect, direction, price, quantity);
}

void FollowStragetyServiceActor::CancelOrder(const std::string& strategy_id,
                                             const std::string& order_id) {
  send(trader_, CancelOrderAtom::value, strategy_id, order_id);
}

caf::behavior FollowStragetyServiceActor::make_behavior() {
  auto block_trader = caf::make_function_view(trader_);
  std::vector<std::string> stragetys = {"Foo", "Bar"};
  std::shared_ptr<int> leave_reply =
      std::make_shared<int>(2 + stragetys.size() * 2);
  auto cta_rtn_orders =
      std::make_shared<std::list<boost::shared_ptr<OrderField>>>();
  auto cta_inverstor_positions = std::make_shared<std::vector<OrderPosition>>();
  auto strategy_orders = std::make_shared<
      std::map<std::string, std::list<boost::shared_ptr<OrderField>>>>();
  auto strategy_positions =
      std::make_shared<std::map<std::string, std::vector<OrderPosition>>>();

  auto init_strategy_func = [=]() {
    for (auto s : stragetys) {
      auto master_context_ =
          std::make_shared<OrdersContext>(master_account_id_);
      for (auto o : *cta_rtn_orders) {
        master_context_->HandleRtnOrder(o);
      }

      master_context_->InitPositions(*cta_inverstor_positions);

      auto slave_context = std::make_shared<OrdersContext>(s);
      for (auto order : (*strategy_orders)[s]) {
        (void)slave_context->HandleRtnOrder(order);
      }
      slave_context->InitPositions((*strategy_positions)[s]);

      auto cta_strategy = std::make_shared<CTAGenericStrategy>(s);
      cta_strategy->Subscribe(&strategy_server_);

      auto signal = std::make_shared<CTASignal>();
      signal->SetOrdersContext(master_context_, slave_context);
      auto signal_dispatch = std::make_shared<CTASignalDispatch>(signal);
      signal_dispatch->SubscribeEnterOrderObserver(cta_strategy);
      signal_dispatch->SetOrdersContext(master_context_, slave_context);

      strategy_server_.SubscribeRtnOrderObserver(s, signal_dispatch);
    }
  };

  request(cta_signal_, caf::infinite, SubscribeRtnOrderAtom::value,
          caf::actor_cast<caf::strong_actor_ptr>(this))
      .then([=](const std::list<boost::shared_ptr<OrderField>>& orders) {
        std::copy(orders.begin(), orders.end(),
                  std::back_inserter(*cta_rtn_orders));
        if (--(*leave_reply) == 0) {
          init_strategy_func();
        }
      });

  request(cta_signal_, caf::infinite, QueryInverstorPositionAtom::value)
      .then([=](std::vector<OrderPosition> order_positions) {
        std::copy(order_positions.begin(), order_positions.end(),
                  std::back_inserter(*cta_inverstor_positions));
        if (--(*leave_reply) == 0) {
          init_strategy_func();
        }
      });

  for (const auto& strategy : stragetys) {
    request(trader_, caf::infinite, SubscribeRtnOrderAtom::value, strategy,
            caf::actor_cast<caf::strong_actor_ptr>(this))
        .then([=](std::list<boost::shared_ptr<OrderField>> orders) {
          strategy_orders->insert({strategy, std::move(orders)});
          if (--(*leave_reply) == 0) {
            init_strategy_func();
          }
        });
    request(trader_, caf::infinite, QueryInverstorPositionAtom::value, strategy)
        .then([=](std::vector<OrderPosition> positions) {
          strategy_positions->insert({strategy, std::move(positions)});
          if (--(*leave_reply) == 0) {
            init_strategy_func();
          }
        });
  }

  return {[=](RtnOrderAtom, const boost::shared_ptr<OrderField>& order) {
    strategy_server_.RtnOrder(order);
  }};
}
