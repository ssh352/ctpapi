#include "follow_strategy_service.h"

FollowStragetyService::FollowStragetyService(const std::string& master_account,
                                             const std::string& slave_account,
                                             TradeOrderDelegate* delegate,
                                             int start_order_id_seq)
    : stragety_(master_account, slave_account, delegate, this, &context_),
      master_account_(master_account),
      slave_account_(slave_account),
      order_id_mananger_(start_order_id_seq) {}

void FollowStragetyService::HandleRtnOrder(OrderData rtn_order) {
  OrderData adjust_order = order_id_mananger_.AdjustOrder(std::move(rtn_order));
  switch (BeforeHandleOrder(adjust_order)) {
    case StragetyStatus::kPending:
      outstanding_orders_.push_back(std::move(adjust_order));
      break;
    case StragetyStatus::kIdle:
      DoHandleRtnOrder(std::move(adjust_order));
      break;
    default:
      break;
  }
}

void FollowStragetyService::DoHandleRtnOrder(OrderData rtn_order) {
  switch (context_.HandlertnOrder(rtn_order)) {
    case OrderEventType::kNewOpen:
      stragety_.HandleOpening(rtn_order);
      break;
    case OrderEventType::kNewClose:
      stragety_.HandleCloseing(rtn_order);
      break;
    case OrderEventType::kOpenTraded:
      stragety_.HandleOpened(rtn_order);
      break;
    case OrderEventType::kCloseTraded:
      stragety_.HandleClosed(rtn_order);
      break;
    case OrderEventType::kCanceled:
      stragety_.HandleCanceled(rtn_order);
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

void FollowStragetyService::Trade(const std::string& order_no) {
  waiting_reply_order_.push_back(order_no);
}

FollowStragetyService::StragetyStatus FollowStragetyService::BeforeHandleOrder(
    OrderData order) {
  StragetyStatus status = waiting_reply_order_.empty()
                              ? StragetyStatus::kIdle
                              : StragetyStatus::kPending;
  if (!waiting_reply_order_.empty()) {
    auto it = std::find(waiting_reply_order_.begin(),
                        waiting_reply_order_.end(), order.order_id());
    if (it != waiting_reply_order_.end()) {
      waiting_reply_order_.erase(it);
      if (waiting_reply_order_.empty()) {
        while (!outstanding_orders_.empty() && waiting_reply_order_.empty()) {
          DoHandleRtnOrder(outstanding_orders_.front());
          outstanding_orders_.pop_front();
        }
        if (waiting_reply_order_.empty()) {
          status = StragetyStatus::kIdle;
        }
      }
    }
  }
  return status;
}
