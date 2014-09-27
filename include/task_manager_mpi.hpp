// Every task must be created by a manager, that controls its execution. This
// file describes the manager that allows them to be computed using MPI.

#ifndef __TASK_DISTRIBUTION__TASK_MANAGER_MPI_HPP__
#define __TASK_DISTRIBUTION__TASK_MANAGER_MPI_HPP__

#include <boost/mpi/communicator.hpp>
#include "object_archive_mpi.hpp"
#include "mpi_handler.hpp"

#include "computing_unit_manager_mpi.hpp"
#include "task_manager.hpp"

namespace TaskDistribution {
  class MPITaskManager: public TaskManager {
    public:
      // Tags that the managers use to communicate. The user can provide his own
      // values as long as they are differente and aren't used in any other
      // place.
      struct Tags {
        int finish = 9;
        int key_update = 10;
      };

      // Constructor with default tags.
      MPITaskManager(boost::mpi::communicator& world, MPIHandler& handler,
          MPIObjectArchive<Key>& archive,
          MPIComputingUnitManager& unit_manager);
      MPITaskManager(Tags const& tags, boost::mpi::communicator& world,
          MPIHandler& handler, MPIObjectArchive<Key>& archive,
          MPIComputingUnitManager& unit_manager);

      ~MPITaskManager();

      // MPI task processing.
      virtual void run();

      // Id of this manager, which is its rank with MPI.
      virtual size_t id() const;

    protected:
      // Runs the manager that allocates tasks.
      void run_master();

      // Runs the slaves that just compute stuff.
      void run_slave();

      // Allocates tasks to slaves and returns the number of new tasks running
      // remotely.
      size_t allocate_tasks();

      // Sends the next ready task to a given slave. Returns true if a task was
      // allocated.
      bool send_next_task(int slave);

      // Handler to MPI tag.
      bool process_finish(int source, int tag);
      bool process_key_update(int source, int tag);

      // Sends a finish tag to all other nodes.
      void broadcast_finish();

      // Updates the keys used in the archive, so that new keys don't conflict,
      // and sends relevant information for other nodes to update their keys.
      virtual void update_used_keys(std::map<int, size_t> const& used_keys);

      // Checks if the given data already has a local key. If it does, returns
      // it. Otherwise, creates a new key and inserts it into the archive. This
      // is useful to avoid having lots of similar data with differente keys.
      template <class T>
      Key get_key(T const& data, Key::Type type);

      // Creates a new key of a given type.
      virtual Key new_key(Key::Type type);

      Tags tags_;
      boost::mpi::communicator& world_;
      MPIHandler& handler_;
      MPIObjectArchive<Key>& archive_;
      MPIComputingUnitManager& unit_manager_;
      bool finished_;

      // Number of tasks allocated to each slave.
      std::vector<int> tasks_per_node_;
  };
};

#endif
