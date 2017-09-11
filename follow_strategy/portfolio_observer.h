#ifndef follow_strategy_PORTFOLIO_OBSERVER_H
#define follow_strategy_PORTFOLIO_OBSERVER_H

class PortfolioObserver {
 public:
  virtual void Notify(std::vector<AccountPortfolio> portfolio) = 0;
};

#endif  // follow_strategy_PORTFOLIO_OBSERVER_H
