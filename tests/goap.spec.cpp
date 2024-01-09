#include "doctest.h"

#include "../include/aitoolkit/goap.hpp"

using namespace aitoolkit::goap;
struct blackboard_type {
  bool have_storage;
  int wood;
  int food;
  int gold;
  int stone;
};

bool operator==(const blackboard_type& a, const blackboard_type& b) {
  return (
    a.have_storage == b.have_storage &&
    a.wood == b.wood &&
    a.food == b.food &&
    a.gold == b.gold &&
    a.stone == b.stone
  );
}

namespace std {
  template<>
  struct hash<blackboard_type> {
    size_t operator()(const blackboard_type& blackboard) const {
      return (
        std::hash<bool>{}(blackboard.have_storage) ^
        std::hash<int>{}(blackboard.wood) ^
        std::hash<int>{}(blackboard.food) ^
        std::hash<int>{}(blackboard.gold) ^
        std::hash<int>{}(blackboard.stone)
      );
    }
  };
}

class chop_wood final : public action<blackboard_type> {
  public:
    virtual float cost(const blackboard_type& blackboard) const override {
      return 1.0f;
    }

    virtual bool check_preconditions(const blackboard_type& blackboard) const override {
      return true;
    }

    virtual void apply_effects(blackboard_type& blackboard) const override {
      blackboard.wood += 1;
    }
};

class build_storage final : public action<blackboard_type> {
  public:
    virtual float cost(const blackboard_type& blackboard) const override {
      return 1.0f;
    }

    virtual bool check_preconditions(const blackboard_type& blackboard) const override {
      return (
        blackboard.wood >= 10 &&
        !blackboard.have_storage
      );
    }

    virtual void apply_effects(blackboard_type& blackboard) const override {
      blackboard.have_storage = true;
      blackboard.wood -= 10;
    }
};

class gather_food final : public action<blackboard_type> {
  public:
    virtual float cost(const blackboard_type& blackboard) const override {
      return 1.0f;
    }

    virtual bool check_preconditions(const blackboard_type& blackboard) const override {
      return blackboard.have_storage;
    }

    virtual void apply_effects(blackboard_type& blackboard) const override {
      blackboard.food += 1;
    }
};

class mine_gold final : public action<blackboard_type> {
  public:
    virtual float cost(const blackboard_type& blackboard) const override {
      return 1.0f;
    }

    virtual bool check_preconditions(const blackboard_type& blackboard) const override {
      return blackboard.have_storage;
    }

    virtual void apply_effects(blackboard_type& blackboard) const override {
      blackboard.gold += 1;
    }
};

class mine_stone final : public action<blackboard_type> {
  public:
    virtual float cost(const blackboard_type& blackboard) const override {
      return 1.0f;
    }

    virtual bool check_preconditions(const blackboard_type& blackboard) const override {
      return blackboard.have_storage;
    }

    virtual void apply_effects(blackboard_type& blackboard) const override {
      blackboard.stone += 1;
    }
};

TEST_CASE("goap planning") {
  SUBCASE("planner can generate a plan") {
    auto initial = blackboard_type{
      .have_storage = false,
      .wood = 0,
      .food = 0,
      .gold = 0,
      .stone = 0
    };

    auto goal = blackboard_type{
      .have_storage = true,
      .wood = 0,
      .food = 3,
      .gold = 2,
      .stone = 1
    };

    auto actions = std::vector<action_ptr<blackboard_type>>{
      std::make_shared<chop_wood>(),
      std::make_shared<build_storage>(),
      std::make_shared<gather_food>(),
      std::make_shared<mine_gold>(),
      std::make_shared<mine_stone>()
    };

    auto p = planner<blackboard_type>(actions, initial, goal);
    CHECK(p);

    // 10 chop wood, 1 build storage, 3 gather food, 2 mine gold, 1 mine stone
    CHECK(p.size() == 17);

    while (p) {
      p.run_next(initial);
    }
    CHECK(initial == goal);
  }

  SUBCASE("planner fails to find a plan") {
    auto initial = blackboard_type{
      .have_storage = false,
      .wood = 0,
      .food = 0,
      .gold = 0,
      .stone = 0
    };

    auto goal = blackboard_type{
      .have_storage = false,
      .wood = 0,
      .food = 3,
      .gold = 2,
      .stone = 1
    };

    auto actions = std::vector<action_ptr<blackboard_type>>{
      std::make_shared<chop_wood>(),
      std::make_shared<build_storage>(),
      std::make_shared<gather_food>(),
      std::make_shared<mine_gold>(),
      std::make_shared<mine_stone>()
    };

    auto p = planner<blackboard_type>(actions, initial, goal, 1000);
    CHECK(!p);
  }
}
