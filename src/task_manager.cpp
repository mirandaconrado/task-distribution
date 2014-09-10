#include "task_manager.hpp"

namespace TaskDistribution {
#if ENABLE_MPI
  TaskManager::TaskManager(boost::mpi::communicator& world,
      MPIHandler& handler):
    TaskManager(Tags(), world, handler) { }

  TaskManager::TaskManager(Tags const& tags, boost::mpi::communicator& world,
      MPIHandler& handler):
    tags_(tags),
    world_(world),
    handler_(handler),
    archive_(world_, handler_,
        [](Key const& key, boost::mpi::communicator& world)
        { return key.is_valid() && world.rank() == 0; }),
    finished_(false),
    unit_manager_(world_, handler_, archive_) {
      handler.insert(tags_.finish,
          std::bind(&TaskManager::process_finish, this,
            std::placeholders::_1, tags.finish));

      clear_task_creation_handler();
      clear_task_begin_handler();
      clear_task_end_handler();
    }

  TaskManager::~TaskManager() {
    if (world_.rank() == 0)
      broadcast_finish();
  }
#else
  TaskManager::TaskManager():
    unit_manager_(archive_) { }

  TaskManager::~TaskManager() { }
#endif

  void TaskManager::run() {
#if ENABLE_MPI
    if (world_.size() > 1) {
      if (world_.rank() == 0)
        run_master();
      else
        run_slave();
    } else
#endif
      run_single();
  }

#if ENABLE_MPI
  void TaskManager::run_master() {
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

      ComputingUnitManager::TasksList const& finished_tasks =
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

  void TaskManager::run_slave() {
    while (!finished_)
      unit_manager_.process_remote();
  }

  bool TaskManager::send_next_task(int slave) {
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

  bool TaskManager::process_finish(int source, int tag) {
    world_.recv(source, tag, finished_);

    return true;
  }

  void TaskManager::broadcast_finish() {
    for (int i = 1; i < world_.size(); i++)
      world_.send(i, tags_.finish, true);
  }
#endif

  void TaskManager::run_single() {
    while (!ready_.empty()) {
      Key task_key = ready_.front();
      ready_.pop_front();
      TaskEntry entry;
      archive_.load(task_key, entry);
      task_begin_handler_(task_key);
      unit_manager_.process_local(entry);
      task_completed(task_key);
    }
  }

  void TaskManager::task_completed(Key const& task_key) {
    {
      auto it = map_task_to_children_.find(task_key);
      if (it != map_task_to_children_.end()) {
        for (auto& child_key: it->second) {
          TaskEntry child_entry;
          archive_.load(child_key, child_entry);
          child_entry.active_parents--;
          if (child_entry.active_parents == 0)
            ready_.push_back(child_key);
          archive_.insert(child_key, child_entry);
        }
      }
    }
    task_end_handler_(task_key);
  }

  size_t TaskManager::id() const {
#if ENABLE_MPI
    return world_.rank();
#else
    return 0;
#endif
  }

  void TaskManager::set_task_creation_handler(creation_handler_type handler) {
    task_creation_handler_ = handler;
  }

  void TaskManager::clear_task_creation_handler() {
    task_creation_handler_ = [](std::string const&, Key const&){};
  }

  void TaskManager::set_task_begin_handler(action_handler_type handler) {
    task_begin_handler_ = handler;
  }

  void TaskManager::clear_task_begin_handler() {
    task_begin_handler_ = [](Key const&){};
  }

  void TaskManager::set_task_end_handler(action_handler_type handler) {
    task_end_handler_ = handler;
  }

  void TaskManager::clear_task_end_handler() {
    task_end_handler_ = [](Key const&){};
  }

  Key TaskManager::new_key(Key::Type type) {
#if ENABLE_MPI
    return Key::new_key(world_, type);
#else
    return Key::new_key(type);
#endif
  }

  void TaskManager::create_family_lists(TaskEntry& entry) {
    if (!entry.parents_key.is_valid()) {
      entry.parents_key = new_key(Key::Parents);
      archive_.insert(entry.parents_key, KeySet());
    }

    if (!entry.children_key.is_valid()) {
      entry.children_key = new_key(Key::Children);
      archive_.insert(entry.children_key, KeySet());
    }
  }

  void TaskManager::add_dependency(TaskEntry& child_entry,
      Key const& parent_key, KeySet& parents) {
    TaskEntry parent_entry;
    archive_.load(parent_key, parent_entry);

    KeySet children;
    archive_.load(parent_entry.children_key, children);

    // Creates bidirectional maps
    map_task_to_parents_[child_entry.task_key].insert(parent_key);
    map_task_to_children_[parent_key].insert(child_entry.task_key);

    // Only add as active if it's a new parent
    if (parents.find(parent_key) == parents.end()) {
      parents.insert(parent_key);
      if (!parent_entry.result_key.is_valid() && parent_entry.should_save)
        child_entry.active_parents++;
    }

    children.insert(child_entry.task_key);

    archive_.insert(parent_entry.children_key, children);
  }
};
