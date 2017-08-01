#include <boost/lexical_cast.hpp>
#include <iostream>
#include "caf/all.hpp"

using SubscribeRtnOrderAtom = caf::atom_constant<caf::atom("subro")>;

caf::behavior foo(caf::event_based_actor* self) {
  return {[](const std::string& str) { std::cout << "func: " << str << "\n"; }};
}

class Bar : public caf::event_based_actor {
 public:
  Bar(caf::actor_config& cfg) : caf::event_based_actor(cfg) {
    // grp_ = system().groups().anonymous();

    std::string module = "local";
    std::string id = "foo";
    auto expected_grp = system().groups().get(module, id);
    if (!expected_grp) {
      std::cerr << "*** cannot load group: "
                << system().render(expected_grp.error()) << std::endl;
      return;
    }
    auto grp = std::move(*expected_grp);
    join(grp);
  }

  caf::behavior make_behavior() override {
    set_exit_handler([=](caf::scheduled_actor* self, const caf::exit_msg& em) {
      grp_ = nullptr;
      caf::aout(this) << "errr\n";
    });
    return {[=](bool) { std::cout << "abc\n"; },
            [=](const std::string& str) {
              send(grp_, str);
              quit();
            },
            [=](SubscribeRtnOrderAtom) { return grp_; },
            [=](const caf::exit_msg& msg) { caf::aout(this) << "errr\n"; }};
  }

 private:
  caf::group grp_;
};

class Foo : public caf::event_based_actor {
 public:
  Foo(caf::actor_config& cfg) : caf::event_based_actor(cfg) {
    // grp_ = system().groups().anonymous();

    std::string module = "local";
    std::string id = "foo";
    auto expected_grp = system().groups().get(module, id);
    if (!expected_grp) {
      std::cerr << "*** cannot load group: "
                << system().render(expected_grp.error()) << std::endl;
      return;
    }
    auto grp = std::move(*expected_grp);
    join(grp);
  }

  caf::behavior make_behavior() override {
    set_exit_handler([=](caf::scheduled_actor* self, const caf::exit_msg& em) {
      grp_ = nullptr;
      caf::aout(this) << "errr\n";
    });
    return {[=](bool) { std::cout << "abc\n"; },
            [=](const std::string& str) {
              send(grp_, str);
              quit();
            },
            [=](SubscribeRtnOrderAtom) { return grp_; },
            [=](const caf::exit_msg& msg) { caf::aout(this) << "errr\n"; }};
  }

 private:
  caf::group grp_;
};

int caf_main(caf::actor_system& system, const caf::actor_system_config& cfg) {
  auto bar = system.spawn<Bar>();
  auto foo = system.spawn<Foo>();

  std::string module = "local";
  std::string id = "foo";
  auto expected_grp = system.groups().get(module, id);
  if (!expected_grp) {
    std::cerr << "*** cannot load group: "
              << system.render(expected_grp.error()) << std::endl;
    return 1;
  }
  auto grp = std::move(*expected_grp);

  caf::scoped_actor self(system);
  self->send(grp, false);
  caf::anon_send_exit(bar, caf::exit_reason::user_shutdown);

  system.await_all_actors_done();

  //  self->receive([](const std::string& str) { std::cout << str << "\n"; });

  return 0;
}

CAF_MAIN()
