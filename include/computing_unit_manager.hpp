#ifndef __TASK_DISTRIBUTION__COMPUTING_UNIT_MANAGER_HPP__
#define __TASK_DISTRIBUTION__COMPUTING_UNIT_MANAGER_HPP__

#if ENABLE_MPI
#include <boost/mpi/communicator.hpp>
#endif

#include "computing_unit.hpp"

namespace TaskDistribution {
  class ComputingUnitManager {
    public:
      struct Tags {
        int task_begin = 7;
        int task_end = 8;
      };

#if ENABLE_MPI
      ComputingUnitManager(boost::mpi::communicator& world,
          ObjectArchive<ArchiveKey>& archive);
      ComputingUnitManager(Tags const& tags, boost::mpi::communicator& world,
          ObjectArchive<ArchiveKey>& archive);
#else
      ComputingUnitManager(ObjectArchive<ArchiveKey>& archive);
#endif

      void process_local(TaskEntry const& task);

#if ENABLE_MPI
      void process_remote();
      void send_remote(TaskEntry const& task, int remote);
#endif

    private:
#if ENABLE_MPI
      boost::mpi::communicator& world_;
      Tags tags_;
#endif
      ObjectArchive<ArchiveKey>& archive_;
  };
};

#endif
