#pragma once

/**
@defgroup goap Goal Oriented Action Planning

## Introduction

Goal Oriented Action Planning (GOAP) is a planning algorithm that can be used
to find a sequence of actions that will lead to a goal state. The algorithm
works by searching through a graph of possible actions and their effects. The
algorithm is guaranteed to find a solution if one exists, but it is not
guaranteed to find the optimal solution.

<center><pre class="mermaid">
graph LR

  start[Start] --> action1
  action1[Get axe] --> action2
  action2[Chop tree] --> goal
  goal[Goal]

  action3[Get pickaxe] --> action4
  action3 --> action5

  action4[Mine gold]
  action5[Mine stone]

  style start fill:darkred
  style goal fill:darkgreen
</pre></center>


## Usage

First, include the header file:

```cpp
#include <aitoolkit/goap.hpp>
```

Then, create a blackboard class that will hold the state of the planner:

```cpp
struct blackboard_type {
  bool has_axe{false};
  bool has_pickaxe{false};
  int wood{0};
  int gold{0};
  int stone{0};
};
```

> **NB:** The blackboard needs to be comparable (`a == b`) and hashable.

Next, create a class for each action that you want to be able to perform:

```cpp
using namespace aitoolkit::goap;

class get_axe final : public action<blackboard_type> {
  public:
    virtual float cost(const blackboard_type& blackboard) const override {
      return 1.0f;
    }

    virtual bool check_preconditions(const blackboard_type& blackboard) const override {
      return !blackboard.has_axe;
    }

    virtual void apply_effects(blackboard_type& blackboard, bool dry_run) const override {
      blackboard.has_axe = true;
    }
};

class get_pickaxe final : public action<blackboard_type> {
  public:
    virtual float cost(const blackboard_type& blackboard) const override {
      return 1.0f;
    }

    virtual bool check_preconditions(const blackboard_type& blackboard) const override {
      return !blackboard.has_pickaxe;
    }

    virtual void apply_effects(blackboard_type& blackboard, bool dry_run) const override {
      blackboard.has_pickaxe = true;
    }
};

class chop_tree final : public action<blackboard_type> {
  public:
    virtual float cost(const blackboard_type& blackboard) const override {
      return 1.0f;
    }

    virtual bool check_preconditions(const blackboard_type& blackboard) const override {
      return blackboard.has_axe;
    }

    virtual void apply_effects(blackboard_type& blackboard, bool dry_run) const override {
      blackboard.wood += 1;
    }
};

class mine_gold final : public action<blackboard_type> {
  public:
    virtual float cost(const blackboard_type& blackboard) const override {
      return 1.0f;
    }

    virtual bool check_preconditions(const blackboard_type& blackboard) const override {
      return blackboard.has_pickaxe;
    }

    virtual void apply_effects(blackboard_type& blackboard, bool dry_run) const override {
      blackboard.gold += 1;
    }
};

class mine_stone final : public action<blackboard_type> {
  public:
    virtual float cost(const blackboard_type& blackboard) const override {
      return 1.0f;
    }

    virtual bool check_preconditions(const blackboard_type& blackboard) const override {
      return blackboard.has_pickaxe;
    }

    virtual void apply_effects(blackboard_type& blackboard, bool dry_run) const override {
      blackboard.stone += 1;
    }
};
```

Finally, create a plan and run it:

```cpp
auto actions = std::vector<action_ptr<blackboard_type>>{
  std::make_shared<get_axe>(),
  std::make_shared<get_pickaxe>(),
  std::make_shared<chop_tree>(),
  std::make_shared<mine_gold>(),
  std::make_shared<mine_stone>()
};
auto initial = blackboard_type{};
auto goal = blackboard_type{
  .has_axe = true,
  .has_pickaxe = true,
  .wood = 1,
  .gold = 1,
  .stone = 1
};

auto p = planner<blackboard_type>(actions, initial, goal);

auto blackboard = initial;
while (p) {
  p.run_next(blackboard); // will mutate the blackboard
}
```
*/

#include <unordered_set>
#include <functional>
#include <coroutine>
#include <memory>
#include <vector>
#include <queue>

#include <type_traits>
#include <concepts>

namespace aitoolkit::goap {
  template <typename T>
  concept blackboard_trait = requires(const T &a, const T &b) {
    { a == b } -> std::convertible_to<bool>;
    { std::hash<T>{}(a) } -> std::convertible_to<size_t>;
  };

