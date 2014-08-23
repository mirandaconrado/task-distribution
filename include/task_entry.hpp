// Each task has an entry in the ObjectArchive, which is described in this file.
//
// The entry is composed of a set of keys required to compute the task and get
// its value. Additionally, flags "should_save" and "run_locally", the same in
// ComputingUnit, are provided for faster and easier management, as the unit
// would have to be loaded otherwise.
//
// As the id of the computing unit is required to call it, a key to it is stored
// in the entry. This may lead to data duplication (all nodes can create
// differente keys for the same id), but allows faster lookup and transmission.

#ifndef __TASK_DISTRIBUTION__TASK_ENTRY_HPP__
#define __TASK_DISTRIBUTION__TASK_ENTRY_HPP__

#include "key.hpp"

namespace TaskDistribution {
  // Archive entry for a task, having all values required work with it.
  struct TaskEntry {
    Key task_key;
    Key computing_unit_key;
    Key arguments_key;
    Key arguments_tasks_key;
    Key result_key;
    Key computing_unit_id_key;
    Key parents_key;
    Key children_key;
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
