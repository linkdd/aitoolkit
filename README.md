# AI Toolkit

<center>

![tests](https://img.shields.io/github/actions/workflow/status/linkdd/aitoolkit/tests.yml?style=flat-square&logo=github&label=tests)
![docs](https://img.shields.io/github/actions/workflow/status/linkdd/aitoolkit/docs.yml?style=flat-square&logo=github&label=docs)
![license](https://img.shields.io/github/license/linkdd/aitoolkit?style=flat-square&color=blue)
![version](https://img.shields.io/github/v/release/linkdd/aitoolkit?style=flat-square&color=red)

</center>

**AI Toolkit** is a header-only C++ library which provides tools for building
the brain of your game's NPCs.

It provides:

 - Finite State Machines
 - Behavior Tree
 - Utility AI
 - Goal Oriented Action Planning

## Installation

Add the `include` folder of this repository to your include paths.

Or add it as a submodule:

```
$ git submodule add https://github.com/linkdd/aitoolkit.git
$ g++ -std=c++23 -Iaiotoolkit/include main.cpp -o mygame
```

## Usage

### Finite State Machine

First, include the header:

```cpp
#include <aitoolkit/fsm.hpp>

using namespace aitoolkit::fsm;
```

Then, create your blackboard type:

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

Create your simple state machine:

```cpp
auto simple_bb = blackboard_type{};
auto simple_fsm = simple_machine<blackboard_type>();

simple_fsm.set_state(std::make_shared<state_dummy>(), simple_bb);
simple_fsm.pause(simple_bb);
simple_fsm.resume(simple_bb);
simple_fsm.update(simple_bb);
```

Or with a stack state machine:

```cpp
auto stack_bb = blackboard_type{};
auto stack_fsm = stack_machine<blackboard_type>{};

stack_fsm.push_state(std::make_shared<state_dummy>(), stack_bb);
stack_fsm.push_state(std::make_shared<state_dummy>(), stack_bb);

stack_fsm.update(stack_bb);

stack_fsm.pop_state(stack_bb);
stack_fsm.pop_state(stack_bb);
```

### Behavior Tree

First, include the header:

```cpp
#include <aitoolkit/behtree.hpp>

using namespace aitoolkit::bt;
```

Then, create your blackboard type:

```cpp
struct blackboard_type {
  // ...
};
```

Then, create your tree:

```cpp
auto tree = seq<blackboard_type>::make({
  check<blackboard_type>::make([](const blackboard_type& bb) {
    // check some condition
    return true;
  }),
  task<blackboard_type>::make([](blackboard_type& bb) {
    // perform some action
    return execution_state::success;
  })
});
```

Finally, evaluate it:

```cpp
auto blackboard = blackboard_type{
  // ...
};

auto state = tree->evaluate(blackboard);
```

For more informations, consult the
[documentation](https://linkdd.github.io/aitoolkit/group__behtree.html).

### Utility AI

TODO

### Goal Oriented Action Planning

First, include the header file:

```cpp
#include <aitoolkit/goap.hpp>

using namespace aitoolkit::goap;
```

Then, create a blackboard class that will hold the state of the planner:

```cpp
struct blackboard_type {
  bool has_axe{false};
  int wood{0};
};
```

Next, create a class for each action that you want to be able to perform:

```cpp
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
```

Finally, create a plan and run it:

```cpp
auto actions = std::vector<action_ptr<blackboard_type>>{
  std::make_shared<get_axe>(),
  std::make_shared<chop_tree>()
};
auto initial = blackboard_type{};
auto goal = blackboard_type{
  .has_axe = true,
  .wood = 3
};

auto p = plan<blackboard_type>(actions, initial, goal);
p.run(initial); // will mutate the blackboard
```

For more informations, consult the
[documentation](https://linkdd.github.io/aitoolkit/group__goap.html).

## Documentation

The documentation is available online [here](https://linkdd.github.io/aitoolkit).

You can build it locally using [doxygen](https://www.doxygen.nl/):

```
$ make docs
```

## License

This library is released under the terms of the [MIT License](./LICENSE.txt).
