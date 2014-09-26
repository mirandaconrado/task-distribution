// Each task is represented by a front-end given back to the user, which can be
// used to access the result of the computation. This file defines this front
// end.
//
// The only thing the user has access is to the type of result provided by the
// computing unit used to create the task, as it identifies which time it can be
// converted.
//
// The usual method to access the result is just to cast the task, like:
// Task<int> t = ...;
// int bar = ...;
// int foo = t + bar;
//
// This automatically coerces the task to its type. If the result hasn't been
// computed yet (maybe because the task manager hasn't run), it is computed
// locally at the time of coercion.
//
// The operator() is provided and behaves like coercing, but some functions,
// like printf, will give error because they don't implicitly perform type
// conversion.

#ifndef __TASK_DISTRIBUTION__TASK_HPP__
#define __TASK_DISTRIBUTION__TASK_HPP__

#include "key.hpp"

namespace TaskDistribution {
  class TaskManager;

  template <class T>
  class Task {
    public:
      Task(): task_manager_(nullptr) { }

      Task(Task<T> const& other) {
        task_key_ = other.task_key_;
        task_manager_ = other.task_manager_;
      }

      Task<T> const& operator=(Task<T> const& other) {
        task_key_ = other.task_key_;
        task_manager_ = other.task_manager_;
        return *this;
      }

      // Can't use operator== as it may force type coercion
      bool is_same_task(Task<T> const& other) const {
        return task_key_ == other.task_key_ &&
               task_manager_ == other.task_manager_;
      }

      template <class Other>
      bool is_same_task(Task<Other> const& other) const {
        return false;
      }

      operator T() const;

      T operator()() const {
        return (T)*this;
      }

    private:
      friend class TaskManager;

      Task(Key task_key, TaskManager* task_manager):
        task_key_(task_key),
        task_manager_(task_manager) { }

      Key task_key_;

      TaskManager* task_manager_;
  };
};

#include "task_impl.hpp"

#endif
