#include "doctest.h"

#include "../include/aitoolkit/behtree.hpp"

using namespace aitoolkit::bt;
struct blackboard_type {
  int node_count{0};
};

TEST_CASE("behtree task node evaluation") {
  SUBCASE("task node returns success") {
    blackboard_type blackboard;

    auto task_node = task<blackboard_type>::make([](auto& blackboard) {
      return execution_state::success;
    });

    auto state = task_node->evaluate(blackboard);

    CHECK(state == execution_state::success);
  }

  SUBCASE("task node returns failure") {
    blackboard_type blackboard;

    auto task_node = task<blackboard_type>::make([](auto& blackboard) {
      return execution_state::failure;
    });

    auto state = task_node->evaluate(blackboard);

    CHECK(state == execution_state::failure);
  }

  SUBCASE("task node returns running") {
    blackboard_type blackboard;

    auto task_node = task<blackboard_type>::make([](auto& blackboard) {
      return execution_state::running;
    });

    auto state = task_node->evaluate(blackboard);

    CHECK(state == execution_state::running);
  }
}

TEST_CASE("behtree check node evaluation") {
  SUBCASE("check node returns success") {
    blackboard_type blackboard;

    auto check_node = check<blackboard_type>::make([](auto& blackboard) {
      return true;
    });

    auto state = check_node->evaluate(blackboard);

    CHECK(state == execution_state::success);
  }

  SUBCASE("check node returns failure") {
    blackboard_type blackboard;

    auto check_node = check<blackboard_type>::make([](auto& blackboard) {
      return false;
    });

    auto state = check_node->evaluate(blackboard);

    CHECK(state == execution_state::failure);
  }
}

TEST_CASE("behtree neg node evaluation") {
  SUBCASE("neg node returns success") {
    blackboard_type blackboard;

    auto neg_node = neg<blackboard_type>::make(
      task<blackboard_type>::make([](auto& blackboard) {
        return execution_state::failure;
      })
    );

    auto state = neg_node->evaluate(blackboard);

    CHECK(state == execution_state::success);
  }

  SUBCASE("neg node returns failure") {
    blackboard_type blackboard;

    auto neg_node = neg<blackboard_type>::make(
      task<blackboard_type>::make([](auto& blackboard) {
        return execution_state::success;
      })
    );

    auto state = neg_node->evaluate(blackboard);

    CHECK(state == execution_state::failure);
  }

  SUBCASE("neg node returns running") {
    blackboard_type blackboard;

    auto neg_node = neg<blackboard_type>::make(
      task<blackboard_type>::make([](auto& blackboard) {
        return execution_state::running;
      })
    );

    auto state = neg_node->evaluate(blackboard);

    CHECK(state == execution_state::running);
  }
}

TEST_CASE("behtree seq node evaluation") {
  SUBCASE("seq node returns success") {
    blackboard_type blackboard;

    auto seq_node = seq<blackboard_type>::make({
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::success;
      }),
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::success;
      })
    });

    auto state = seq_node->evaluate(blackboard);

    CHECK(state == execution_state::success);
    CHECK(blackboard.node_count == 2);
  }

  SUBCASE("seq node returns failure") {
    blackboard_type blackboard;

    auto seq_node = seq<blackboard_type>::make({
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::success;
      }),
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::failure;
      })
    });

    auto state = seq_node->evaluate(blackboard);

    CHECK(state == execution_state::failure);
    CHECK(blackboard.node_count == 2);
  }

  SUBCASE("seq node returns failure early") {
    blackboard_type blackboard;

    auto seq_node = seq<blackboard_type>::make({
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::failure;
      }),
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::success;
      })
    });

    auto state = seq_node->evaluate(blackboard);

    CHECK(state == execution_state::failure);
    CHECK(blackboard.node_count == 1);
  }

  SUBCASE("seq node returns running") {
    blackboard_type blackboard;

    auto seq_node = seq<blackboard_type>::make({
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::success;
      }),
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::running;
      })
    });

    auto state = seq_node->evaluate(blackboard);

    CHECK(state == execution_state::running);
    CHECK(blackboard.node_count == 2);
  }

  SUBCASE("seq node returns running early") {
    blackboard_type blackboard;

    auto seq_node = seq<blackboard_type>::make({
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::running;
      }),
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::success;
      })
    });

    auto state = seq_node->evaluate(blackboard);

    CHECK(state == execution_state::running);
    CHECK(blackboard.node_count == 1);
  }
}

TEST_CASE("behtree sel node evaluation") {
  SUBCASE("sel node returns success when one child succeeds") {
    blackboard_type blackboard;

    auto sel_node = sel<blackboard_type>::make({
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::failure;
      }),
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::success;
      })
    });

    auto state = sel_node->evaluate(blackboard);

    CHECK(state == execution_state::success);
    CHECK(blackboard.node_count == 2);
  }

  SUBCASE("sel node returns success when one child succeeds early") {
    blackboard_type blackboard;

    auto sel_node = sel<blackboard_type>::make({
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::success;
      }),
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::failure;
      })
    });

    auto state = sel_node->evaluate(blackboard);

    CHECK(state == execution_state::success);
    CHECK(blackboard.node_count == 1);
  }

  SUBCASE("sel node return failure when all children fail") {
    blackboard_type blackboard;

    auto sel_node = sel<blackboard_type>::make({
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::failure;
      }),
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::failure;
      })
    });

    auto state = sel_node->evaluate(blackboard);

    CHECK(state == execution_state::failure);
    CHECK(blackboard.node_count == 2);
  }

  SUBCASE("sel node returns running when one child returns running") {
    blackboard_type blackboard;

    auto sel_node = sel<blackboard_type>::make({
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::failure;
      }),
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::running;
      })
    });

    auto state = sel_node->evaluate(blackboard);

    CHECK(state == execution_state::running);
    CHECK(blackboard.node_count == 2);
  }

  SUBCASE("sel node returns running when one child returns running early") {
    blackboard_type blackboard;

    auto sel_node = sel<blackboard_type>::make({
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::running;
      }),
      task<blackboard_type>::make([](auto& blackboard) {
        blackboard.node_count++;
        return execution_state::failure;
      })
    });

    auto state = sel_node->evaluate(blackboard);

    CHECK(state == execution_state::running);
    CHECK(blackboard.node_count == 1);
  }
}
