#ifndef __TASK_DISTRIBUTION__TASK_MANAGER_HPP__
#define __TASK_DISTRIBUTION__TASK_MANAGER_HPP__

#include "function_traits.hpp"
#if ENABLE_MPI
#include <boost/mpi/communicator.hpp>
#include "mpi_object_archive.hpp"
#include "mpi_handler.hpp"
#else
#include "object_archive.hpp"
#endif

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

#if ENABLE_MPI
      struct Tags {
        int finish = 9;
      };

      TaskManager(boost::mpi::communicator& world, MPIHandler& handler,
          MPIObjectArchive<Key>& archive);
      TaskManager(Tags const& tags, boost::mpi::communicator& world,
          MPIHandler& handler, MPIObjectArchive<Key>& archive);
#else
      TaskManager(ObjectArchive<Key>& archive);
#endif

      ~TaskManager();

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

      // Proxy to either local or MPI task processing.
      void run();

      // Id of this manager, which is its rank with MPI or 0 otherwise.
      size_t id() const;

      void set_task_creation_handler(creation_handler_type handler);
      void clear_task_creation_handler();

      void set_task_begin_handler(action_handler_type handler);
      void clear_task_begin_handler();

      void set_task_end_handler(action_handler_type handler);
      void clear_task_end_handler();

    private:
#if ENABLE_MPI
      // Runs the manager that allocates tasks.
      void run_master();

      // Runs the slaves that just compute stuff.
      void run_slave();

      // Sends the next ready task to a given slave. Returns true if a task was
      // allocated.
      bool send_next_task(int slave);

      // Handler to MPI tag.
      bool process_finish(int source, int tag);

      // Sends a finish tag to all other nodes.
      void broadcast_finish();
#endif

      // Runs locally until there are not more tasks
      void run_single();

      // Processes the end of a task, evaluating if its children may run
      void task_completed(Key const& task_key);

      // Checks if the given data already has a local key. If it does, returns
      // it. Otherwise, creates a new key and inserts it into the archive. This
      // is useful to avoid having lots of similar data with differente keys.
      template <class T>
      Key get_key(T const& data, Key::Type type);

      // Creates a new key of a given type.
      Key new_key(Key::Type type);

      // Creates children and parents if they are invalid.
      void create_family_lists(TaskEntry& entry);

      // Creates the bilateral link between child and parent task.
      void add_dependency(TaskEntry& child_entry, Key const& parent_key,
          KeySet& parents);

      // Gets the result for a given task.
      template <class> friend class Task;
      template <class T>
      void get_result(Key const& task_key, T& ret);

#if ENABLE_MPI
      Tags tags_;
      boost::mpi::communicator& world_;
      MPIHandler& handler_;
      MPIObjectArchive<Key>& archive_;
      bool finished_;
#else
      ObjectArchive<Key>& archive_;
#endif

      creation_handler_type task_creation_handler_;
      action_handler_type task_begin_handler_, task_end_handler_;

      // Maps object hashes to their keys, to avoid duplicated objects.
      std::unordered_multimap<size_t, Key> map_hash_to_key_;

      // Maps tasks to their children. This is store in the archive, but a copy
      // is kept here for faster dependency analysis.
      std::unordered_map<Key, KeySet> map_task_to_children_;

      // List of tasks that are ready to compute.
      KeyList ready_;

      ComputingUnitManager unit_manager_;

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
