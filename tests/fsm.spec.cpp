#include "doctest.h"

#include "../include/aitoolkit/fsm.hpp"

using namespace aitoolkit::fsm;

struct blackboard_type {
  int enter{0};
  int exit{0};
  int pause{0};
  int resume{0};
  int update{0};
};

class state_dummy final : public state<blackboard_type> {
  public:
    state_dummy(int value) : m_val(value) {}

    virtual void enter(blackboard_type& blackboard) override {
      blackboard.enter = m_val;
    }

    virtual void exit(blackboard_type& blackboard) override {
      blackboard.exit = m_val;
    }

    virtual void pause(blackboard_type& blackboard) override {
      blackboard.pause = m_val;
    }

    virtual void resume(blackboard_type& blackboard) override {
      blackboard.resume = m_val;
    }

    virtual void update(blackboard_type& blackboard) override {
      blackboard.update = m_val;
    }

  private:
    int m_val;
};

TEST_CASE("fsm simple machine") {
  auto blackboard = blackboard_type{};
  auto fsm = simple_machine<blackboard_type>{};

  fsm.set_state(std::make_shared<state_dummy>(1), blackboard);
  CHECK(blackboard.enter == 1);

  fsm.pause(blackboard);
  CHECK(blackboard.pause == 1);

  fsm.resume(blackboard);
  CHECK(blackboard.resume == 1);

  fsm.update(blackboard);
  CHECK(blackboard.update == 1);

  fsm.set_state(std::make_shared<state_dummy>(2), blackboard);
  CHECK(blackboard.exit == 1);
  CHECK(blackboard.enter == 2);

  fsm.set_state(nullptr, blackboard);
  CHECK(blackboard.exit == 2);

  fsm.pause(blackboard);
  fsm.set_state(std::make_shared<state_dummy>(3), blackboard);
  CHECK(blackboard.enter == 3);
  CHECK(blackboard.pause == 3);
}

TEST_CASE("fsm stack machine") {
  auto blackboard = blackboard_type{};
  auto fsm = stack_machine<blackboard_type>{};

  fsm.push_state(std::make_shared<state_dummy>(1), blackboard);
  CHECK(blackboard.enter == 1);

  fsm.push_state(std::make_shared<state_dummy>(2), blackboard);
  CHECK(blackboard.enter == 2);
  CHECK(blackboard.pause == 1);

  fsm.update(blackboard);
  CHECK(blackboard.update == 2);

  fsm.pop_state(blackboard);
  CHECK(blackboard.exit == 2);
  CHECK(blackboard.resume == 1);

  fsm.pop_state(blackboard);
  CHECK(blackboard.exit == 1);
}
