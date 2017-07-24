#ifndef CTP_BIND_DEMO_MD_OBSERVER_H
#define CTP_BIND_DEMO_MD_OBSERVER_H
#include <boost/enable_shared_from_this.hpp>
#include "md.h"

namespace ctp_bind {
class MdObserver : public boost::enable_shared_from_this<MdObserver> {
  typedef std::function<void(const CThostFtdcDepthMarketDataField*)>
      MarketCallback;

 public:
  MdObserver(Md* md) : md_(md) {}
  void Subscribe(std::vector<std::string> instruments, MarketCallback cb) {
    md_->Subscribe(instruments, cb, shared_from_this());
  }

  void Unsubscribe(std::vector<std::string> instruments) {

  }

 private:
  Md* md_;
  std::set<std::string> instruments;
};
}  // namespace ctp_bind

#endif  // CTP_BIND_DEMO_MD_OBSERVER_H
