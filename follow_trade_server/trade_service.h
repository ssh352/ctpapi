#ifndef FOLLOW_TRADE_SERVER_TRADE_SERVICE_H
#define FOLLOW_TRADE_SERVER_TRADE_SERVICE_H

class TradeService {
 public:
  virtual void NormalOpen(const std::string& instrument,
                          const std::string& order_id,
                          OrderDirection direction,
                          int quantity) = 0;

  virtual void NormalClose(const std::string& instrument,
                           const std::string& order_id,
                           OrderDirection direction,
                           int quantity) = 0;

  virtual void Cancel(const std::string& order_id) = 0;

  virtual void StrategyOpen(const std::string& instrument,
                            const std::string& order_id,
                            OrderDirection direction,
                            int quantity) = 0;

  virtual void StrategyClose(const std::string& instrument,
                             const std::string& order_id,
                             OrderDirection direction,
                             int quantity) = 0;
};

#endif  // FOLLOW_TRADE_SERVER_TRADE_SERVICE_H
