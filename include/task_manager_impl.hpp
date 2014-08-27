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
    typedef typename clean_tuple<Args...>::type given_args_tuple_type;
    typedef typename repeated_tuple<sizeof...(Args), Key>::type
      args_tasks_tuple_type;
    typedef typename function_traits<Unit>::arg_tuple_type unit_args_tuple_type;

    // Checks if the arguments are valid
    static_assert(
        std::tuple_size<given_args_tuple_type>::value ==
        std::tuple_size<unit_args_tuple_type>::value,
        "Invalid number of arguments."
    );

    static_assert(
        is_tuple_convertible<
          given_args_tuple_type,
          unit_args_tuple_type
        >::value,
        "Can't convert from arguments provided to expected."
    );

    // Make tuples of normal arguments and task arguments
    unit_args_tuple_type args_tuple(
        make_args_tuple<unit_args_tuple_type>(args...));
    args_tasks_tuple_type args_tasks_tuple(
        make_args_tasks_tuple<args_tasks_tuple_type>(args...));

    // Gets keys
    Key computing_unit_key = get_key(computing_unit,
        Key::ComputingUnit);
    Key computing_unit_id_key = get_key(computing_unit.get_id(),
        Key::ComputingUnitId);
    Key arguments_key = get_key(args_tuple,
        Key::Arguments);
    Key arguments_tasks_key = get_key(args_tasks_tuple,
        Key::ArgumentsTasks);

    // Builds entry
    TaskEntry task_entry;
    task_entry.computing_unit_key = computing_unit_key;
    task_entry.computing_unit_id_key = computing_unit_id_key;
    task_entry.arguments_key = arguments_key;
    task_entry.arguments_tasks_key = arguments_tasks_key;
    task_entry.should_save = computing_unit.should_save();
    task_entry.run_locally = computing_unit.run_locally();

    // Stores task entry and update its internal data
    Key task_key = get_key(task_entry, Key::Task);
    archive_.load(task_key, task_entry);
    task_entry.task_key = task_key;

    // Creates children and parents if they don't exist
    create_family_lists(task_entry);

    archive_.insert(task_key, task_entry);

    // Do dependency analysis
    DependencyAnalyzer da;
    da.analyze(args_tasks_tuple);

    // Add dependencies
    KeySet parents;
    archive_.load(task_entry.parents_key, parents);

    for (auto& parent_key: da.dependencies)
      add_dependency(task_entry, parent_key, parents);

    archive_.insert(task_entry.parents_key, parents);

    // Check if task can be run now
    if (task_entry.active_parents == 0) {
      bool found = false;
      for (auto& key : ready_) {
        if (key == task_key) {
          found = true;
          break;
        }
      }
      if (!found)
        ready_.push_back(task_key);
    }

    archive_.insert(task_key, task_entry);

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
  Key TaskManager::get_key(T const& data, Key::Type type) {
    std::string data_str = ObjectArchive<Key>::serialize(data);
    // If collision happens, we are screwed.
    std::hash<std::string> hasher;
    size_t hash = hasher(data_str);
    auto it = map_hash_to_key_.find(hash);
    if (it == map_hash_to_key_.end()) {
      Key key = new_key(type);
      map_hash_to_key_.emplace(hash, key);
      archive_.insert_raw(key, std::move(data_str));
      return key;
    }

    return it->second;
  }

  template <class T>
  void TaskManager::get_result(Key const& task_key, T& ret) {
    TaskEntry entry;
    archive_.load(task_key, entry);
    if (!entry.result_key.is_valid())
      unit_manager_.process_local(entry);

    archive_.load(entry.result_key, ret);

    if (!entry.result_key.is_valid())
      archive_.remove(entry.result_key);
  }
};

#endif
