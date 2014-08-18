// Each task carries information about the computing unit used, but a wrapping
// is needed around the execution. This manager provides the wrapping.
//
// If the computation must be performed locally, the method "process_local" must
// be called with the task. The manager then will find the correct computing
// unit, create a new key for the result and execute the computation, which will
// load the data and store the result.
//
// For remote operation, the manager can send a task to a specific node using
// "send_remote". A node must call "process_remote" to process the MPI
// communication. There are two possible cases:
//
// 1) The manager receives a task_begin tag, which indicates that it must run
// the task locally. The task is computed using "process_local" and, after it
// finished, the requesting node is notified.
//
// 2) The manager receives a task_end tag, which indicates that a task it
// requested finished running. In this case, the manager updates the result key
// in the task, which allows the result to be loaded.
//
// Hence both sides must call "process_remote".

#ifndef __TASK_DISTRIBUTION__COMPUTING_UNIT_MANAGER_HPP__
#define __TASK_DISTRIBUTION__COMPUTING_UNIT_MANAGER_HPP__

#if ENABLE_MPI
#include <boost/mpi/communicator.hpp>
#include "mpi_object_archive.hpp"
#endif

#include "computing_unit.hpp"

namespace TaskDistribution {
  class ComputingUnitManager {
    public:
#if ENABLE_MPI
      // Tags that the managers use to communicate. The user can provide his own
      // values as long as they are differente and aren't used in any other
      // place.
      struct Tags {
        int task_begin = 7;
        int task_end = 8;
      };

      // Constructs with default tags.
      ComputingUnitManager(boost::mpi::communicator& world,
          MPIObjectArchive<ArchiveKey>& archive);
      ComputingUnitManager(Tags const& tags, boost::mpi::communicator& world,
          MPIObjectArchive<ArchiveKey>& archive);
#else
      ComputingUnitManager(ObjectArchive<ArchiveKey>& archive);
#endif

      // Processes the task locally, so that loading task.result gives the
      // result.
      void process_local(TaskEntry& task);

#if ENABLE_MPI
      // Processes messages from remote nodes.
      void process_remote();

      // Requests remote node to compute the task.
      void send_remote(TaskEntry const& task, int remote);
#endif

    private:
#if ENABLE_MPI
      boost::mpi::communicator& world_;
      Tags tags_;
      MPIObjectArchive<ArchiveKey>& archive_;
#else
      ObjectArchive<ArchiveKey>& archive_;
#endif
  };
};

#endif
