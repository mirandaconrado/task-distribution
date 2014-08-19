#include "computing_unit_manager.hpp"

#include "computing_unit.hpp"

namespace TaskDistribution {
#if ENABLE_MPI
  ComputingUnitManager::ComputingUnitManager(boost::mpi::communicator& world,
      MPIObjectArchive<ArchiveKey>& archive):
    ComputingUnitManager(Tags(), world, archive) { }

  ComputingUnitManager::ComputingUnitManager(Tags const& tags,
      boost::mpi::communicator& world, MPIObjectArchive<ArchiveKey>& archive):
    world_(world),
    tags_(tags),
    archive_(archive) { }
#else
  ComputingUnitManager::ComputingUnitManager(
      ObjectArchive<ArchiveKey>& archive):
    archive_(archive) { }
#endif

  void ComputingUnitManager::process_local(TaskEntry& task) {
    if (task.should_save) {
#if ENABLE_MPI
      task.result_key = ArchiveKey::new_key(world_);
#else
      task.result_key = ArchiveKey::new_key();
#endif
    }

    BaseComputingUnit const* unit =
      BaseComputingUnit::get_by_id(task.computing_unit_id);
    unit->execute(archive_, task, *this);
  }

#if ENABLE_MPI
  void ComputingUnitManager::process_remote() {
    archive_.mpi_process();

    bool stop = false;
    while (!stop) {
      // Probes world and, if something is found, check if it's a tag it can
      // deal right here. Otherwise, stops.
      auto status_opt = world_.iprobe();
      if (status_opt) {
        auto status = status_opt.get();

        if (status.tag() == tags_.task_begin) {
          TaskEntry task;
          world_.recv(status.source(), status.tag(), task);

          process_local(task);

          world_.isend(status.source(), tags_.task_end, task);
        }
        else if (status.tag() == tags_.task_end) {
          TaskEntry task;
          world_.recv(status.source(), status.tag(), task);

          archive_.insert(task.task_key, task);
        }
        else
          stop = true;
      }
      else
        stop = true;
    }
  }

  void ComputingUnitManager::send_remote(TaskEntry const& task, int remote) {
    world_.isend(remote, tags_.task_begin, task);
  }
#endif
}
