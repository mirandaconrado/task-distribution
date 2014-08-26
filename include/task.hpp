#ifndef __TASK_DISTRIBUTION__TASK_HPP__
#define __TASK_DISTRIBUTION__TASK_HPP__

//#include <boost/any.hpp>
/*#if !(NO_MPI)
#include <boost/mpi/communicator.hpp>
#endif*/

#include "key.hpp"

namespace TaskDistribution {
  class TaskManager;

  class BaseTask {
    public:
      BaseTask():
        parents_active_(0),
        children_active_(0) { }

      /*template<class Archive>
      void serialize(Archive& ar, const unsigned int version);*/

      /*BaseTask(size_t id, TaskManager* task_manager);

      virtual ~BaseTask() { }

      size_t get_id() const {
        return id_;
      }

#if !(NO_MPI)
      virtual bool assign(boost::mpi::communicator& world, size_t node)=0;
      virtual void receive_value(boost::mpi::communicator& world,
          size_t node)=0;
#endif

      virtual bool run_locally() const=0;
      virtual bool should_save() const=0;

      virtual void compute()=0;

      virtual boost::any call()=0;

      virtual std::string get_name() const=0;

      virtual void unload()=0;*/

    protected:
      BaseTask(Key task_key):
        BaseTask() {
          task_key_ = task_key;
      }

      friend class TaskManager;

      /*bool on_memory_, on_disk_;

      size_t id_;

      TaskManager* task_manager_;*/

      size_t parents_active_;
      size_t children_active_;
      Key task_key_;
  };

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