  /**
   * @ingroup goap
   * @class action
   * @brief An action that can be performed on a blackboard.
   *
   * An action hae a cost, which is used during planning, if 2 actions have a
   * similar effect, the one with the lowest cost will be chosen.
   *
   * An action also has preconditions, which are used to determine if an action
   * can be performed on a blackboard. If the preconditions are not met, the
   * action will not be considered during planning.
   *
   * Finally, an action has effects, which are applied to the blackboard when
   * the action is performed.
   */
  template <blackboard_trait T>
  class action {
    public:
      virtual ~action() = default;

      /**
       * @brief The cost of performing this action.
       */
      virtual float cost(const T& blackboard) const = 0;

      /**
       * @brief Check if the preconditions for this action are met.
       */
      virtual bool check_preconditions(const T& blackboard) const = 0;

      /**
       * @brief Apply the effects of this action to the blackboard.
       */
      virtual void apply_effects(T& blackboard, bool dry_run) const = 0;
  };

  /**
   * @brief Heeap allocated pointer to an action.
  */
  template <blackboard_trait T>
  using action_ptr = std::shared_ptr<action<T>>;

  /**
   * @ingroup goap
   * @class plan
   * @brief A plan is a sequence of actions that will lead to a goal state.
   */
  template <blackboard_trait T>
  class plan {
    public:
      plan(std::queue<action_ptr<T>> actions) : m_actions(actions) {}

      /**
       * @brief Get the number of actions in the plan.
       */
      size_t size() const {
        return m_actions.size();
      }

      /**
       * @brief Check if the plan is empty.
       */
      operator bool() const {
        return !m_actions.empty();
      }

      /**
       * @brief Execute the next planned action.
       */
      void run_next(T& blackboard) {
        if (!m_actions.empty()) {
          auto action = m_actions.front();
          m_actions.pop();
          action->apply_effects(blackboard, false);
        }
      }

    private:
      std::queue<action_ptr<T>> m_actions;
  };

  /**
   * @ingroup goap
   * @brief Create a plan.
   *
   * The plan is created by providing a list of actions, an initial blackboard
   * state, and a goal blackboard state. The plan will then find a sequence of
   * actions that will lead to the goal state.
   *
   * @param actions The list of actions that can be performed.
   * @param initital_blackboard The initial state of the blackboard.
   * @param goal_blackboard The goal state of the blackboard.
   * @param max_iterations The maximum number of iterations to perform
   * before giving up. If 0, the plan will run until it finds a solution.
   * @return A plan that can be executed.
   */
  template <blackboard_trait T>
  plan<T> planner(
    std::vector<action_ptr<T>> actions,
    T initital_blackboard,
    T goal_blackboard,
    size_t max_iterations = 0
  ) {
    struct node_type {
      T blackboard;
      float cost;

      action_ptr<T> action_taken;
      std::shared_ptr<node_type> parent;
    };

    using node_ptr = std::shared_ptr<node_type>;

    struct node_compare {
      bool operator()(const node_ptr& a, const node_ptr& b) const {
        return a->cost > b->cost;
      }
    };

    std::priority_queue<node_ptr, std::vector<node_ptr>, node_compare> open_set;
    std::unordered_set<T> closed_set;

    open_set.push(std::make_shared<node_type>(node_type{
      .blackboard = initital_blackboard,
      .cost = 0.0f,
      .action_taken = nullptr,
      .parent = nullptr
    }));

    for (
      size_t iteration = 0;
      !open_set.empty() && (max_iterations == 0 || iteration < max_iterations);
      ++iteration
    ) {
      auto current_node = open_set.top();
      open_set.pop();

      if (current_node->blackboard == goal_blackboard) {
        auto actions = std::queue<action_ptr<T>>{};

        while (current_node->parent != nullptr) {
          actions.push(current_node->action_taken);
          current_node = current_node->parent;
        }

        return plan<T>(actions);
      }

      if (!closed_set.contains(current_node->blackboard)) {
        closed_set.insert(current_node->blackboard);

        for (auto& action : actions) {
          if (action->check_preconditions(current_node->blackboard)) {
            auto next_blackboard = current_node->blackboard;
            action->apply_effects(next_blackboard, true);
            auto next_cost = current_node->cost + action->cost(current_node->blackboard);

            if (!closed_set.contains(next_blackboard)) {
              open_set.push(std::make_shared<node_type>(node_type{
                .blackboard = next_blackboard,
                .cost = next_cost,
                .action_taken = action,
                .parent = current_node
              }));
            }
          }
        }
      }
    }

    return plan<T>(std::queue<action_ptr<T>>{});
  }
}
