#include "task_manager_mpi.hpp"

namespace TaskDistribution {
  MPITaskManager::MPITaskManager(boost::mpi::communicator& world,
      MPIHandler& handler, MPIObjectArchive<Key>& archive,
      MPIComputingUnitManager& unit_manager):
    MPITaskManager(Tags(), world, handler, archive, unit_manager) { }

  MPITaskManager::MPITaskManager(Tags const& tags,
      boost::mpi::communicator& world, MPIHandler& handler,
      MPIObjectArchive<Key>& archive, MPIComputingUnitManager& unit_manager):
    TaskManager(archive, unit_manager),
    tags_(tags),
    world_(world),
    handler_(handler),
    archive_(archive),
    unit_manager_(unit_manager),
    finished_(false) {
      handler.insert(tags_.finish,
          std::bind(&MPITaskManager::process_finish, this,
            std::placeholders::_1, tags.finish));

      clear_task_creation_handler();
      clear_task_begin_handler();
      clear_task_end_handler();

      archive_.set_insert_filter(
        [](Key const& key, boost::mpi::communicator& world)
        { return key.is_valid() && world.rank() == 0; });
    }

  MPITaskManager::~MPITaskManager() {
    if (world_.rank() == 0)
      broadcast_finish();
  }

  void MPITaskManager::run() {
    if (world_.size() > 1) {
      if (world_.rank() == 0)
        run_master();
      else
        run_slave();
    } else
      run_single();
  }

  void MPITaskManager::run_master() {
    std::vector<int> tasks_per_node(world_.size()-1, 0);

    size_t n_running = 0;

    // Process whatever is left for MPI first
    handler_.run();

    // Initial task allocation
    int index = 1;
    while (!ready_.empty()) {
      // Maximum fixed allocation. TODO: not fixed
      if (tasks_per_node[index-1] == 1)
        break;

      tasks_per_node[index-1]++;
      if (!send_next_task(index))
        break;
      ++index;
      ++n_running;
      if (index == world_.size())
        index = 1;
    }

    while (!ready_.empty() || n_running != 0) {
      // Process MPI stuff until a task has ended
      unit_manager_.clear_tasks_ended();

      do {
        handler_.run();
      } while (unit_manager_.get_tasks_ended().empty());

      MPIComputingUnitManager::TasksList const& finished_tasks =
        unit_manager_.get_tasks_ended();

      for (auto& it : finished_tasks) {
        task_completed(it.first);
        int slave = it.second;
        --tasks_per_node[slave-1];
        --n_running;
      }

      // Send new tasks to idle nodes
      for (auto& it : finished_tasks) {
        if (ready_.empty())
          break;
        int slave = it.second;
        if (send_next_task(slave)) {
          ++tasks_per_node[slave-1];
          ++n_running;
        }
      }
    }
  }

  void MPITaskManager::run_slave() {
    while (!finished_)
      unit_manager_.process_remote();
  }

  bool MPITaskManager::send_next_task(int slave) {
    bool got_task_for_remote = false;
    Key task_key;
    TaskEntry entry;

    while (!got_task_for_remote) {
      if (ready_.empty())
        return false;

      task_key = ready_.front();
      ready_.pop_front();

      archive_.load(task_key, entry);

      // If we already computed this task, gets the next one
      if (!entry.result_key.is_valid()) {
        if (entry.run_locally) {
          task_begin_handler_(task_key);
          unit_manager_.process_local(entry);
        }
        else
          got_task_for_remote = true;
      }
    }

    task_begin_handler_(task_key);
    unit_manager_.send_remote(entry, slave);
    return true;
  }

  bool MPITaskManager::process_finish(int source, int tag) {
    world_.recv(source, tag, finished_);

    return true;
  }

  void MPITaskManager::broadcast_finish() {
    for (int i = 1; i < world_.size(); i++)
      world_.send(i, tags_.finish, true);
  }

  size_t MPITaskManager::id() const {
    return world_.rank();
  }

  Key MPITaskManager::new_key(Key::Type type) {
    return Key::new_key(world_, type);
  }
};
