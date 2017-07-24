#ifndef CTP_BIND_DEMO_MD_OBSERVER_H
#define CTP_BIND_DEMO_MD_OBSERVER_H
#include <boost/enable_shared_from_this.hpp>
#include <boost/make_shared.hpp>
#include "md.h"

namespace ctp_bind {
class MdObserver : public boost::enable_shared_from_this<MdObserver> {
  typedef std::function<void(const CThostFtdcDepthMarketDataField*)>
      MarketCallback;
 public:
  static boost::shared_ptr<MdObserver> Create(Md* md) {
    return boost::shared_ptr<MdObserver>(new MdObserver(md),
                                         [](MdObserver* observer) {
                                           observer->MdUnscribeAllInstruments();
                                           delete observer;
                                         });
  }
  void Subscribe(std::vector<std::string> instruments, MarketCallback cb) {
    std::vector<std::pair<std::string, ctp_bind::MdSingnal::slot_type> >
        instrument_slots;
    for (auto instrument : instruments) {
      auto instrument_tracker = boost::make_shared<std::string>(instrument);
      instrument_trackers_.push_back(instrument_tracker);
      instrument_slots.push_back(
          {std::move(instrument),
           MdSingnal::slot_type(cb).track(instrument_tracker)});
    }
    md_->Subscribe(instrument_slots);
  }

  void Unsubscribe(std::vector<std::string> instruments) {
    for (auto& instrument : instruments) {
      auto it = std::find_if(instrument_trackers_.begin(), instrument_trackers_.end(), [=](auto i){
        return *i == instrument;
      });
      if (it != instrument_trackers_.end()) {
        instrument_trackers_.erase(it);
      }
      /*
      instrument_trackers_.erase(
          boost::shared_ptr<std::string>(&instrument, [](auto p) {}));
      */
    }
  }

 private:
  MdObserver(Md* md) : md_(md) {}
  ~MdObserver() {}
  void MdUnscribeAllInstruments() {}
  Md* md_;
  std::vector<boost::shared_ptr<std::string> > instrument_trackers_;
};
}  // namespace ctp_bind

#endif  // CTP_BIND_DEMO_MD_OBSERVER_H
