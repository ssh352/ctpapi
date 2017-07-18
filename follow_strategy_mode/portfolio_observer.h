#ifndef FOLLOW_STRATEGY_MODE_PORTFOLIO_OBSERVER_H
#define FOLLOW_STRATEGY_MODE_PORTFOLIO_OBSERVER_H

class PortfolioObserver {
public:
  virtual void Notify(std::vector<AccountPortfolio> portfolio) = 0;
};

#endif // FOLLOW_STRATEGY_MODE_PORTFOLIO_OBSERVER_H



