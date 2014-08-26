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

      // Can't use operator== as it may force type conversion
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
