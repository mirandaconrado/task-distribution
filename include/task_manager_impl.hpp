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
            Key>::type args_tasks_tuple_type;

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
      KeySet parents;
      archive_.load(task_entry.parents_key, parents);

      // Add dependencies
      for (auto& parent_key: da.dependencies)
        add_dependency(task, task_entry, parent_key, parents);

      if (task->parents_active_ == 0)
        ready_.push_back(task_key);
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
};

#endif
