#ifndef __TASK_DISTRIBUTION__TASK_MANAGER_HPP__
#define __TASK_DISTRIBUTION__TASK_MANAGER_HPP__

#include "function_traits.hpp"
#include "object_archive.hpp"

#include "computing_unit_manager.hpp"
#include "key.hpp"

#include <functional>

namespace TaskDistribution {
  template <class T> class Task;
  struct TaskEntry;

  class TaskManager {
    public:
      typedef std::function<void (std::string const&, Key const&)>
        creation_handler_type;
      typedef std::function<void (Key const&)> action_handler_type;

      TaskManager(ObjectArchive<Key>& archive,
          ComputingUnitManager& unit_manager);

      virtual ~TaskManager();

      // Creates a task for the unit and arguments provided.
      template <class Unit, class... Args>
      Task<typename CompileUtils::function_traits<Unit>::return_type>
      new_task(Unit const& computing_unit, Args const&... args);

      // Creates a task that just gives back its argument. This should be used
      // with big arguments, as it avoids them being serialized many times,
      // which can be expensive.
      template <class T>
      Task<T> new_identity_task(T const& arg);

      // If the argument is a task already, just give it back.
      template <class T>
      Task<T> new_identity_task(Task<T> const& arg);

      // Local task processing.
      virtual void run();

      // Id of this manager, which is 0 for non-parallel approaches.
      virtual size_t id() const;

      void set_task_creation_handler(creation_handler_type handler);
      void clear_task_creation_handler();

      void set_task_begin_handler(action_handler_type handler);
      void clear_task_begin_handler();

      void set_task_end_handler(action_handler_type handler);
      void clear_task_end_handler();

      void load_archive();

    protected:
      template <class Unit>
      Task<typename CompileUtils::function_traits<Unit>::return_type>
      new_invalid_task(Unit const& computing_unit);

      // Runs locally until there are not more tasks
      void run_single();

      // Processes the end of a task, evaluating if its children may run
      void task_completed(Key const& task_key);

      std::string load_string_to_hash(Key const& key);

      virtual void update_used_keys(std::map<int, size_t> const& used_keys);

      // Checks if the given data already has a local key. If it does, returns
      // it. Otherwise, creates a new key and inserts it into the archive. This
      // is useful to avoid having lots of similar data with differente keys.
      template <class T>
      Key get_key(T const& data, Key::Type type);

      // Creates a new key of a given type.
      virtual Key new_key(Key::Type type);

      // Creates children and parents if they are invalid.
      void create_family_lists(TaskEntry& entry);

      // Creates the bilateral link between child and parent task.
      void add_dependency(TaskEntry& child_entry, Key const& parent_key,
          KeySet& parents);

      // Gets the result for a given task.
      template <class> friend class Task;
      template <class T>
      void get_result(Key const& task_key, T& ret);

      ObjectArchive<Key>& archive_;
      ComputingUnitManager& unit_manager_;

      creation_handler_type task_creation_handler_;
      action_handler_type task_begin_handler_, task_end_handler_;

      // Maps object hashes to their keys, to avoid duplicated objects.
      std::unordered_multimap<size_t, Key> map_hash_to_key_;

      // Maps tasks to their children. This is store in the archive, but a copy
      // is kept here for faster dependency analysis.
      std::unordered_map<Key, KeySet> map_task_to_children_;

      // List of tasks that are ready to compute.
      KeyList ready_;


      // Auxiliary methods to build argument tuples tuples.

      // Returns the given argument.
      template <class T>
      static T get_value(T const& arg);

      // Returns an empty value T.
      template <class T>
      static T get_value(Task<T> const& arg);

      // Returns an empty key.
      template <class T>
      static Key get_task_key(T const& arg);

      // Returns the task key.
      template <class T>
      static Key get_task_key(Task<T> const& arg);

      // Builds the tuple with only the value of arguments that aren't tasks.
      template <class Tuple, class... Args>
      static Tuple make_args_tuple(Args const&... args);

      // Builds the tuple with only the keys to tasks arguments.
      template <class Tuple, class... Args>
      static Tuple make_args_tasks_tuple(Args const&... args);
  };
};

#include "task_manager_impl.hpp"

#endif
