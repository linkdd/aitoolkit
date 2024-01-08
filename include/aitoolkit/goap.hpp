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

    virtual void apply_effects(blackboard_type& blackboard) const override {
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

    virtual void apply_effects(blackboard_type& blackboard) const override {
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

    virtual void apply_effects(blackboard_type& blackboard) const override {
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
      return blackboard.has_pickaxe;
    }

    virtual void apply_effects(blackboard_type& blackboard) const override {
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

auto p = plan<blackboard_type>(actions, initial, goal);
p.run(initial); // will mutate the blackboard
```
*/

#include <unordered_set>
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

  template <blackboard_trait T>
  class action {
    public:
      virtual ~action() = default;

      virtual float cost(const T& blackboard) const = 0;
      virtual bool check_preconditions(const T& blackboard) const = 0;
      virtual void apply_effects(T& blackboard) const = 0;
  };

  template <blackboard_trait T>
  using action_ptr = std::shared_ptr<action<T>>;

  template <blackboard_trait T>
  class plan {
    public:
      plan(
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
            while (current_node->parent != nullptr) {
              m_actions.push(current_node->action_taken);
              current_node = current_node->parent;
            }

            break;
          }

          if (!closed_set.contains(current_node->blackboard)) {
            closed_set.insert(current_node->blackboard);

            for (auto& action : actions) {
              if (action->check_preconditions(current_node->blackboard)) {
                auto next_blackboard = current_node->blackboard;
                action->apply_effects(next_blackboard);
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
      }

      size_t size() const {
        return m_actions.size();
      }

      operator bool() const {
        return !m_actions.empty();
      }

      void run(T& blackboard) {
        auto actions = m_actions;

        while (!actions.empty()) {
          auto action = actions.front();
          actions.pop();

          action->apply_effects(blackboard);
        }
      }

    private:
      std::queue<action_ptr<T>> m_actions;
  };
}
