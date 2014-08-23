#ifndef __TASK_DISTRIBUTION__TASK_MANAGER_HPP__
#define __TASK_DISTRIBUTION__TASK_MANAGER_HPP__

#if ENABLE_MPI
#include <boost/mpi/communicator.hpp>
#include "mpi_object_archive.hpp"
#else
#include "object_archive.hpp"
#endif
//#include <fstream>
//#include <unordered_map>
//#include <unordered_set>

//#include <boost/predef.h>

#include "compile_utils.hpp"
#include "key.hpp"

namespace TaskDistribution {
  class BaseTask;
  template <class T> class Task;
  struct TaskEntry;

  class TaskManager {
    public:
      TaskManager() { }

      ~TaskManager() { }

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

      template <class Unit, class... Args>
      Task<typename function_traits<Unit>::return_type>
      new_task(Unit const& computing_unit, Args const&... args);

      template <class T>
      Task<T> new_identity_task(T const& arg);

      template <class T>
      Task<T> new_identity_task(Task<T> const& arg);

      /*template <class To, class From>
      Task<To> new_conversion_task(From&& arg);*/

      /*void add_free_task(BaseTask* task);

      void run();

      BaseTask* get(size_t hash) const;

      void insert(size_t hash, std::string name, BaseTask* task);

      template <class T> void save(BaseTask* task, T const& val);

      template <class T> bool load(BaseTask* task, T& val);

      bool check(BaseTask* task) const;*/

      //size_t id() const;

      /*void print_status();

      void invalidate(std::string name);

      void invalidate(BaseTask* task);

      void clean();

      void unload(BaseTask* task);*/

    private:
/*#if !(NO_MPI)
      void run_manager();

      void run_others();
#endif

      void run_single();

      void task_completed(BaseTask *task);*/

      // Checks if the given data already has a local key. If it does, returns
      // it. Otherwise, creates a new key and inserts it into the archive. This
      // is useful to avoid having lots of similar data with differente keys.
      template <class T>
      Key get_key(T const& data, Key::Type type);

      // Creates a new key of a given type
      Key new_key(Key::Type type);

      // Creates children and parents if they are invalid
      void create_family_lists(TaskEntry& entry);

      void add_dependency(BaseTask* child, TaskEntry const& child_entry,
          Key const& parent_key, KeySet& parents);

      /*void check_if_ready(ArchiveKey const& task_key);

      ArchiveKey new_object_key();

      ArchiveKey get_task(std::string const& unit_key,
          std::string const& args_key,
          std::string const& unit_str,
          std::string const& args_str);

      ArchiveKey get_component(std::string const& map_key,
          std::string const& str,
          std::unordered_multimap<std::string, ArchiveKey>& map);*/

#if ENABLE_MPI
      boost::mpi::communicator world_;
      MPIObjectArchive<Key> archive_;
#else
      ObjectArchive<Key> archive_;
#endif

      /*size_t next_free_obj_id_;*/


      //std::unordered_map<std::string, ArchiveKey> map_unit_ids_to_unit_ids_key_;
      std::unordered_map<Key, BaseTask*> map_key_to_task_;
      std::unordered_map<size_t, Key> map_hash_to_key_;
      std::unordered_map<Key, KeySet> map_task_to_parents_,
        map_task_to_children_;
      std::list<Key> ready_;
      //std::unordered_multimap<std::string, ArchiveKey> map_typenames_to_tasks_;
      //std::unordered_multimap<std::string, ArchiveKey> map_arg_names_to_args_;

      //std::unordered_set<ArchiveKey> tasks_ready_to_run_;

      /*std::unordered_map<size_t,BaseTask*> hash_map_;

      std::map<std::string,std::list<size_t>> name_map_;

      std::map<std::string,std::pair<size_t,size_t>> count_map_;

      std::list<BaseTask*> free_;*/

      /*time_t last_print_;*/

