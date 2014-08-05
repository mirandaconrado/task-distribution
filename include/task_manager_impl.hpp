#ifndef __TASK_DISTRIBUTION__TASK_MANAGER_IMPL_HPP__
#define __TASK_DISTRIBUTION__TASK_MANAGER_IMPL_HPP__

#include "task_manager.hpp"

#include "computing_unit.hpp"
#include "dependency_analyzer.hpp"
#include "task.hpp"

#include <boost/integer/static_min_max.hpp>

//#include <algorithm>
//#include <tuple>

namespace TaskDistribution {
  template <class Unit, class... Args>
  Task<typename function_traits<Unit>::return_type>
  TaskManager::new_task(Unit const& computing_unit, Args const&... args) {
    typedef typename clean_tuple<Args...>::type args_tuple_type;

    static_assert(
        std::tuple_size<args_tuple_type>::value == function_traits<Unit>::arity,
        "Invalid number of arguments."
    );

    static_assert(
        is_tuple_convertible<
          std::tuple<Args...>,
          typename function_traits<Unit>::arg_tuple_type
        >::value,
        "Can't convert from arguments provided to expected.");

    typedef typename convert_tuple<
      typename function_traits<Unit>::arg_tuple_type, args_tuple_type>::type
      converted_args_tuple_type;

    converted_args_tuple_type args_tuple(args...);

    std::string unit_str = ObjectArchive<ArchiveKey>::serialize(computing_unit);
    std::string args_str = ObjectArchive<ArchiveKey>::serialize(args_tuple);

    // Checks if the task already exists
    std::string unit_key = typeid(Unit).name();
    std::string args_key = typeid(args_tuple).name();

    ArchiveKey task_key = get_task(unit_key, args_key, unit_str, args_str);
    TaskEntry task_entry;
    archive_.load(task_key, task_entry, true);

    BaseTask* task;

    if (task_entry.task.is_valid())
      archive_.load(task_entry.task, task, true);
    else {
      // New task
      task_entry.should_save = computing_unit.should_save();
      task_entry.run_locally = computing_unit.run_locally();
      task_entry.task = new_object_key();
      archive_.insert(task_key, task_entry, true);
      task = new RealTask<Unit, std::tuple<Args...>>(task_key);
      archive_.insert(task_entry.task, task, true);
    }

    DependencyAnalyzer da;
    da.analyze(args_tuple);

    for (auto& parent_key: da.dependencies) {
      TaskEntry parent_entry;
      archive_.load(parent_key, parent_entry, true);

      BaseTask* parent;
      archive_.load(parent_entry.task, parent, true);

      parent->children_.push_back(task_key);
      task->parents_.push_back(parent_key);

      if (!parent_entry.result.is_valid())
        task->parents_active_++;
      if (!task_entry.result.is_valid())
        parent->children_active_++;

      archive_.insert(parent_entry.task, parent, true);

      delete parent;
    }

    if (!da.dependencies.empty())
      archive_.insert(task_entry.task, task, true);

    delete task;

    check_if_ready(task_key);

    return Task<typename function_traits<Unit>::return_type>(task_key, this);
  }

  template <class T>
  Task<T> TaskManager::new_identity_task(T const& arg) {
    return new_task(IdentityComputingUnit<T>(), arg);
  }

  template <class To, class From>
  Task<To> TaskManager::new_conversion_task(From const& arg) {
    return new_task(ConvertComputingUnit<From,To>(), arg);
  }

  /*template <class T>
  void TaskManager::save(BaseTask* t, T const& val) {
    size_t id = t->get_id();
    archive_.insert(id, val);
  }

  template <class T>
  bool TaskManager::load(BaseTask* t, T& val) {
    size_t id = t->get_id();
    size_t size = archive_.load(id, val);
    return size == 0;
  }*/
};

#endif
