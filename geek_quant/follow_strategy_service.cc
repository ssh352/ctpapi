#include "follow_strategy_service.h"

FollowStragetyService::FollowStragetyService(const std::string& master_account,
                                             const std::string& slave_account,
                                             Delegate* delegate,
                                             int start_order_id_seq)
    : stragety_(master_account, slave_account, this, &context_),
      delegate_(delegate),
      master_account_(master_account),
      slave_account_(slave_account),
      context_(start_order_id_seq) {}

void FollowStragetyService::HandleRtnOrder(OrderData rtn_order) {
  OrderData adjust_order = context_.AdjustOrder(std::move(rtn_order));
  switch (BeforeHandleOrder(adjust_order)) {
    case StragetyStatus::kWaitReply:
      outstanding_orders_.push_back(std::move(adjust_order));
      break;
    case StragetyStatus::kReady:
      DoHandleRtnOrder(std::move(adjust_order));
      break;
    case StragetyStatus::kSkip:
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

void FollowStragetyService::OpenOrder(const std::string& instrument,
                                      const std::string& order_no,
                                      OrderDirection direction,
                                      OrderPriceType price_type,
                                      double price,
                                      int quantity) {
  Trade(order_no);
  delegate_->OpenOrder(instrument, order_no, direction, price_type, price,
                       quantity);
}
void FollowStragetyService::CloseOrder(const std::string& instrument,
                                       const std::string& order_no,
                                       OrderDirection direction,
                                       PositionEffect position_effect,
                                       OrderPriceType price_type,
                                       double price,
                                       int quantity) {
  Trade(order_no);
  delegate_->CloseOrder(instrument, order_no, direction, position_effect,
                        price_type, price, quantity);
}

void FollowStragetyService::CancelOrder(const std::string& order_no) {
  Trade(order_no);
  delegate_->CancelOrder(order_no);
}

FollowStragetyService::StragetyStatus FollowStragetyService::BeforeHandleOrder(
    OrderData order) {
  StragetyStatus status = waiting_reply_order_.empty()
                              ? StragetyStatus::kReady
                              : StragetyStatus::kWaitReply;
  if (!waiting_reply_order_.empty() && order.account_id() == slave_account_) {
    auto it = std::find(waiting_reply_order_.begin(),
                        waiting_reply_order_.end(), order.order_id());
    if (it != waiting_reply_order_.end()) {
      waiting_reply_order_.erase(it);
      DoHandleRtnOrder(order);
      status = StragetyStatus::kSkip;
      if (waiting_reply_order_.empty()) {
        while (!outstanding_orders_.empty() && waiting_reply_order_.empty()) {
          DoHandleRtnOrder(outstanding_orders_.front());
          outstanding_orders_.pop_front();
        }
      }
    } else {
      status = StragetyStatus::kWaitReply;
    }
  }
  return status;
}
