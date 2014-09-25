// Each task carries information about the computing unit used, but a wrapping
// is needed around the execution. This manager provides the wrapping.
//
// If the computation must be performed locally, the method "process_local" must
// be called with the task. The manager then will find the correct computing
// unit, create a new key for the result and execute the computation, which will
// load the data and store the result.
//
// For remote operation, see the file computing_unit_manager_mpi.hpp.

#ifndef __TASK_DISTRIBUTION__COMPUTING_UNIT_MANAGER_HPP__
#define __TASK_DISTRIBUTION__COMPUTING_UNIT_MANAGER_HPP__

#include "object_archive.hpp"
#include "key.hpp"
#include "task_entry.hpp"

namespace TaskDistribution {
  class ComputingUnitManager {
    public:
      ComputingUnitManager(ObjectArchive<Key>& archive);

      // Processes the task locally, so that loading task.result gives the
      // result.
      void process_local(TaskEntry& task);

    private:
      // Creates a new key of the given type.
      virtual Key new_key(Key::Type type);

      ObjectArchive<Key>& archive_;
  };
};

#endif
