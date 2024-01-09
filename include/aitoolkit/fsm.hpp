#pragma once

/**
@defgroup fsm Finite State Machine

## Introduction

A finite state machine (FSM) is a mathematical model of computation. It is an
abstract machine that can be in exactly one of a finite number of states at any
given time. The FSM can change from one state to another in response to some
external inputs; the change from one state to another is called a transition.

This library provides 2 types of FSMs: a simple machiine and a stack machine.
The simple machine is the simplest form of FSM, it can only be in one state at a
time. The stack machine is a more complex form of FSM, it can be in multiple
states at a time.

## Usage

First, include the header:

```cpp
#include <aitoolkit/fsm.hpp>
```

Then, create a blackboard type:

```cpp
struct blackboard_type {
  // ...
};
```

Then, create a state type for each of your states:

```cpp
class state_dummy final : public state<blackboard_type> {
  public:
    virtual void enter(blackboard_type& blackboard) override {
      // ...
    }

    virtual void exit(blackboard_type& blackboard) override {
      // ...
    }

    virtual void pause(blackboard_type& blackboard) override {
      // ...
    }

    virtual void resume(blackboard_type& blackboard) override {
      // ...
    }

    virtual void update(blackboard_type& blackboard) override {
      // ...
    }
};
```

### Simple state machine

Create an instance of the FSM:

```cpp
using namespace aitoolkit::fsm;

auto machine = simple_machine<blackboard_type>{};
```

To transition to a new state, call `set_state()`:

```cpp
machine.set_state(std::make_shared<state_dummy>(), blackboard);
```

> **NB:**
>
>  - this will call the `exit` method of the current state (if any) and the
>    `enter` method of the new state
>  - if the machine is paused while transitioning to a new state, the new state
>    will be paused as well

To pause the machine, call `pause()`:

```cpp
machine.pause(blackboard);
```

> **NB:** This will call the `pause` method of the current state (if any).

To resume the machine, call `resume()`:

```cpp
machine.resume(blackboard);
```

> **NB:** This will call the `resume` method of the current state (if any).

To update the machine, call `update()`:

```cpp
machine.update(blackboard);
```

> **NB:**
>
>  - this will call the `update` method of the current state (if any)
>  - if the machine is paused, calling `update()` will do nothing

To clear any state, call `set_state()` with a `nullptr`:

```cpp
machine.set_state(nullptr, blackboard);
```

> **NB:** This will call the `exit` method of the current state (if any).

### Stack state machine

Create an instance of the FSM:

```cpp
using namespace aitoolkit::fsm;

auto machine = stack_machine<blackboard_type>{};
```

To push a new state onto the stack, call `push_state()`:

```cpp
machine.push_state(std::make_shared<state_dummy>(), blackboard);
```

> **NB:** This will call the `pause` method of the current state (if any) and
> the `enter` method of the new state.

To pop the top state off the stack, call `pop_state()`:

```cpp
machine.pop_state(blackboard);
```

> **NB:** This will call the `exit` method of the current state (if any) and
> the `resume` method of the new top state (if any).

To update the machine, call `update()`:

```cpp
machine.update(blackboard);
```

> **NB:** This will call the `update` method of the top state (if any).
*/

#include <memory>
#include <vector>

namespace aitoolkit::fsm {
  /**
   * @ingroup fsm
   * @class state
   * @brief A state of the FSM.
   */
  template <typename T>
  class state {
    public:
      virtual ~state() = default;

      virtual void enter(T& blackboard) {};
      virtual void exit(T& blackboard) {};

      virtual void pause(T& blackboard) {};
      virtual void resume(T& blackboard) {};

      virtual void update(T& blackboard) {};
  };

  /**
   * @ingroup fsm
   * @brief Heap-allocated pointer to a state.
   */
  template <typename T>
  using state_ptr = std::shared_ptr<state<T>>;

  /**
   * @ingroup fsm
   * @class simple_machine
   * @brief A simple FSM.
   */
  template <typename T>
  class simple_machine {
    public:
      /**
       * @brief Enters in a new state, exiting the previous one (if any).
       */
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

      /**
       * @brief Pause the machine.
       */
      void pause(T& blackboard) {
        m_paused = true;

        if (m_current_state) {
          m_current_state->pause(blackboard);
        }
      }

      /**
       * @brief Resume the machine.
       */
      void resume(T& blackboard) {
        m_paused = false;

        if (m_current_state) {
          m_current_state->resume(blackboard);
        }
      }

      /**
       * @brief Update the machine.
       */
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

  /**
   * @ingroup fsm
   * @class stack_machine
   * @brief A stack FSM.
   */
  template <typename T>
  class stack_machine {
    public:
      /**
       * @brief Enters in a new state, pausing the previous one (if any).
      */
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

      /**
       * @brief Exits the current state, resuming the previous one (if any).
       */
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

      /**
       * @brief Update the machine.
       */
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
