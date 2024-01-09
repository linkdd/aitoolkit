#pragma once

/**
@defgroup utility Utility AI

## Introduction

Utility AI is a planning algorithm that can be used to find the best action to
perform in a given situation. The algorithm works by assigning a score to each
action based on how well it will achieve the goal. The algorithm is guaranteed
to find a solution.

<center><pre class="mermaid">
flowchart TD

  a1[Collect food\nscore: +50]
  a2[Collect wood\nscore: +150]
  a3[Collect stone\nscore: -10]
  a4[Collect gold\nscore: +75]

  style a1 color:darkred
  style a2 color:darkgreen
  style a3 color:darkred
  style a4 color:darkred
</pre></center>


## Usage

First, include the header file:

```cpp
#include <aitoolkit/utility.hpp>
```

Then, create a blackboard type:

```cpp
struct blackboard_type {
  int food{0};
  int wood{0};
  int stone{0};
  int gold{0};
};
```

Next, create a class for each action that you want to be able to perform:

```cpp
using namespace aitoolkit::utility;

class collect_food final : public action<blackboard_type> {
  public:
    virtual float score(const blackboard_type& blackboard) const override {
      return 50.0f;
    }

    virtual void apply(blackboard_type& blackboard) const override {
      blackboard.food += 1;
    }
};

class collect_wood final : public action<blackboard_type> {
  public:
    virtual float score(const blackboard_type& blackboard) const override {
      return 150.0f;
    }

    virtual void apply(blackboard_type& blackboard) const override {
      blackboard.wood += 1;
    }
};

class collect_stone final : public action<blackboard_type> {
  public:
    virtual float score(const blackboard_type& blackboard) const override {
      return -10.0f;
    }

    virtual void apply(blackboard_type& blackboard) const override {
      blackboard.stone += 1;
    }
};

class collect_gold final : public action<blackboard_type> {
  public:
    virtual float score(const blackboard_type& blackboard) const override {
      return 75.0f;
    }

    virtual void apply(blackboard_type& blackboard) const override {
      blackboard.gold += 1;
    }
};
```

Finally, create an evaluator and run it:

```cpp
auto evaluator = evaluator<blackboard_type>{
  std::make_shared<collect_food>(),
  std::make_shared<collect_wood>(),
  std::make_shared<collect_stone>(),
  std::make_shared<collect_gold>()
};

auto blackboard = blackboard_type{};
evaluator.run(blackboard);
```
*/

#include <initializer_list>
#include <memory>
#include <vector>
#include <limits>

namespace aitoolkit::utility {
  /**
   * @ingroup utility
   * @class action
   * @brief Base abstract class for all actions
   */
  template <typename T>
  class action {
    public:
      virtual ~action() = default;

      /**
       * @brief Return the score of the action
       */
      virtual float score(const T& blackboard) const = 0;

      /**
       * @brief Apply the action to the blackboard
       */
      virtual void apply(T& blackboard) const = 0;
  };

  /**
   * @ingroup utility
   * @brief Heap allocated pointer to an action.
   */
  template <typename T>
  using action_ptr = std::shared_ptr<action<T>>;

  /**
   * @ingroup utility
   * @class evaluator
   * @brief Evaluate a set of actions and apply the best one.
   */
  template <typename T>
  class evaluator {
    public:
      /**
       * @brief Construct an evaluator from a list of actions
       */
      evaluator(std::initializer_list<action_ptr<T>> actions) {
        m_actions.reserve(actions.size());
        for (auto action : actions) {
          m_actions.push_back(action);
        }
      }

      /**
       * @brief Find the best action and apply it to the blackboard
       */
      void run(T& blackboard) const {
        if (m_actions.empty()) {
          return;
        }

        auto best_score = std::numeric_limits<float>::min();
        auto best_action = m_actions.front();

        for (auto& action : m_actions) {
          auto score = action->score(blackboard);
          if (score > best_score) {
            best_score = score;
            best_action = action;
          }
        }

        best_action->apply(blackboard);
      }

    private:
      std::vector<action_ptr<T>> m_actions;
  };
}
