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
    if (task.result_key.is_valid())
      return;

    if (task.should_save) {
#if ENABLE_MPI
      task.result_key = ArchiveKey::new_key(world_, ArchiveKey::Result);
#else
      task.result_key = ArchiveKey::new_key(ArchiveKey::Result);
#endif
    }

    // Assumes that the computing unit is defined. TODO: remove this assumption.
    BaseComputingUnit const* unit;

    // Gets the correct computing unit and executes it
    unit = BaseComputingUnit::get_by_key(task.computing_unit_id_key);
    if (unit == nullptr) {
      std::string computing_unit_id;
      archive_.load(task.computing_unit_id_key, computing_unit_id);
      // Assumes true is returned always.
      BaseComputingUnit::bind_key(computing_unit_id,
          task.computing_unit_id_key);
      unit = BaseComputingUnit::get_by_key(task.computing_unit_id_key);
    }

    unit->execute(archive_, task, *this);

    archive_.insert(task.task_key, task);
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

          world_.isend(status.source(), tags_.task_end, task.task_key);
        }
        else if (status.tag() == tags_.task_end) {
          ArchiveKey task_key;
          world_.recv(status.source(), status.tag(), task_key);

          tasks_ended_.push_back(task_key);
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

  std::list<ArchiveKey> const& ComputingUnitManager::get_tasks_ended() const {
    return tasks_ended_;
  }

  void ComputingUnitManager::clear_tasks_ended() {
    tasks_ended_.clear();
  }
#endif
}
