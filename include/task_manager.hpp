#ifndef __TASK_DISTRIBUTION__TASK_MANAGER_HPP__
#define __TASK_DISTRIBUTION__TASK_MANAGER_HPP__

#include "function_traits.hpp"
#include "sequence.hpp"
#if ENABLE_MPI
#include <boost/mpi/communicator.hpp>
#include "mpi_object_archive.hpp"
#include "mpi_handler.hpp"
#else
#include "object_archive.hpp"
#endif

#include "computing_unit_manager.hpp"
#include "key.hpp"

namespace TaskDistribution {
  template <class T> class Task;
  struct TaskEntry;

  class TaskManager {
    public:
#if ENABLE_MPI
      struct Tags {
        int finish = 9;
      };

      TaskManager(boost::mpi::communicator& world, MPIHandler& handler);
      TaskManager(Tags const& tags, boost::mpi::communicator& world,
          MPIHandler& handler);
#else
      TaskManager();
#endif

      ~TaskManager();

      //-- Begin Archive mappings --

      // Initializes the archive using a temporary file as backend. As the names
      // are random, it's possible to have a collision!
      void init() {
        archive_.init();
      }

      // Initializes the archive using a new file as backend.
      void init(std::string const& filename) {
        archive_.init(filename);
      }

      // Resets the buffer size to a certain number of bytes.
      void set_buffer_size(size_t max_buffer_size) {
        archive_.set_buffer_size(max_buffer_size);
      }

      // Same as the other, but the string holds the number of bytes for the
      // buffer, possibly with modifiers K, M or G. If more than one modifier is
      // found, then the first one is used.
      void set_buffer_size(std::string const& max_buffer_size) {
        archive_.set_buffer_size(max_buffer_size);
      }

#if BOOST_OS_LINUX
      // Sets the buffer size to a percentage of the FREE memory available in
      // the system. Currently only Linux is supported.
      void set_buffer_size_scale(float max_buffer_size) {
        archive_.set_buffer_size_scale(max_buffer_size);
      }
#endif

      //-- End Archive mappings --

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
      MPIObjectArchive<Key> archive_;
      bool finished_;
#else
      ObjectArchive<Key> archive_;
#endif

      // Maps object hashes to their keys, to avoid duplicated objects. If
      // there's a hash collision, the key will give the wrong object.
      std::unordered_map<size_t, Key> map_hash_to_key_;
      std::unordered_map<Key, KeySet> map_task_to_parents_,
        map_task_to_children_;
      // List of tasks that are ready to compute.
      KeyList ready_;
      ComputingUnitManager unit_manager_;

      // Auxiliary methods to build argument tuples tuples.
      template <class T>
      static Key get_task_key(T const& arg);

      template <class T>
      static Key get_task_key(Task<T> const& arg);

      template <class T1, class T2, size_t... S>
      static T1 make_args_tuple(T2 const& tuple, CompileUtils::sequence<S...>);

      // TODO: remove tuple argument
      template <class Tuple, class... Args>
      static Tuple make_args_tasks_tuple(Args const&... args);
  };
};

#include "task_manager_impl.hpp"

#endif
