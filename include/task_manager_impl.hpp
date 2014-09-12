#ifndef __TASK_DISTRIBUTION__TASK_MANAGER_IMPL_HPP__
#define __TASK_DISTRIBUTION__TASK_MANAGER_IMPL_HPP__

#include "clean_tuple.hpp"
#include "is_tuple_convertible.hpp"
#include "repeated_tuple.hpp"

#include "task_manager.hpp"

#include "computing_unit.hpp"
#include "task.hpp"

#include <boost/serialization/set.hpp>
#include <functional>
#include <type_traits>

namespace TaskDistribution {
  template <class Unit, class... Args>
  Task<typename CompileUtils::function_traits<Unit>::return_type>
  TaskManager::new_task(Unit const& computing_unit, Args const&... args) {
    if (world_.rank() != 0)
      return Task<
        typename CompileUtils::function_traits<Unit>::return_type>(Key(), this);

    typedef typename CompileUtils::clean_tuple<Args...>::type
      given_args_tuple_type;
    typedef typename CompileUtils::repeated_tuple<sizeof...(Args), Key>::type
      args_tasks_tuple_type;
    typedef typename CompileUtils::clean_tuple_from_tuple<
      typename CompileUtils::function_traits<Unit>::arg_tuple_type>::type
      unit_args_tuple_type;

    // Checks if the arguments are valid
    static_assert(
        std::tuple_size<given_args_tuple_type>::value ==
        std::tuple_size<unit_args_tuple_type>::value,
        "Invalid number of arguments."
    );

    static_assert(
        CompileUtils::is_tuple_convertible<
          given_args_tuple_type,
          unit_args_tuple_type
        >::value,
        "Can't convert from arguments provided to expected."
    );

    BOOST_ASSERT_MSG(!computing_unit.run_locally() ||
        computing_unit.should_save(),
        "a computing unit can only run locally if it stores its result");

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

    // Do dependency analysis
    KeyList dependencies({get_task_key(args)...});

    // Add dependencies
    KeySet parents;
    archive_.load(task_entry.parents_key, parents);

    for (auto& parent_key: dependencies)
      if (parent_key.is_valid())
        add_dependency(task_entry, parent_key, parents);

    archive_.insert(task_entry.parents_key, parents);

    // Check if task can be run now
    if (task_entry.active_parents == 0 && task_entry.should_save) {
      bool found = false;
      for (auto& key : ready_) {
        if (key == task_key) {
          found = true;
          break;
        }
      }

      // If task doesn't exist already, tell everyone we have a new task
      if (!found)
        ready_.push_back(task_key);
    }

    archive_.insert(task_key, task_entry);

    task_creation_handler_(computing_unit.get_id(), task_key);

    return Task<
      typename CompileUtils::function_traits<Unit>::return_type>(task_key,
          this);
  }

  template <class T>
  Task<T> TaskManager::new_identity_task(T const& arg) {
    return new_task(IdentityComputingUnit<T>(), arg);
  }

  template <class T>
  Task<T> TaskManager::new_identity_task(Task<T> const& arg) {
    return arg;
  }

  template <class T>
  Key TaskManager::get_key(T const& data, Key::Type type) {
    std::string data_str = ObjectArchive<Key>::serialize(data);
    std::hash<std::string> hasher;
    size_t hash = hasher(data_str);
    auto range = map_hash_to_key_.equal_range(hash);

    // Process found keys
    for (auto it = range.first; it != range.second; ++it) {
      std::string other_data_str;
      archive_.load_raw(it->second, other_data_str);
      if (data_str == other_data_str)
          return it->second;
    }

    // If no correct entry was found
    Key key = new_key(type);
    map_hash_to_key_.emplace(hash, key);
    archive_.insert_raw(key, std::move(data_str));
    return key;
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

  template <class T>
  T TaskManager::get_value(T const& arg) {
    return arg;
  }

  template <class T>
  T TaskManager::get_value(Task<T> const& arg) {
    return T();
  }

  template <class T>
  Key TaskManager::get_task_key(T const& arg) {
    return Key();
  }

  template <class T>
  Key TaskManager::get_task_key(Task<T> const& arg) {
    return arg.task_key_;
  }

  template <class Tuple, class... Args>
  Tuple TaskManager::make_args_tuple(Args const&... args) {
    return Tuple(get_value(args)...);
  }

  template <class Tuple, class... Args>
  Tuple TaskManager::make_args_tasks_tuple(Args const&... args) {
    return Tuple(get_task_key(args)...);
  }
};

#endif
