#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "caf/all.hpp"

using namespace std;
using namespace std::chrono;
using namespace caf;

using start_atom = atom_constant<atom("start")>;
using hello_atom = atom_constant<atom("hello")>;
using done_atom = atom_constant<atom("done")>;
using MarketDataAtom = atom_constant<atom("market")>;
using MarketDataReplyAtom = atom_constant<atom("marketrep")>;
using KDJSignalAtom = atom_constant<atom("kdj")>;

const int fib_param = 20;

struct MarketData {
  double bid_price1;
  int bid_vol1;
  double ask_price1;
  int ask_vol1;
  double last_price;
};

struct config {
  std::size_t m_cooperations = 1024;
  std::size_t m_agents = 512;
  std::size_t m_messages = 100;
  std::size_t m_demands_at_once = 0;
  std::size_t m_threads = 0;
  std::size_t m_messages_to_send_at_start = 1;
};

int fib(int x) {
  if (x == 0)
    return 0;

  if (x == 1)
    return 1;

  return fib(x - 1) + fib(x - 2);
}

class test : public event_based_actor {
 public:
  test(actor_config& cfg, std::size_t messages_to_send)
      : event_based_actor(cfg), m_messages_to_send(messages_to_send) {}

  virtual ~test() {}

  behavior make_behavior() override {
    return {[=](hello_atom) {
      promise_ = make_response_promise<std::vector<int>>();
      promise_.deliver(std::vector<int>());
      ++m_messages_received;
      if (m_messages_received >= m_messages_to_send) {
        quit();
      }
      return fib(m_messages_received % fib_param);
    }};
  }

 private:
  const std::size_t m_messages_to_send;
  std::size_t m_messages_received = 0;
  caf::response_promise promise_;
};

class controller : public event_based_actor {
 public:
  controller(actor_config& cfg, const std::size_t working_agents)
      : event_based_actor(cfg), m_working_agents(working_agents) {}

  virtual ~controller() {}

  behavior make_behavior() override {
    return {[=](done_atom) {
      --m_working_agents;
      if (!m_working_agents) {
        std::cout << "quit\n";
        quit();
      }
    }};
  }

 private:
  std::size_t m_working_agents;
};

class requester : public event_based_actor {
 public:
  requester(actor_config& cfg,
            actor controller,
            actor test,
            size_t message_to_send)
      : event_based_actor(cfg),
        controller_(controller),
        test_(test),
        m_messages_to_send(message_to_send) {}
  virtual behavior make_behavior() override {
    return {[=](start_atom) { do_request(); }};
  }

  void do_request() {
    request(test_, infinite, make_message(hello_atom::value)).then([=](int) {
      if (++message_response_ >= m_messages_to_send) {
        send(controller_, make_message(done_atom::value));
        quit();
      } else {
        do_request();
      }
    });
    // std::cout << "evt_start\n";
  }

 private:
  actor test_;
  actor controller_;
  const std::size_t m_messages_to_send;
  std::size_t message_response_ = 0;
};

class MACDSignal : public event_based_actor {
 public:
  virtual behavior make_behavior() override {
    return {[=](MarketDataAtom, MarketData data) {}};
  }

  std::vector<strong_actor_ptr> subscribers_;
};

class KDJSignal : public event_based_actor {
 public:
  virtual behavior make_behavior() override {
    return {[=](MarketDataAtom, MarketData data) {
      auto rp = make_response_promise();
      std::shared_ptr<size_t> subscriber_num =
          std::make_shared<size_t>(subscribers_.size());

      for (auto& actor_ptr : subscribers_) {
        auto actor = caf::actor_cast<caf::actor>(actor_ptr);
        request(actor, caf::infinite, KDJSignalAtom::value).then([=]() mutable {
          if (--*subscriber_num <= 0) {
            rp.deliver(MarketDataReplyAtom::value);
          }
        });
      }
      return rp;
    }};
  }

 private:
  std::vector<strong_actor_ptr> subscribers_;
};

