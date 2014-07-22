#ifndef __TASK_DISTRIBUTION__TASK_HPP__
#define __TASK_DISTRIBUTION__TASK_HPP__

#include <boost/any.hpp>
#if !(NO_MPI)
#include <boost/mpi/communicator.hpp>
#endif

namespace TaskDistribution {
  class TaskManager;

  class BaseTask {
    public:
      BaseTask(size_t id, TaskManager* task_manager);

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

      virtual void unload()=0;

    protected:
      friend class TaskManager;

      bool on_memory_, on_disk_;

      size_t id_;

      TaskManager* task_manager_;

      size_t parents_active_;
      std::list<BaseTask*> parents_;

      size_t children_active_;
      std::list<BaseTask*> children_;
  };

  template <class Unit, class Args>
  class RealTask: public BaseTask {
    public:
      virtual boost::any call();

      virtual ~RealTask();

    private:
      // Not implemented
      RealTask();
      RealTask(RealTask const&);
      RealTask<Unit,Args> const& operator=(RealTask<Unit,Args> const&);

      friend class TaskManager;

      static RealTask<Unit,Args>* get(TaskManager* task_manager,
                                  Unit const& callable,
                                  Args const& args);

      RealTask(TaskManager* task_manager,
               Unit const& callable,
               Args const& args,
               size_t id);

#if !(NO_MPI)
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
      }

      Unit const computing_unit_;

      Args const args_;

      typename Unit::result_type* result_;
  };

  template <class T>
  class Task {
    public:
      Task(): task(nullptr) { }

      operator T() const {
        BOOST_ASSERT_MSG(task != nullptr, "task not created by a TaskManager");
        return boost::any_cast<T>(task->call());
      }

      T operator()() const {
        return (T)*this;
      }

      void unload() {
        task->unload();
      }

      size_t get_id() {
        return task->get_id();
      }

      friend size_t hash_value(Task const& t) {
        BOOST_ASSERT_MSG(t.task != nullptr,
                         "task not created by a TaskManager");
        return t.task->get_id();
      }

    private:
      friend class TaskManager;

      friend struct DependencyAnalyzer;

      Task(BaseTask* t): task(t) { }

      BaseTask* task;
  };
};

#include "task_impl.hpp"

#endif
