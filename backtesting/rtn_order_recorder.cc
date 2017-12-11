#include "rtn_order_recorder.h"

void RtnOrderToCSV::HandleOrder(const std::shared_ptr<OrderField>& order) {
  boost::posix_time::ptime pt(
      boost::gregorian::date(1970, 1, 1),
      boost::posix_time::milliseconds(order->update_timestamp));
  orders_csv_ << pt << "," << order->order_id << ","
              << (order->position_effect == PositionEffect::kOpen ? "O" : "C")
              << "," << (order->direction == OrderDirection::kBuy ? "B" : "S")
              << "," << static_cast<int>(order->status) << ","
              << order->input_price << "," << order->qty << "\n";
}

RtnOrderToCSV::RtnOrderToCSV(bft::ChannelDelegate* mail_box,
                             const std::string& out_dir,
                             const std::string& prefix_)
    : mail_box_(mail_box), orders_csv_(out_dir + prefix_ + "_orders.csv") {
  bft::MessageHandler handler;
  handler.Subscribe(&RtnOrderToCSV::HandleOrder, this);
  mail_box_->Subscribe(std::move(handler));
  boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
  facet->format("%Y-%m-%d %H:%M:%S");
  orders_csv_.imbue(std::locale(std::locale::classic(), facet));
}
