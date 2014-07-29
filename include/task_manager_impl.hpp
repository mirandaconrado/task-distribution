#ifndef __TASK_DISTRIBUTION__TASK_MANAGER_IMPL_HPP__
#define __TASK_DISTRIBUTION__TASK_MANAGER_IMPL_HPP__

#include "task_manager.hpp"

//#include "dependency_analyzer.hpp"
//#include "task.hpp"

//#include <algorithm>
//#include <tuple>

namespace TaskDistribution {
  template <class Unit, class... Args>
  Task<typename Unit::result_type>
  TaskManager::new_task(Unit const& computing_unit, Args const&... args) {
    auto args_tuple = std::make_tuple(args...);

    std::string unit_str = ObjectArchive<ArchiveKey>::serialize(computing_unit);
    std::string args_str = ObjectArchive<ArchiveKey>::serialize(args_tuple);

    // Checks if the task already exists
    std::string unit_key = typeid(Unit).name();
    std::string args_key = typeid(args_tuple).name();

    ArchiveKey task_key = get_task(unit_key, args_key, unit_str, args_str);

/*    DependencyAnalyzer da;
    std::for_each(args_tuple, da);
    auto* real_task =
      RealTask<Unit,std::tuple<Args...>>::get(this, computing_unit, args_tuple);

    for (auto &t: da.dependencies) {
      t->children_.push_back(real_task);
      real_task->parents_.push_back(t);

      if (!t->on_disk_ && t->should_save())
        real_task->parents_active_++;
      if (!real_task->on_disk && real_task->should_save())
        t->children_active_++;
    }

    add_free_task(real_task);

    return Task<typename Unit::result_type>(real_task);*/
  }

  /*template <class T>
  Task<T> TaskManager::new_identity_task(const T& arg) {
    return new_task(IdentityComputingUnit<T>(), arg);
  }

  template <class T>
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
