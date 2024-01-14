#pragma once

/**
@defgroup behtree Behavior Tree

## Introduction

A behavior tree is a tree structure where each node represents a behavior. The
tree is evaluated from the root node to the leaf nodes. The leaf nodes are
either tasks or checks. A task is a node that performs an action and returns
either success or failure. A check is a node that returns either success or
failure based on some condition.

<center><pre class="mermaid">
flowchart TD

  root{{Selector}} --> seq1
  root --> seq2
  root --> seq3

  seq1{{Sequence}} --> seq1a1
  seq1 --> seq1a2

  seq2{{Sequence}} --> seq2a1
  seq2 --> seq2a2

  seq3{{Sequence}} --> seq3a1
  seq3 --> seq3a2

  seq1a1[Check enemy in attack range]
  seq1a2[[Destroy enemy]]

  seq2a1[Check enemy in sight]
  seq2a2[[Move towards enemy]]

  seq3a1[[Move towards waypoint]]
  seq3a2[[Select next waypoint]]

  style root fill:darkred
  style seq1 fill:darkgreen
  style seq2 fill:darkgreen
  style seq3 fill:darkgreen
</pre></center>


## Usage

First, include the header file:

```cpp
#include <aitoolkit/behtree.hpp>
```

Then, create a blackboard class that will hold the state of the tree:

```cpp
struct blackboard_type {
  glm::vec2 agent_position;
  glm::vec2 enemy_position;

  float attack_range;
  float sight_range;

  size_t current_waypoint;
  std::vector<glm::vec2> waypoints;
};
```

Next, create the tree:

```cpp
using namespace aitoolkit::bt;

auto tree = sel<blackboard_type>(
  node_list<blackboard_type>(
    seq<blackboard_type>(
      node_list<blackboard_type>(
        check<blackboard_type>([](const blackboard_type& bb) {
          auto distance = glm::distance(bb.agent_position, bb.enemy_position);
          return distance <= bb.attack_range;
        }),
        task<blackboard_type>([](blackboard_type& bb) {
          // Destroy enemy
          return execution_state::success;
        })
      )
    ),
    seq<blackboard_type>(
      node_list<blackboard_type>(
        check<blackboard_type>([](const blackboard_type& bb) {
          auto distance = glm::distance(bb.agent_position, bb.enemy_position);
          return distance <= bb.sight_range;
        }),
        task<blackboard_type>([](blackboard_type& bb) {
          // Move towards enemy
          return execution_state::success;
        })
      )
    ),
    seq<blackboard_type>::make({
      task<blackboard_type>([](blackboard_type& bb) {
        // Move towards waypoint
        return execution_state::success;
      }),
      task<blackboard_type>([](blackboard_type& bb) {
        // Select next waypoint
        return execution_state::success;
      })
    })
  )
);
```

Finally, evaluate the tree:

```cpp
auto bb = blackboard_type{
  .agent_position = { 0.0f, 0.0f },
  .enemy_position = { 1.0f, 1.0f },
  .attack_range = 0.5f,
  .sight_range = 1.0f
};

while (true) {
  auto state = tree.evaluate(bb);
  if (state == execution_state::success) {
    break;
  }
}
```
*/

#include <functional>
#include <memory>
#include <vector>

#include <type_traits>
#include <concepts>

namespace aitoolkit::bt {
  /**
   * @ingroup behtree
   * @enum execution_state
   * @brief Represent the state of a node
   */
  enum class execution_state {
    success, /**< The node was successful */
    failure, /**< The node failed */
    running  /**< The node is still running */
  };

  /**
   * @ingroup behtree
   * @class node
   * @brief Base abstract class for all nodes
   */
  template <class T>
  class node {
    public:
      node() = default;
      node(const node&) = delete;
      node(node&& other) {
        m_children = std::move(other.m_children);
      }

      virtual ~node() = default;

      virtual execution_state evaluate(T& blackboard) const = 0;

    protected:
      std::vector<std::unique_ptr<node<T>>> m_children;
  };

  /**
   * @ingroup behtree
   * @brief Heap-allocated pointer to node
   */
  template <class T>
  using node_ptr = std::unique_ptr<node<T>>;

  template <typename N, class T>
  concept node_trait = std::derived_from<N, node<T>>;

  /**
   * @ingroup behtree
   * @brief Helper function to create a list of nodes
   */
  template <typename T, node_trait<T> ...Children>
  std::vector<node_ptr<T>> node_list(Children&&... children) {
    auto nodes = std::vector<node_ptr<T>>{};
    nodes.reserve(sizeof...(children));
    (nodes.push_back(std::make_unique<Children>(std::move(children))), ...);
    return nodes;
  }

  /**
   * @ingroup behtree
   * @class seq
   * @brief Sequence node, will execute all children in order until one fails
   */
  template <class T>
  class seq final : public node<T> {
    public:
      seq(std::vector<node_ptr<T>> children) {
        this->m_children = std::move(children);
      }

      virtual execution_state evaluate(T& blackboard) const override {
        for (auto& child : this->m_children) {
          auto state = child->evaluate(blackboard);
          if (state != execution_state::success) {
            return state;
          }
        }

        return execution_state::success;
      }
  };

  /**
   * @ingroup behtree
   * @class sel
   * @brief Selector node, will execute all children in order until one succeeds
   */
  template <class T>
  class sel final : public node<T> {
    public:
      sel(std::vector<node_ptr<T>> children) {
        this->m_children = std::move(children);
      }

      virtual execution_state evaluate(T& blackboard) const override {
        for (auto& child : this->m_children) {
          auto state = child->evaluate(blackboard);
          if (state != execution_state::failure) {
            return state;
          }
        }

        return execution_state::failure;
      }
  };

  /**
   * @ingroup behtree
   * @class neg
   * @brief Negate node, will return the opposite of the child node
   */
  template <class T>
  class neg final : public node<T> {
    public:
      template <node_trait<T> N>
      neg(N&& child) {
        this->m_children.reserve(1);
        this->m_children.push_back(std::make_unique<N>(std::move(child)));
      }

      virtual execution_state evaluate(T& blackboard) const override {
        if (this->m_children.size() != 1) {
          return execution_state::failure;
        }

        auto& child = this->m_children.front();
        auto state = child->evaluate(blackboard);
        if (state == execution_state::success) {
          return execution_state::failure;
        } else if (state == execution_state::failure) {
          return execution_state::success;
        }

        return state;
      }
  };

  /**
   * @ingroup behtree
   * @class check
   * @brief Check node, will return success if the callback returns true
   */
  template <class T>
  class check final : public node<T> {
    public:
      using callback_type = std::function<bool(const T&)>;

    public:
      check(callback_type fn) : m_fn(fn) {}

      virtual execution_state evaluate(T& blackboard) const override {
        if (m_fn(blackboard)) {
          return execution_state::success;
        }

        return execution_state::failure;
      }

    private:
      callback_type m_fn;
  };

  /**
   * @ingroup behtree
   * @class task
   * @brief Task node, will execute the callback and return the result
   */
  template <class T>
  class task final : public node<T> {
    public:
      using callback_type = std::function<execution_state(T&)>;

    public:
      task(callback_type fn) : m_fn(fn) {}

      virtual execution_state evaluate(T& blackboard) const override {
        return m_fn(blackboard);
      }

    private:
      callback_type m_fn;
  };
}
