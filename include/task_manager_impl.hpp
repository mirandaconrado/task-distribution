#ifndef __TASK_DISTRIBUTION__TASK_MANAGER_IMPL_HPP__
#define __TASK_DISTRIBUTION__TASK_MANAGER_IMPL_HPP__

#include "task_manager.hpp"

#include "computing_unit.hpp"
#include "dependency_analyzer.hpp"
#include "task.hpp"

//#include <algorithm>
//#include <tuple>

namespace TaskDistribution {
  template <class Unit, class... Args>
  Task<typename Unit::result_type>
  TaskManager::new_task(Unit const& computing_unit, Args const&... args) {
    auto args_tuple = std::make_tuple(args...);

    BaseComputingUnit const* unit = &computing_unit;

    std::string unit_str = ObjectArchive<ArchiveKey>::serialize<Unit>(unit);
    std::string args_str = ObjectArchive<ArchiveKey>::serialize(args_tuple);

    // Checks if the task already exists
    std::string unit_key = typeid(Unit).name();
    std::string args_key = typeid(args_tuple).name();

    ArchiveKey task_key = get_task(unit_key, args_key, unit_str, args_str);
    TaskEntry task_entry;
    archive_.load(task_key, task_entry, true);

    BaseTask* task;

    if (task_entry.task.obj_id != 0)
      archive_.load(task_entry.task, task, true);
    else {
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

      if (parent_entry.result.obj_id == 0)
        task->parents_active_++;
      if (task_entry.result.obj_id == 0)
        parent->children_active_++;

      archive_.insert(parent_entry.task, parent, true);

      delete parent;
    }

    if (!da.dependencies.empty())
      archive_.insert(task_entry.task, task, true);

    delete task;

    check_if_ready(task_key);

    return Task<typename Unit::result_type>(task_key);
  }

  template <class T>
  Task<T> TaskManager::new_identity_task(T const& arg) {
    return new_task(IdentityComputingUnit<T>(), arg);
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
