#include "computing_unit_manager.hpp"

namespace TaskDistribution {
#if ENABLE_MPI
  ComputingUnitManager::ComputingUnitManager(boost::mpi::communicator& world,
      ObjectArchive<ArchiveKey>& archive):
    ComputingUnitManager(Tags(), world, archive) { }

  ComputingUnitManager::ComputingUnitManager(Tags const& tags,
      boost::mpi::communicator& world, ObjectArchive<ArchiveKey>& archive):
    world_(world),
    tags_(tags),
    archive_(archive) { }
#else
  ComputingUnitManager::ComputingUnitManager(
      ObjectArchive<ArchiveKey>& archive):
    archive_(archive) { }
#endif

  void ComputingUnitManager::process_local(TaskEntry const& task) {
    BaseComputingUnit const* unit =
      BaseComputingUnit::get_by_id(task.computing_unit_id);
    unit->execute(archive_, task);
  }

#if ENABLE_MPI
  void ComputingUnitManager::process_remote() {
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

          task.result = ArchiveKey::new_key(world_);

          process_local(task);

          world_.isend(status.source(), tags_.task_end, task);
        }
        else if (status.tag() == tags_.task_end) {
          TaskEntry task;
          world_.recv(status.source(), status.tag(), task);

          archive_.insert(task.task, task);

          std::string val;
          archive_.load_raw(task.result, val, false);
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
