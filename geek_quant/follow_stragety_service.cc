#include "follow_stragety_service.h"

FollowStragetyService::FollowStragetyService(const std::string& master_account,
                                             const std::string& slave_account,
                                             Delegate* delegate)
    : stragety_(master_account, slave_account, this),
      delegate_(delegate),
      master_account_(master_account),
      slave_account_(slave_account) {}

void FollowStragetyService::HandleRtnOrder(RtnOrderData rtn_order) {
  if (!waiting_reply_order_.empty()) {
    auto it = std::find(waiting_reply_order_.begin(),
                        waiting_reply_order_.end(), rtn_order.order_no);
    if (it != waiting_reply_order_.end()) {
      waiting_reply_order_.erase(it);
      if (waiting_reply_order_.empty()) {
        while (!outstanding_rtn_orders_.empty() &&
               waiting_reply_order_.empty()) {
          DoHandleRtnOrder(outstanding_rtn_orders_.front());
          outstanding_rtn_orders_.pop_front();
        }
        if (waiting_reply_order_.empty()) {
          DoHandleRtnOrder(rtn_order);
        } else {
          outstanding_rtn_orders_.push_back(rtn_order);
        }
      } else {
        outstanding_rtn_orders_.push_back(rtn_order);
      }
    } else {
      outstanding_rtn_orders_.push_back(rtn_order);
    }
  } else {
    DoHandleRtnOrder(rtn_order);
  }
}

void FollowStragetyService::DoHandleRtnOrder(RtnOrderData& rtn_order) {
  context_.HandlertnOrder(rtn_order);

  switch (rtn_order.order_status) {
    case OrderStatus::kOpening:
      stragety_.HandleOpening(rtn_order, context_);
      break;
    case OrderStatus::kCloseing:
      stragety_.HandleCloseing(rtn_order, context_);
      break;
    case OrderStatus::kOpened:
      break;
    case OrderStatus::kClosed:
      break;
    case OrderStatus::kOpenCanceled:
    case OrderStatus::kCloseCanceled:
      stragety_.HandleCanceled(rtn_order, context_);
      break;
    default:
      break;
  }

  /*
  order_manager.HandleRtnOrder();

  class OrderPosition {
   public:
    std::string order_no_;
    int volume_;
    int closeable_volume_;
    bool today_;
  };

  class InstrumentPosition {
   public:
    std::string instrument_;
    OrderDirection direction;
    std::vector<OrderPosition> positions_;
  };


  class Order {
  public:
  private:
    std::string order_no_;
    int traded_volume_;
    int total_volume_;
    OrderFeet open_close;
    std::vector<std::pair<std::string, int>> corr_orders_;
  };

  class OrderManager {
  public:
    void HandleRtnOrder();
  private:
    std::vector<Order> orders_;
  };

  order_manager.HandleRtnOrder(&position_manager);
  void HandleRtnOrder(&position_manager) {
    position_manager->Closeiong(order_no, -volume);
    position_manager->Cloesd(order_no, -volume);
    position_manager->CancelClose(order_no, volume);
    position_manager->OpenedOrder(instrument, order_no, direction, volume);
  }
  */
}

void FollowStragetyService::OpenOrder(const std::string& instrument,
                                      const std::string& order_no,
                                      OrderDirection direction,
                                      double price,
                                      int quantity) {
  waiting_reply_order_.push_back(order_no);
  delegate_->OpenOrder(instrument, order_no, direction, price, quantity);
}
