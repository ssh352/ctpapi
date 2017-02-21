#ifndef FOLLOW_STRATEGY_H
#define FOLLOW_STRATEGY_H
#include "geek_quant/caf_defines.h"

class FollowStrategy : public CtpObserver::base {
{
public:
  FollowStrategy(caf::actor_config& cfg) : CtpObserver::base(cfg) {

  }
  virtual ~FollowStrategy() { };

  virtual CtpObserver::behavior_type make_behavior() override {
    return {
      [=](RtnOrderAtom, std::shared_ptr<CThostFtdcOrderField> order) {
      },
      [=](AddListener) {

      }
    }
  }

private:
  std::map<std::string, std::shared_ptr<CThostFtdcOrderField> > enqueue_orders_;
  std::vector<strong_actor_ptr> listeners_;
};

#endif /* FOLLOW_STRATEGY_H */
