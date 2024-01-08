#pragma once

#include <initializer_list>
#include <functional>
#include <memory>
#include <vector>

namespace aitoolkit::bt {
  enum class execution_state {
    success,
    failure,
    running
  };

  template <class T>
  class node {
    public:
      virtual execution_state evaluate(T& blackboard) const = 0;

    protected:
      std::vector<std::shared_ptr<node<T>>> m_children;
  };

  template <class T>
  using node_ptr = std::shared_ptr<node<T>>;

  template <class T>
  class seq final : public node<T> {
    public:
      static node_ptr<T> make(std::initializer_list<node_ptr<T>> children) {
        auto seq_node = std::make_shared<seq<T>>();
        seq_node->m_children.reserve(children.size());

        for (auto& child : children) {
          seq_node->m_children.push_back(child);
        }

        return seq_node;
      }

      virtual execution_state evaluate(T& blackboard) const override {
        for (auto child : this->m_children) {
          auto state = child->evaluate(blackboard);
          if (state != execution_state::success) {
            return state;
          }
        }

        return execution_state::success;
      }
  };

  template <class T>
  class sel final : public node<T> {
    public:
      static node_ptr<T> make(std::initializer_list<node_ptr<T>> children) {
        auto sel_node = std::make_shared<sel<T>>();
        sel_node->m_children.reserve(children.size());

        for (auto child : children) {
          sel_node->m_children.push_back(child);
        }

        return sel_node;
      }

      virtual execution_state evaluate(T& blackboard) const override {
        for (auto child : this->m_children) {
          auto state = child->evaluate(blackboard);
          if (state != execution_state::failure) {
            return state;
          }
        }

        return execution_state::failure;
      }
  };

  template <class T>
  class neg final : public node<T> {
    public:
      static node_ptr<T> make(node_ptr<T> child) {
        auto neg_node = std::make_shared<neg<T>>();

        neg_node->m_children.reserve(1);
        neg_node->m_children.push_back(child);

        return neg_node;
      }

      virtual execution_state evaluate(T& blackboard) const override {
        if (this->m_children.size() != 1) {
          return execution_state::failure;
        }

        auto state = this->m_children.front()->evaluate(blackboard);
        if (state == execution_state::success) {
          return execution_state::failure;
        } else if (state == execution_state::failure) {
          return execution_state::success;
        }

        return state;
      }
  };

  template <class T>
  class check final : public node<T> {
    public:
      using callback_type = std::function<bool(const T&)>;

    public:
      static node_ptr<T> make(callback_type fn) {
        return std::make_shared<check<T>>(fn);
      }

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

  template <class T>
  class task final : public node<T> {
    public:
      using callback_type = std::function<execution_state(T&)>;

    public:
      static node_ptr<T> make(callback_type fn) {
        return std::make_shared<task<T>>(fn);
      }

      task(callback_type fn) : m_fn(fn) {}

      virtual execution_state evaluate(T& blackboard) const override {
        return m_fn(blackboard);
      }

    private:
      callback_type m_fn;
  };
}
