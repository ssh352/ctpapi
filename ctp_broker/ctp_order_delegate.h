#ifndef CTP_BROKER_CTP_ORDER_DELEGATE_H
#define CTP_BROKER_CTP_ORDER_DELEGATE_H
#include <string>
#include "common/api_struct.h"

class CTPOrderDelegate {
 public:
  virtual void EnterOrder(CTPEnterOrder enter_order) = 0;
  virtual void CancelOrder(const std::string& account_id,
                           const std::string& order_id) = 0;
  virtual void ReturnOrderField(const std::shared_ptr<OrderField>& order) = 0;
};

#endif  // CTP_BROKER_CTP_ORDER_DELEGATE_H
