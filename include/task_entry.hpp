#ifndef __TASK_DISTRIBUTION__TASK_ENTRY_HPP__
#define __TASK_DISTRIBUTION__TASK_ENTRY_HPP__

#include "archive_key.hpp"

namespace TaskDistribution {
  // Archive entry for a task, having all values required work with it.
  struct TaskEntry {
    ArchiveKey task_key;
    ArchiveKey computing_unit_key;
    ArchiveKey arguments_key;
    ArchiveKey arguments_tasks_key;
    ArchiveKey result_key;
    ArchiveKey computing_unit_id_key;
    bool should_save, run_locally;

    TaskEntry(): should_save(true), run_locally(false) { }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & task_key;
      ar & computing_unit_key;
      ar & arguments_key;
      ar & arguments_tasks_key;
      ar & result_key;
      ar & computing_unit_id_key;
    }
  };
};

// Enables faster MPI transmission.
#if ENABLE_MPI
BOOST_IS_MPI_DATATYPE(TaskDistribution::TaskEntry);
#endif

#endif
