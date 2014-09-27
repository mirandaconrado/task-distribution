#include "computing_unit_manager_mpi.hpp"

#include "computing_unit.hpp"

namespace TaskDistribution {
  MPIComputingUnitManager::MPIComputingUnitManager(
      boost::mpi::communicator& world, MPIHandler& handler,
      MPIObjectArchive<Key>& archive):
    MPIComputingUnitManager(Tags(), world, handler, archive) { }

  MPIComputingUnitManager::MPIComputingUnitManager(Tags const& tags,
      boost::mpi::communicator& world, MPIHandler& handler,
      MPIObjectArchive<Key>& archive):
    ComputingUnitManager(archive),
    world_(world),
    handler_(handler),
    tags_(tags) {
      // Set-up handlers
      handler.insert(tags_.task_begin,
          std::bind(&MPIComputingUnitManager::process_task_begin, this,
            std::placeholders::_1, tags.task_begin));
      handler.insert(tags_.task_end,
          std::bind(&MPIComputingUnitManager::process_task_end, this,
            std::placeholders::_1, tags.task_end));
    }

  void MPIComputingUnitManager::process_remote() {
    while (1) {
      handler_.run();

      // Returns if no more tasks are required
      if (tasks_requested_.empty())
        break;

      // Gets the first task and processes it
      auto task_description = tasks_requested_.front();
      tasks_requested_.pop_front();

      process_local(task_description.first);

      // Send information back to requester saying the task has finished
      world_.isend(task_description.second, tags_.task_end,
          task_description.first.task_key);
    }
  }

  void MPIComputingUnitManager::send_remote(TaskEntry const& task, int remote) {
    world_.isend(remote, tags_.task_begin, task);
  }

  MPIComputingUnitManager::TasksList const&
  MPIComputingUnitManager::get_tasks_ended() const {
    return tasks_ended_;
  }

  void MPIComputingUnitManager::clear_tasks_ended() {
    tasks_ended_.clear();
  }

  bool MPIComputingUnitManager::process_task_begin(int source, int tag) {
    TaskEntry task;
    world_.recv(source, tag, task);

    tasks_requested_.push_back(std::make_pair(task, source));

    return true;
  }

  bool MPIComputingUnitManager::process_task_end(int source, int tag) {
    Key task_key;
    world_.recv(source, tag, task_key);

    tasks_ended_.push_back(std::make_pair(task_key, source));

    return true;
  }

  Key MPIComputingUnitManager::new_key(Key::Type type) {
    return Key::new_key(world_, type);
  }
}
