#include "cta_trade_actor.h"
#include "follow_trade_server/caf_defines.h"
#include "follow_trade_server/ctp_util.h"

CtpTrader::CtpTrader(caf::actor_config& cfg,
                     const std::string& front_server,
                     const std::string& broker_id,
                     const std::string& user_id,
                     const std::string& password,
                     caf::actor binary_log)
    : caf::event_based_actor(cfg),
      ctp_(this, user_id + "_"),
      front_server_(front_server),
      broker_id_(broker_id),
      user_id_(user_id),
      password_(password),
      binary_log_(binary_log) {
  front_id_ = 0;
  session_id_ = 0;
}

CtpTrader::~CtpTrader() {}

void CtpTrader::OnOrderData(CThostFtdcOrderField* field) {
  // send(this, CTPRtnOrderAtom::value, MakeOrderData(field));
  send(this, CTPRtnOrderAtom::value, *field);
  send(binary_log_, *field);
}

void CtpTrader::OnLogon(int front_id, int session_id, bool success) {
  send(this, CTPRspLogin::value, front_id, session_id, success);
}

void CtpTrader::OnPositions(std::vector<OrderPosition> positions) {
  send(this, CTPRspQryInvestorPositionsAtom::value, positions);
}

void CtpTrader::OnSettlementInfoConfirm() {
  send(this, CTPRspSettlementInfoConfirm::value);
}

void CtpTrader::OnRspQryInstrumentList(
    std::vector<CThostFtdcInstrumentField> instruments) {}

void CtpTrader::OnRspQryInstrumentMarginRate(
    CThostFtdcInstrumentMarginRateField* pInstrumentMarginRate) {}

caf::behavior CtpTrader::make_behavior() {
  return {
      [=](CTPReqLogin) -> caf::result<bool> {
        ctp_.LoginServer(front_server_, broker_id_, user_id_, password_);
        auto logon_response_ = make_response_promise<bool>();
        logon_response_promises_.push_back(logon_response_);
        return logon_response_;
      },
      [=](CTPRspLogin, int front_id, int session_id, bool success) {
        front_id_ = front_id;
        session_id_ = session_id;
        for (auto promise : logon_response_promises_) {
          promise.deliver(success);
        }
        logon_response_promises_.clear();
      },
      [=](CTPReqSettlementInfoConfirm) -> caf::result<bool> {
        ctp_.SettlementInfoConfirm();
        settlement_info_confirm_response_promises_ =
            make_response_promise<bool>();
        return settlement_info_confirm_response_promises_;
      },
      [=](CTPRspSettlementInfoConfirm) {
        settlement_info_confirm_response_promises_.deliver(true);
      },
      [=](CTPReqQryInvestorPositionsAtom)
          -> caf::result<std::vector<OrderPosition>> {
        auto promise = make_response_promise<std::vector<OrderPosition>>();
        positions_response_promises.push_back(promise);
        ctp_.QryInvestorPosition();
        return promise;
      },
      [=](CTPRspQryInvestorPositionsAtom,
          std::vector<OrderPosition> positions) {
        for (auto promise : positions_response_promises) {
          promise.deliver(positions);
        }
        positions_response_promises.clear();
      },
      [=](CTPSubscribeRtnOrderAtom) {
        rtn_orders_subscribers_.push_back(current_sender());
      },
      [=](CTPRtnOrderAtom, CThostFtdcOrderField order) {
        for (auto subscriber : rtn_orders_subscribers_) {
          send(caf::actor_cast<caf::actor>(subscriber), CTPRtnOrderAtom::value,
               order);
        }
        rtn_orders_.push_back(std::move(order));
      },
      [=](CTPReqHistoryRtnOrdersAtom,
          size_t start_seq) -> caf::result<std::vector<OrderData>> {
        std::vector<OrderData> orders;
        orders.resize(rtn_orders_.size() - start_seq);
        std::transform(rtn_orders_.begin() + start_seq, rtn_orders_.end(),
                       orders.begin(),
                       std::bind(&MakeOrderData, std::placeholders::_1));
        return orders;
      },
      [=](CTPReqOpenOrderAtom, const std::string& instrument,
          const std::string& order_id, OrderDirection direction, double price,
          int quantity) {
        ctp_.OrderInsert(
            MakeCtpOpenOrder(instrument, order_id, direction, price, quantity));
      },
      [=](CTPReqCloseOrderAtom, const std::string& instrument,
          const std::string& order_id, OrderDirection direction,
          PositionEffect position_effect, double price, int quantity) {
        ctp_.OrderInsert(MakeCtpCloseOrder(instrument, order_id, direction,
                                           position_effect, price, quantity));
      },
      [=](CTPCancelOrderAtom, std::string order_id) {
        ctp_.OrderAction(
            MakeCtpCancelOrderAction(front_id_, session_id_, order_id));
      },
      /*
    [=](ActorTimerAtom) {
      if (last_check_rtn_order_size_ == restart_rtn_orders_.size()) {
        for (auto promise : restart_rtn_orders_response_promises_) {
          promise.deliver(restart_rtn_orders_);
        }
        restart_rtn_orders_response_promises_.clear();
      } else {
        last_check_rtn_order_size_ = restart_rtn_orders_.size();
        delayed_send(this, std::chrono::seconds(1), ActorTimerAtom::value);
      }
    },
      */
  };
}
