#ifndef FOLLOW_TRADE_CLOSE_CORR_ORDERS_MANAGER_H
#define FOLLOW_TRADE_CLOSE_CORR_ORDERS_MANAGER_H
#include "geek_quant/caf_defines.h"

class CloseCorrOrdersManager {
 public:
  std::vector<std::pair<std::string, int> > GetCorrOrderQuantiys(
      const std::string& order_id) const;

  std::vector<std::string> GetCloseCorrOrderIds(
      const std::string& order_id) const;

  bool IsNewCloseOrder(const OrderData& rtn_order) const;

  void AddCloseCorrOrders(
      const std::string& order_id,
      std::vector<std::pair<std::string, int> > corr_orders);

 private:
  std::map<std::string, std::vector<std::pair<std::string, int> > >
      close_corr_orders_;
};

#endif  // FOLLOW_TRADE_CLOSE_CORR_ORDERS_MANAGER_H
