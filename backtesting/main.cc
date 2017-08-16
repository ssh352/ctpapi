#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include "caf/all.hpp"
#include "tick_series_data_base.h"

int fib(int x) {
  if (x == 0)
    return 0;

  if (x == 1)
    return 1;

  return fib(x - 1) + fib(x - 2);
}

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::shared_ptr<Tick>);

using DispatchTickAtom = caf::atom_constant<caf::atom("distick")>;
using SubscribeAtom = caf::atom_constant<caf::atom("sub")>;

class TickDataDispatcher : public caf::event_based_actor {
 public:
  TickDataDispatcher(caf::actor_config& cfg,
                     caf::actor strategy,
                     caf::actor order_book)
      : caf::event_based_actor(cfg),
        strategy_(strategy),
        order_book_(order_book) {
    TickSeriesDataBase ts_db("d:/ts_futures.h5");

    tick_containter_ = ts_db.ReadRange(
        "/dc/a_major",
        boost::posix_time::time_from_string("2016-12-01 09:00:00"),
        boost::posix_time::time_from_string("2017-07-31 15:00:00"));

    it_ = tick_containter_.begin();
  }

  virtual caf::behavior make_behavior() {
    // send(this, DispatchTickAtom::value);

    return {[=](DispatchTickAtom) {
              if (current_tick_index_ + 1 >= it_->second) {
                current_tick_index_ = 0;
                ++it_;
              }

              if (it_ == tick_containter_.end()) {
                quit();
                std::cout << "quit\n";
              } else {
                auto null_deleter = [](Tick*) { /*do nothing*/ };

                auto leave_response =
                    std::make_shared<size_t>(2 + tick_subscribers_.size());
                std::shared_ptr<Tick> tick(
                    &it_->first.get()[current_tick_index_], null_deleter);
                request(strategy_, caf::infinite, tick).then([=] {
                  if (--(*leave_response) == 0) {
                    send(this, DispatchTickAtom::value);
                  }
                });

                request(order_book_, caf::infinite, tick).then([=] {
                  if (--(*leave_response) == 0) {
                    send(this, DispatchTickAtom::value);
                  }
                });
                for (auto& actor : tick_subscribers_) {
                  request(actor, caf::infinite, tick).then([=] {
                    if (--(*leave_response) == 0) {
                      send(this, DispatchTickAtom::value);
                    }
                  });
                }
                ++current_tick_index_;
              }

            },
            [=](SubscribeAtom, caf::actor subscriber) {
              tick_subscribers_.push_back(subscriber);
            }};
  }

 private:
  caf::actor strategy_;
  caf::actor order_book_;
  std::vector<caf::actor> tick_subscribers_;
  std::vector<std::pair<std::unique_ptr<Tick[]>, int64_t> > tick_containter_;
  std::vector<std::pair<std::unique_ptr<Tick[]>, int64_t> >::const_iterator it_;
  int current_tick_index_ = 0;
};

caf::behavior DummpyStrategy(caf::event_based_actor* self) {
  auto count = std::make_shared<int>(0);
  return {[=](const std::shared_ptr<Tick>& tick) {
    // std::cout << (*count)++ << "\n";
  }};
}

caf::behavior DummpyOrderBook(caf::event_based_actor* self) {
  auto count = std::make_shared<int>(0);
  return {[=](const std::shared_ptr<Tick>& tick) {
    // std::cout << (*count)++ << "\n";
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }};
}

caf::behavior DummpySignal(caf::event_based_actor* self) {
  auto count = std::make_shared<int>(0);
  return {[=](const std::shared_ptr<Tick>& tick) {
    // std::cout << (*count)++ << "\n";
    // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    (*count) += fib(20);
    if ((*count) == 10000000) {
      std::cout << "Oops!\n";
    }
  }};
}

int caf_main(caf::actor_system& system) {
  using hrc = std::chrono::high_resolution_clock;

  auto beg = hrc::now();
  {
  auto dispatcher = system.spawn<TickDataDispatcher>(system.spawn(DummpyStrategy),
                                   system.spawn(DummpyOrderBook));

  for (int i = 0; i < 10;++i) {
    anon_send(dispatcher, SubscribeAtom::value, system.spawn(DummpySignal));
  }
  anon_send(dispatcher, DispatchTickAtom::value);
  }

  system.await_all_actors_done();

  std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(
                   hrc::now() - beg)
                   .count()
            << "\n";
  return 0;
}
CAF_MAIN()
