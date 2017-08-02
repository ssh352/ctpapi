#include "caf_ctp_util.h"
#include "follow_trade_server/atom_defines.h"
/*


bool Logon(caf::actor actor) {
  auto f = caf::make_function_view(actor, {caf::time_unit::seconds, 3});

  auto ret = f(CTPReqLogin::value);
  if (!ret || !ret->get_as<bool>(0)) {
    return false;
  }
  return true;
}

std::vector<OrderPosition> BlockRequestInitPositions(caf::actor actor) {
  auto f = caf::make_function_view(actor);
  return f(CTPReqQryInvestorPositionsAtom::value)
      ->get_as<std::vector<OrderPosition>>(0);
}

std::vector<OrderField> BlockRequestHistoryOrder(caf::actor actor) {
  auto f = caf::make_function_view(actor);

  std::vector<OrderField> history_orders;
  size_t next_seq = 0;
  while (true) {
    auto ret = f(CTPReqHistoryRtnOrdersAtom::value, next_seq);
    auto orders = ret->get_as<std::vector<OrderField>>(0);
    if (orders.empty()) {
      break;
    }
    history_orders.insert(history_orders.end(), orders.begin(), orders.end());
    next_seq = history_orders.size();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  return history_orders;
}

void SettlementInfoConfirm(caf::actor actor) {
  auto f = caf::make_function_view(actor);
  f(CTPReqSettlementInfoConfirm::value);
}
*/

