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
    finished_(false),
    tasks_per_node_(world_.size()-1, 0) {
      handler.insert(tags_.finish,
          std::bind(&MPITaskManager::process_finish, this,
            std::placeholders::_1, tags.finish));

      handler.insert(tags_.key_update,
          std::bind(&MPITaskManager::process_key_update, this,
            std::placeholders::_1, tags.key_update));

      clear_task_creation_handler();
      clear_task_begin_handler();
      clear_task_end_handler();

      archive_.set_insert_filter(
        [](Key const& key, boost::mpi::communicator& world)
        { return key.is_valid() && world.rank() == 0; });
    }

  MPITaskManager::~MPITaskManager() { }

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
    size_t n_running = 0;

    // Process whatever is left for MPI first
    handler_.run();

    n_running += allocate_tasks();

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
        --tasks_per_node_[slave-1];
        --n_running;
      }

      n_running += allocate_tasks();
    }

    broadcast_finish();
  }

  size_t MPITaskManager::allocate_tasks() {
    size_t n_running = 0;

    bool allocated_a_task = true;

    while (!ready_.empty() && allocated_a_task) {
      allocated_a_task = false;

      for (int i = 1; i < world_.size(); i++) {
        // Maximum fixed allocation. TODO: not fixed
        if (tasks_per_node_[i-1] < 1) {

          // Tries to send task to slave. Fails if ready_.empty() after running
          // local tasks.
          if (!send_next_task(i))
            break;

          allocated_a_task = true;
          tasks_per_node_[i-1]++;
          n_running++;
        }
      }
    }

    return n_running;
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
          task_completed(task_key);
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

  bool MPITaskManager::process_key_update(int source, int tag) {
    size_t key;
    world_.recv(source, tag, key);
    Key::next_obj = std::max(Key::next_obj, key);

    return true;
  }

  void MPITaskManager::broadcast_finish() {
    for (int i = 1; i < world_.size(); i++)
      world_.send(i, tags_.finish, true);
  }

  size_t MPITaskManager::id() const {
    return world_.rank();
  }

  void MPITaskManager::update_used_keys(
      std::map<int, size_t> const& used_keys) {
    if (world_.size() == 1)
      TaskManager::update_used_keys(used_keys);
    else {
      for (auto it = used_keys.begin(); it != used_keys.end(); ++it) {
        if (it->first == world_.rank())
          Key::next_obj = std::max(Key::next_obj, it->second + 1);
        else
          world_.send(it->first, tags_.key_update, it->second + 1);
      }
    }
  }

  Key MPITaskManager::new_key(Key::Type type) {
    return Key::new_key(world_, type);
  }
};
