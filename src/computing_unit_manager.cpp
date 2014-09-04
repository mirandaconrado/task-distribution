#include "computing_unit_manager.hpp"

#include "computing_unit.hpp"

namespace TaskDistribution {
#if ENABLE_MPI
  ComputingUnitManager::ComputingUnitManager(boost::mpi::communicator& world,
      MPIHandler& handler, MPIObjectArchive<Key>& archive):
    ComputingUnitManager(Tags(), world, handler, archive) { }

  ComputingUnitManager::ComputingUnitManager(Tags const& tags,
      boost::mpi::communicator& world, MPIHandler& handler,
      MPIObjectArchive<Key>& archive):
    world_(world),
    handler_(handler),
    tags_(tags),
    archive_(archive) {
      handler.insert(tags_.task_begin,
          std::bind(&ComputingUnitManager::process_task_begin, this,
            std::placeholders::_1, tags.task_begin));
      handler.insert(tags_.task_end,
          std::bind(&ComputingUnitManager::process_task_end, this,
            std::placeholders::_1, tags.task_end));
    }
#else
  ComputingUnitManager::ComputingUnitManager(
      ObjectArchive<Key>& archive):
    archive_(archive) { }
#endif

  void ComputingUnitManager::process_local(TaskEntry& task) {
    if (task.result_key.is_valid())
      return;

    if (task.should_save) {
#if ENABLE_MPI
      task.result_key = Key::new_key(world_, Key::Result);
#else
      task.result_key = Key::new_key(Key::Result);
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

    if (task.should_save)
      archive_.insert(task.task_key, task);
  }

#if ENABLE_MPI
  void ComputingUnitManager::process_remote() {
    while (1) {
      handler_.run();

      if (tasks_requested_.empty())
        break;

      auto task_description = tasks_requested_.front();
      tasks_requested_.pop_front();

      process_local(task_description.first);

      world_.isend(task_description.second, tags_.task_end,
          task_description.first.task_key);
    }
  }

  void ComputingUnitManager::send_remote(TaskEntry const& task, int remote) {
    world_.isend(remote, tags_.task_begin, task);
  }

  ComputingUnitManager::TasksList const&
  ComputingUnitManager::get_tasks_ended() const {
    return tasks_ended_;
  }

  void ComputingUnitManager::clear_tasks_ended() {
    tasks_ended_.clear();
  }

  bool ComputingUnitManager::process_task_begin(int source, int tag) {
    TaskEntry task;
    world_.recv(source, tag, task);

    tasks_requested_.push_back(std::make_pair(task, source));

    return true;
  }

  bool ComputingUnitManager::process_task_end(int source, int tag) {
    Key task_key;
    world_.recv(source, tag, task_key);

    tasks_ended_.push_back(std::make_pair(task_key, source));

    return true;
  }
#endif
}
