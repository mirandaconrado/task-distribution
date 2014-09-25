// Each task carries information about the computing unit used, but a wrapping
// is needed around the execution. This manager provides the wrapping.
//
// If the computation must be performed locally, see the file
// computing_unit_manager.hpp.
//
// For remote operation, the manager can send a task to a specific node using
// "send_remote". A slave node must call "process_remote" to process the MPI
// communication. There are two possible cases:
//
// 1) The manager receives a task_begin tag, which indicates that it must run
// the task locally. The task is computed using "process_local" and, after it
// finishes, the requesting node is notified.
//
// 2) The manager receives a task_end tag, which indicates that a task it
// requested finished running. In this case, the manager updates the list of
// tasks finished.

#ifndef __TASK_DISTRIBUTION__COMPUTING_UNIT_MANAGER_MPI_HPP__
#define __TASK_DISTRIBUTION__COMPUTING_UNIT_MANAGER_MPI_HPP__

#include <boost/mpi/communicator.hpp>
#include "object_archive_mpi.hpp"
#include "mpi_handler.hpp"

#include "computing_unit_manager.hpp"

namespace TaskDistribution {
  class MPIComputingUnitManager: public ComputingUnitManager {
    public:
      // Tags that the managers use to communicate. The user can provide his own
      // values as long as they are differente and aren't used in any other
      // place.
      struct Tags {
        int task_begin = 7;
        int task_end = 8;
      };

      typedef std::list<std::pair<Key, int>> TasksList;

      // Constructs with default tags.
      MPIComputingUnitManager(boost::mpi::communicator& world,
          MPIHandler& handler, MPIObjectArchive<Key>& archive);
      MPIComputingUnitManager(Tags const& tags, boost::mpi::communicator& world,
          MPIHandler& handler, MPIObjectArchive<Key>& archive);

      // Processes messages from remote nodes.
      void process_remote();

      // Requests remote node to compute the task.
      void send_remote(TaskEntry const& task, int remote);

      // Interface for the list of tasks that have finished.
      TasksList const& get_tasks_ended() const;
      void clear_tasks_ended();

    private:
      // Creates a new key of the given type.
      virtual Key new_key(Key::Type type);

      // Handlers for MPI tags.
      bool process_task_begin(int source, int tag);
      bool process_task_end(int source, int tag);

      boost::mpi::communicator& world_;
      MPIHandler& handler_;
      Tags tags_;

      // List of tasks that have ended by remotes
      TasksList tasks_ended_;
      // List of tasks that a remote requested to be executed by this node
      std::list<std::pair<TaskEntry,int>> tasks_requested_;
  };
};

#endif