      // Auxiliary methods to build tuples.
      template <size_t I, class Tuple, class T1>
      static auto make_args_tuple_detail(T1 const& arg1) ->
      typename std::enable_if<(I == std::tuple_size<Tuple>::value-1),
               std::tuple<typename std::tuple_element<I,Tuple>::type>>::type
      {
        return std::tuple<typename std::tuple_element<I,Tuple>::type>(arg1);
      }

      template <size_t I, class Tuple, class T1>
      static auto make_args_tuple_detail(Task<T1> const& arg1) ->
      typename std::enable_if<(I == std::tuple_size<Tuple>::value-1),
               std::tuple<typename std::tuple_element<I,Tuple>::type>>::type
      {
        return std::tuple<typename std::tuple_element<I,Tuple>::type>();
      }

      template <size_t I, class Tuple, class T1, class... Types>
      static auto make_args_tuple_detail(T1 const& arg1, Types const&... args) ->
      typename std::enable_if<(I < std::tuple_size<Tuple>::value-1),
        decltype(
            std::tuple_cat(
              std::tuple<typename std::tuple_element<I,Tuple>::type>(arg1),
              make_args_tuple_detail<I+1,Tuple>(args...)
            )
        )>::type
      {
        return std::tuple_cat(
            std::tuple<typename std::tuple_element<I,Tuple>::type>(arg1),
            make_args_tuple_detail<I+1,Tuple>(args...)
        );
      }

      template <size_t I, class Tuple, class T1, class... Types>
      static auto make_args_tuple_detail(Task<T1> const& arg1, Types const&... args) ->
      typename std::enable_if<(I < std::tuple_size<Tuple>::value-1),
        decltype(
            std::tuple_cat(
              std::tuple<typename std::tuple_element<I,Tuple>::type>(),
              make_args_tuple_detail<I+1,Tuple>(args...)
            )
        )>::type
      {
        return std::tuple_cat(
            std::tuple<typename std::tuple_element<I,Tuple>::type>(),
            make_args_tuple_detail<I+1,Tuple>(args...)
        );
      }

      template <class Tuple, class... Types>
      static Tuple make_args_tuple(Types const&... args) {
        return make_args_tuple_detail<0, Tuple>(args...);
      }

      template <size_t I, class Tuple, class T1>
      static auto make_args_tasks_tuple_detail(T1 const& arg1) ->
      typename std::enable_if<(I == std::tuple_size<Tuple>::value-1),
               std::tuple<Key>>::type
      {
        return std::tuple<Key>();
      }

      template <size_t I, class Tuple, class T1>
      static auto make_args_tasks_tuple_detail(Task<T1> const& arg1) ->
      typename std::enable_if<(I == std::tuple_size<Tuple>::value-1),
               std::tuple<Key>>::type
      {
        return std::tuple<Key>(arg1.task_key_);
      }

      template <size_t I, class Tuple, class T1, class... Types>
      static auto make_args_tasks_tuple_detail(T1 const& arg1, Types const&... args) ->
      typename std::enable_if<(I < std::tuple_size<Tuple>::value-1),
        decltype(
            std::tuple_cat(
              std::tuple<Key>(),
              make_args_tasks_tuple_detail<I+1,Tuple>(args...)
            )
        )>::type
      {
        return std::tuple_cat(
            std::tuple<Key>(),
            make_args_tasks_tuple_detail<I+1,Tuple>(args...)
        );
      }

      template <size_t I, class Tuple, class T1, class... Types>
      static auto make_args_tasks_tuple_detail(Task<T1> const& arg1, Types const&... args) ->
      typename std::enable_if<(I < std::tuple_size<Tuple>::value-1),
        decltype(
            std::tuple_cat(
              std::tuple<Key>(arg1.task_key_),
              make_args_tasks_tuple_detail<I+1,Tuple>(args...)
            )
        )>::type
      {
        return std::tuple_cat(
            std::tuple<Key>(arg1.task_key_),
            make_args_tasks_tuple_detail<I+1,Tuple>(args...)
        );
      }

      template <class Tuple, class... Types>
      static Tuple make_args_tasks_tuple(Types const&... args) {
        return make_args_tasks_tuple_detail<0, Tuple>(args...);
      }
  };
};

#include "task_manager_impl.hpp"

#endif