void run(actor_system& system, const config& cfg) {
  auto ctrl = system.spawn<controller>(cfg.m_cooperations * cfg.m_agents);
  for (std::size_t i = 0; i != cfg.m_cooperations; ++i) {
    for (std::size_t a = 0; a != cfg.m_agents; ++a) {
      auto r = system.spawn<requester>(ctrl, system.spawn<test>(cfg.m_messages),
                                       cfg.m_messages);
      anon_send(r, make_message(start_atom::value));
    }
  }
}

bool mandatory_missing(std::set<std::string> opts,
                       std::initializer_list<std::string> xs) {
  auto not_in_opts = [&](const std::string& x) {
    if (opts.count(x) == 0) {
      cerr << "mandatory argument missing: " << x << endl;
      return true;
    }
    return false;
  };
  return std::any_of(xs.begin(), xs.end(), not_in_opts);
}

bool setup(int argc, char** argv, config& cfg) {
  if (!cfg.m_demands_at_once)
    cfg.m_demands_at_once = std::numeric_limits<size_t>::max();

  if (!cfg.m_threads)
    cfg.m_threads = std::thread::hardware_concurrency();

  bool show_cmd = false;

  auto res = message_builder{argv + 1, argv + argc}.extract_opts(
      {{"cooperations,c", "count of groups", cfg.m_cooperations},
       {"agents,a", "count of agents in group", cfg.m_agents},
       {"messages,m", "count of messages for every agent", cfg.m_messages},
       {"demands-at-once,d", "max throughput", cfg.m_demands_at_once},
       {"messages-at-start,S", "count of messages to be sent at start",
        cfg.m_messages_to_send_at_start},
       {"threads,t", "number of threads for the scheduler", cfg.m_threads}});

  if (!res.error.empty() || res.opts.count("help") > 0) {
    cout << res.error << endl << res.helptext << endl;
    return false;
  }

  // set_scheduler( cfg.m_threads, cfg.m_demands_at_once );
  return true;
}

inline std::size_t total_messages(const config& cfg) {
  const auto total_agents = cfg.m_agents * cfg.m_cooperations;
  return total_agents + total_agents + total_agents * cfg.m_messages;
}

void show_cfg(const config& cfg) {
  std::cout << "coops: " << cfg.m_cooperations
            << ", agents in coop: " << cfg.m_agents
            << ", msg per agent: " << cfg.m_messages
            << " (at start: " << cfg.m_messages_to_send_at_start << ")"
            << ", total msgs: " << total_messages(cfg) << std::endl;

  std::cout << "\n*** demands_at_once: ";
  std::cout << cfg.m_demands_at_once;

  std::cout << "\n*** threads in pool: ";
  if (cfg.m_threads)
    std::cout << cfg.m_threads;

  std::cout << std::endl;
}

int main(int argc, char** argv) {
  config cfg;

  if (!setup(argc, argv, cfg)) {
    return 1;
  }

  actor_system_config caf_cfg;
  actor_system system(caf_cfg);

  cfg.m_cooperations = 1;
  cfg.m_agents = 8;
  cfg.m_messages = 1000000;

  show_cfg(cfg);

  {
    size_t count = 0;
    const auto start_time = steady_clock::now();
    size_t tm = cfg.m_agents * cfg.m_messages;
    for (size_t j = 0; j < cfg.m_agents; ++j) {
      for (size_t i = 0; i < cfg.m_messages; ++i) {
        count += fib(i % fib_param);
      }
    }
    const auto total_msec =
        duration_cast<milliseconds>(steady_clock::now() - start_time).count();
    std::cout << "total time msec: " << total_msec << std::endl;
    std::cout << count << "\n";
  }
  {
    const auto start_time = steady_clock::now();

    try {
      run(system, cfg);
    } catch (std::exception& ex) {
      std::cout << "error: " << ex.what() << std::endl;
    }

    system.await_all_actors_done();
    const auto total_msec =
        duration_cast<milliseconds>(steady_clock::now() - start_time).count();
    std::cout << "total time msec: " << total_msec << std::endl;
  }
}
