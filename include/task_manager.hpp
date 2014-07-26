#ifndef __TASK_DISTRIBUTION__TASK_MANAGER_HPP__
#define __TASK_DISTRIBUTION__TASK_MANAGER_HPP__

#if !(NO_MPI)
#include <boost/mpi/communicator.hpp>
#endif
#include <fstream>
#include <unordered_map>

#include "object_archive.hpp"

namespace TaskDistribution {
  class BaseTask;
  template <class T> class Task;

  class TaskManager {
    public:
      TaskManager(std::string const& path, size_t max_buffer_size);

      ~TaskManager();

      template <class Unit, class... Args>
      Task<typename Unit::result_type>
      new_task(Unit const& computing_unit, const Args& ... args);

      template <class T>
      Task<T> new_identity_task(const T& arg);

      void add_free_task(BaseTask* task);

      void run();

      BaseTask* get(size_t hash) const;

      void insert(size_t hash, std::string name, BaseTask* task);

      template <class T> void save(BaseTask* task, T const& val);

      template <class T> bool load(BaseTask* task, T& val);

      bool check(BaseTask* task) const;

      size_t id() const;

      void print_status();

      void invalidate(std::string name);

      void invalidate(BaseTask* task);

      void clean();

      void unload(BaseTask* task);

    private:
#if !(NO_MPI)
      void run_manager();

      void run_others();
#endif

      void run_single();

      void task_completed(BaseTask *task);

      ObjectArchive<size_t> archive_;

      std::unordered_map<size_t,BaseTask*> hash_map_;

      std::map<std::string,std::list<size_t>> name_map_;

      std::map<std::string,std::pair<size_t,size_t>> count_map_;

      std::list<BaseTask*> free_;

#if !(NO_MPI)
      boost::mpi::communicator world_;
#endif

      time_t last_print_;
  };
};

#include "task_manager_impl.hpp"

#endif
