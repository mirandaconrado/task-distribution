// Each task has an entry in the ObjectArchive, which is described in this file.
//
// The entry is composed of a set of keys required to compute the task and get
// its value. Additionally, a flag "run_locally", the same in ComputingUnit, is
// provided for faster and easier management, as the unit would have to be
// loaded otherwise.
//
// As the id of the computing unit is required to call it, a key to it is stored
// in the entry. This may lead to data duplication (all nodes can create
// different keys for the same id), but allows faster lookup and transmission.

#ifndef __TASK_DISTRIBUTION__TASK_ENTRY_HPP__
#define __TASK_DISTRIBUTION__TASK_ENTRY_HPP__

#include "key.hpp"

namespace TaskDistribution {
  // Archive entry for a task, having all values required work with it.
  struct TaskEntry {
    Key task_key;              // Key to this entry
    Key computing_unit_key;    // Key to the computing unit
    Key arguments_key;         // Key to the tuple of arguments
    Key arguments_tasks_key;   // Key to the tuple of tasks as arguments
    Key result_key;            // Key to the result
    Key computing_unit_id_key; // Key to the unit id
    Key parents_key;           // Key to a list of keys of parent tasks
    Key children_key;          // Key to a list of keys of children tasks
    size_t active_parents;     // Number of parents that have to be computed
    bool run_locally;

    TaskEntry():
      active_parents(0),
      run_locally(false) { }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & task_key;
      ar & computing_unit_key;
      ar & arguments_key;
      ar & arguments_tasks_key;
      ar & result_key;
      ar & computing_unit_id_key;
      ar & parents_key;
      ar & children_key;
      ar & active_parents;
      ar & run_locally;
    }
  };
};

// Enables faster MPI transmission.
#if ENABLE_MPI
BOOST_IS_MPI_DATATYPE(TaskDistribution::TaskEntry);
#endif

#endif
