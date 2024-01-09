#pragma once

#include <memory>
#include <vector>

namespace aitoolkit::fsm {
  template <typename T>
  class state {
    public:
      virtual ~state() = default;

      virtual void enter(T& blackboard) = 0;
      virtual void exit(T& blackboard) = 0;

      virtual void pause(T& blackboard) = 0;
      virtual void resume(T& blackboard) = 0;

      virtual void update(T& blackboard) = 0;
  };

  template <typename T>
  using state_ptr = std::shared_ptr<state<T>>;

  template <typename T>
  class simple_machine {
    public:
      void set_state(state_ptr<T> state, T& blackboard) {
        if (m_current_state) {
          m_current_state->exit(blackboard);
        }

        m_current_state = state;

        if (m_current_state) {
          m_current_state->enter(blackboard);

          if (m_paused) {
            m_current_state->pause(blackboard);
          }
        }
      }

      void pause(T& blackboard) {
        m_paused = true;

        if (m_current_state) {
          m_current_state->pause(blackboard);
        }
      }

      void resume(T& blackboard) {
        m_paused = false;

        if (m_current_state) {
          m_current_state->resume(blackboard);
        }
      }

      void update(T& blackboard) {
        if (m_paused) {
          return;
        }

        if (m_current_state) {
          m_current_state->update(blackboard);
        }
      }

    private:
      state_ptr<T> m_current_state{nullptr};
      bool m_paused{false};
  };

  template <typename T>
  class stack_machine {
    public:
      void push_state(state_ptr<T> state, T& blackboard) {
        if (!m_state_stack.empty()) {
          auto current_state = m_state_stack.back();
          current_state->pause(blackboard);
        }

        if (state) {
          state->enter(blackboard);
          m_state_stack.push_back(state);
        }
      }

      void pop_state(T& blackboard) {
        if (!m_state_stack.empty()) {
          auto current_state = m_state_stack.back();
          current_state->exit(blackboard);
          m_state_stack.pop_back();
        }

        if (!m_state_stack.empty()) {
          auto current_state = m_state_stack.back();
          current_state->resume(blackboard);
        }
      }

      void update(T& blackboard) {
        if (!m_state_stack.empty()) {
          auto current_state = m_state_stack.back();
          current_state->update(blackboard);
        }
      }

    private:
      std::vector<state_ptr<T>> m_state_stack;
  };
}
