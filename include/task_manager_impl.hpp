#ifndef __TASK_DISTRIBUTION__TASK_MANAGER_IMPL_HPP__
#define __TASK_DISTRIBUTION__TASK_MANAGER_IMPL_HPP__

#include "task_manager.hpp"

#include "computing_unit.hpp"
#include "dependency_analyzer.hpp"
#include "task.hpp"

#include <boost/serialization/set.hpp>
#include <functional>
/*#include <boost/integer/static_min_max.hpp>*/

//#include <algorithm>
//#include <tuple>

namespace TaskDistribution {

  template <class Unit, class... Args>
  Task<typename function_traits<Unit>::return_type>
  TaskManager::new_task(Unit const& computing_unit, Args const&... args) {
    typedef typename clean_tuple<Args...>::type args_tuple_type;
    typedef typename repeated_tuple<std::tuple_size<args_tuple_type>::value,
            ArchiveKey>::type args_tasks_tuple_type;

    // Checks if the arguments are valid
    static_assert(
        std::tuple_size<args_tuple_type>::value == function_traits<Unit>::arity,
        "Invalid number of arguments."
    );

    static_assert(
        is_tuple_convertible<
          args_tuple_type,
          typename function_traits<Unit>::arg_tuple_type
        >::value,
        "Can't convert from arguments provided to expected."
    );

    // Make tuples of normal arguments and task arguments
    args_tuple_type args_tuple(make_args_tuple<args_tuple_type>(args...));
    args_tasks_tuple_type args_tasks_tuple(
        make_args_tasks_tuple<args_tasks_tuple_type>(args...));

    printf("%s\n", typeid(args_tuple_type).name());

    // Gets keys
    ArchiveKey computing_unit_key = get_key(computing_unit,
        ArchiveKey::ComputingUnit);
    ArchiveKey computing_unit_id_key = get_key(computing_unit.get_id(),
        ArchiveKey::ComputingUnitId);
    ArchiveKey arguments_key = get_key(args_tuple,
        ArchiveKey::Arguments);
    ArchiveKey arguments_tasks_key = get_key(args_tasks_tuple,
        ArchiveKey::ArgumentsTasks);

    // Builds entry
    TaskEntry task_entry;
    task_entry.computing_unit_key = computing_unit_key;
    task_entry.computing_unit_id_key = computing_unit_id_key;
    task_entry.arguments_key = arguments_key;
    task_entry.arguments_tasks_key = arguments_tasks_key;
    task_entry.should_save = computing_unit.should_save();
    task_entry.run_locally = computing_unit.run_locally();

    // Stores task entry and update its internal data
    ArchiveKey task_key = get_key(task_entry, ArchiveKey::Task);
    archive_.load(task_key, task_entry);
    task_entry.task_key = task_key;

    // Creates children and parents if they don't exist
    if (!task_entry.parents_key.is_valid()) {
      task_entry.parents_key = new_key(ArchiveKey::Parents);
      archive_.insert(task_entry.parents_key, std::set<ArchiveKey>());
    }

    if (!task_entry.children_key.is_valid()) {
      task_entry.children_key = new_key(ArchiveKey::Children);
      archive_.insert(task_entry.children_key, std::set<ArchiveKey>());
    }

    archive_.insert(task_key, task_entry);

    // Checks if the task in memory already exists
    auto it = map_key_to_task_.find(task_key);
    if (it == map_key_to_task_.end()) {
      // If not, creates the task object
      BaseTask* task = new BaseTask(task_key);
      map_key_to_task_.emplace(task_key, task);

      // Do dependency analysis
      DependencyAnalyzer da;
      da.analyze(args_tasks_tuple);

      // Loads parents
      std::set<ArchiveKey> parents;
      archive_.load(task_entry.parents_key, parents);

      // For each parent found...
      for (auto& parent_key: da.dependencies) {
        BaseTask* parent = map_key_to_task_.at(parent_key);

        TaskEntry parent_entry;
        archive_.load(parent_key, parent_entry);

        std::set<ArchiveKey> children;
        archive_.load(parent_entry.children_key, children);

        // Creates bidirectional maps
        map_task_to_parents_[task_key].insert(parent_key);
        map_task_to_children_[parent_key].insert(task_key);

        parents.insert(parent_key);
        children.insert(task_key);

        archive_.insert(parent_entry.children_key, children);
        printf("added dependency (%lu,%lu)\n", parent_entry.task_key.obj_id,
            task_entry.task_key.obj_id);

        if (!parent_entry.result_key.is_valid())
          task->parents_active_++;
        if (!task_entry.result_key.is_valid())
          parent->children_active_++;
      }

//      check_if_ready(task_key);
    }
    else
      printf("found existing task\n");

    return Task<typename function_traits<Unit>::return_type>(task_key, this);
  }

  template <class T>
  Task<T> TaskManager::new_identity_task(T const& arg) {
    return new_task(IdentityComputingUnit<T>(), arg);
  }

  template <class T>
  Task<T> TaskManager::new_identity_task(Task<T> const& arg) {
    return arg;
  }

  /*template <class To, class From>
  Task<To> TaskManager::new_conversion_task(From const& arg) {
    return new_task(ConvertComputingUnit<From,To>(), std::forward<From>(arg));
 }*/

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

  template <class T>
  ArchiveKey TaskManager::get_key(T const& data, ArchiveKey::Type type) {
    std::string data_str = ObjectArchive<ArchiveKey>::serialize(data);
    // If collision happens, we are screwed.
    std::hash<std::string> hasher;
    size_t hash = hasher(data_str);
    auto it = map_hash_to_key_.find(hash);
    if (it == map_hash_to_key_.end()) {
      ArchiveKey key = new_key(type);
      map_hash_to_key_.emplace(hash, key);
      archive_.insert_raw(key, std::move(data_str));
      return key;
    }

    return it->second;
  }

  ArchiveKey TaskManager::new_key(ArchiveKey::Type type) {
    return
#if ENABLE_MPI
      ArchiveKey::new_key(world_, type);
#else
      ArchiveKey::new_key(type);
#endif
  }
};

#endif
