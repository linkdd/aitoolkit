#pragma once

#include <initializer_list>
#include <memory>
#include <vector>
#include <limits>

namespace aitoolkit::utility {
  template <typename T>
  class action {
    public:
      virtual ~action() = default;

      virtual float score(const T& blackboard) const = 0;
      virtual void apply(T& blackboard) const = 0;
  };

  template <typename T>
  using action_ptr = std::shared_ptr<action<T>>;

  template <typename T>
  class evaluator {
    public:
      evaluator(std::initializer_list<action_ptr<T>> actions) {
        m_actions.reserve(actions.size());
        for (auto action : actions) {
          m_actions.push_back(action);
        }
      }

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
