#ifndef __TASK_DISTRIBUTION__TASK_HPP__
#define __TASK_DISTRIBUTION__TASK_HPP__

#include <boost/any.hpp>
/*#if !(NO_MPI)
#include <boost/mpi/communicator.hpp>
#endif*/

#include "archive_key.hpp"

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
      BaseTask(ArchiveKey task_key):
        BaseTask() {
          task_key_ = task_key;
      }

      friend class TaskManager;

      /*bool on_memory_, on_disk_;

      size_t id_;

      TaskManager* task_manager_;*/

      size_t parents_active_;
      std::list<ArchiveKey> parents_;

      size_t children_active_;
      std::list<ArchiveKey> children_;

      ArchiveKey task_key_;
  };

  template <class Unit, class Args>
  class RealTask: public BaseTask {
    public:
      /*virtual boost::any call();

      virtual ~RealTask();*/

      /*template<class Archive>
      void serialize(Archive& ar, const unsigned int version) {
        ar & boost::serialization::base_object<BaseTask>(*this);
      }*/

    private:
      // Not implemented
      RealTask();
      RealTask(RealTask const&);
      RealTask<Unit,Args> const& operator=(RealTask<Unit,Args> const&);

      /*static RealTask<Unit,Args>* get(TaskManager* task_manager,
                                  Unit const& callable,
                                  Args const& args);

      RealTask(TaskManager* task_manager,
               Unit const& callable,
               Args const& args,
               size_t id);*/

      RealTask(ArchiveKey task_key): BaseTask(task_key) { }

/*#if !(NO_MPI)
      virtual bool assign(boost::mpi::communicator& world, size_t node);
      virtual void receive_result(boost::mpi::communicator& world, size_t node);
#endif

      virtual void compute();
      virtual void unload();

      virtual bool run_locally() const {
        return computing_unit_.run_locally();
      }

      virtual bool should_save() const {
        return computing_unit_.shoud_save();
      }

      virtual std::string get_name() const {
        return Unit::name;
      }*/

      //friend class TaskManager;

      /*Unit const computing_unit_;

      Args const args_;

      typename Unit::result_type* result_;*/
  };

  template <class T>
  class Task {
    public:
      Task(): task_manager_(nullptr) { }

      Task(Task<T> const& other) {
        task_key_ = other.task_key_;
        task_manager_ = other.task_manager_;
      }

      template <class Other>
      Task(Task<Other> const& other) {
        *this = other.task_manager_->template new_conversion_task<T>(other);
      }

      Task<T> const& operator=(Task<T> const& other) {
        task_key_ = other.task_key_;
        task_manager_ = other.task_manager_;
      }

      template <class Other>
      Task<T> const& operator=(Task<Other> const& other) {
        *this = other.task_manager_->template new_conversion_task<T>(other);
        return *this;
      }

      template<class Archive>
      void serialize(Archive& ar, const unsigned int version) {
        ar & task_key_;
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
      /*template <class> friend class Task;

      friend class TaskManager;

      friend struct DependencyAnalyzer;*/

      Task(ArchiveKey task_key, TaskManager* task_manager):
        task_key_(task_key),
        task_manager_(task_manager) { }

      ArchiveKey task_key_;

      TaskManager* task_manager_;
  };
};

#include "task_impl.hpp"

#endif
