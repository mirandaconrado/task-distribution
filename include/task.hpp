#ifndef __TASK_DISTRIBUTION__TASK_HPP__
#define __TASK_DISTRIBUTION__TASK_HPP__

//#include <boost/any.hpp>
/*#if !(NO_MPI)
#include <boost/mpi/communicator.hpp>
#endif*/

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

      /*template <class Other>
      Task(Task<Other> const& other) {
        *this = task_manager_->template new_conversion_task<T>(other);
      }*/

      Task<T> const& operator=(Task<T> const& other) {
        task_key_ = other.task_key_;
        task_manager_ = other.task_manager_;
        return *this;
      }

      /*template <class Other>
      Task<T> const& operator=(Task<Other> const& other) {
        *this = task_manager_->template new_conversion_task<T>(other);
        return *this;
      }*/

      /*template<class Archive>
      void serialize(Archive& ar, const unsigned int version) {
        ar & task_key_;
      }*/

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
      //template <class> friend class Task;

      friend class TaskManager;

      //friend struct DependencyAnalyzer;

      Task(Key task_key, TaskManager* task_manager):
        task_key_(task_key),
        task_manager_(task_manager) { }

      Key task_key_;

      TaskManager* task_manager_;
  };
};

#include "task_impl.hpp"

#endif
