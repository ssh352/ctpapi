#ifndef BACKTESTING_PORTFOLIO_HANDLER_H
#define BACKTESTING_PORTFOLIO_HANDLER_H
#include <fstream>
#include "common/api_struct.h"
#include "portfolio.h"
#include "bft_core/channel_delegate.h"
#include "caf_common/caf_atom_defines.h"

class PortfolioHandler {
 public:
  PortfolioHandler(double init_cash,
                   bft::ChannelDelegate* mail_box,
                   std::string instrument,
                   const std::string& out_dir,
                   const std::string& csv_file_prefix,
                   double margin_rate,
                   int constract_multiple,
                   CostBasis cost_basis,
                   bool handle_input_signal);

  void BeforeTrading(BeforeTradingAtom, const TradingTime& trading_time);

  void HandleTick(const std::shared_ptr<TickData>& tick);

  void HandleOrder(const std::shared_ptr<OrderField>& order);

  void HandleDaySettleAtom(DaySettleAtom);

  void HandlerInputOrder(const InputOrder& input_order);

  void HandlerActionOrder(const OrderAction& action_order);

  void HandlerCancelOrder(const CancelOrder& action_order);

 private:
  Portfolio portfolio_;
  std::string instrument_;
  std::vector<OrderPosition> quantitys_;
  std::vector<std::shared_ptr<const OrderField>> histor_orders_;
  std::unordered_set<std::string> unique_order_ids_;
  std::shared_ptr<Tick> last_tick_;
  std::ofstream csv_;
  bft::ChannelDelegate* channel_delegate_;
};
#endif  // BACKTESTING_PORTFOLIO_HANDLER_H
