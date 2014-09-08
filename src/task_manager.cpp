#include "task_manager.hpp"

namespace TaskDistribution {
  /*TaskManager::TaskManager():
    next_free_obj_id_(1) { }

  TaskManager::~TaskManager() {
    for (auto& it : hash_map_)
      delete it.second;
  }

  void TaskManager::check_if_ready(ArchiveKey const& task_key) {
    TaskEntry task_entry;
    archive_.load(task_key, task_entry, true);

    if (task_entry.result.is_valid() || !task_entry.should_save)
      return;

    BaseTask *task;
    archive_.load(task_entry.task, task, true);

    if (task->parents_active_ == 0)
        tasks_ready_to_run_.insert(task_key);

    delete task;
  }*/
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
        run_manager();
      else
        run_others();
    } else
#endif
      run_single();
  }

#if ENABLE_MPI
  void TaskManager::run_manager() {
    std::vector<int> tasks_per_node(world_.size()-1, 0);

    size_t n_running = 0;
    //size_t current = 0;

    // Process whatever is left for MPI first
    handler_.run();

    // Initial task allocation
    int index = 1;
    while (!ready_.empty()) {
      // Maximum fixed allocation. TODO: not fixed
      if (tasks_per_node[index-1] == 1)
        break;

      tasks_per_node[index-1]++;
      //      printf("tasks %d\n", tasks_per_node[index-1]);
      if (!send_next_task(index))
        break;
      ++index;
      ++n_running;
      if (index == world_.size())
        index = 1;
    }

    while (!ready_.empty() || n_running != 0) {
      //print_status();

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

  void TaskManager::run_others() {
    while (!finished_) {
      unit_manager_.process_remote();
      /*size_t callable_hash;
      world_.recv(0, 0, callable_hash);
      BaseComputingUnit const* callable = BaseComputingUnit::get_by_id(callable_hash);

      if (!callable)
        break;

      callable->execute(world_);*/
    }
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
        if (entry.run_locally)
          unit_manager_.process_local(entry);
        else
          got_task_for_remote = true;
      }
    }

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
      //print_status();
      Key task_key = ready_.front();
      ready_.pop_front();
      TaskEntry entry;
      archive_.load(task_key, entry);
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

    //if (task->should_save())
    //  count_map_.at(task->get_name()).second++;
  }

  /*void TaskManager::unload(BaseTask *task) {
    task->unload();

    for (auto& it: task->parents_) {
      it->children_active_--;
      if (it->children_active_ == 0)
        unload(it);
    }
  }

  BaseTask* TaskManager::get(size_t hash) const {
    if (hash_map_.find(hash) != hash_map_.end())
      return hash_map_.at(hash);

    return nullptr;
  }

  void TaskManager::insert(size_t hash, std::string name, BaseTask* t) {
    hash_map_[hash] = t;
    name_map_[name].push_back(hash);

    if (t->should_save()) {
      if (count_map_.find(name) == count_map_.end())
        count_map_[name] = std::pair<size_t,size_t>(1, check(t));
      else {
        count_map_[name].first++;
        count_map_[name].second += check(t);
      }
    }
  }

  bool TaskManager::check(BaseTask* t) const {
    return archive_.is_available(t->get_id());
  }*/

  /*size_t TaskManager::id() const {
#if ENABLE_MPI
    return world_.rank();
#else
    return 0;
#endif
  }*/

  /*void TaskManager::print_status() {
    time_t current = time(NULL);

    if (current == last_print_)
      return;

    last_print_ = current;

    size_t descriptor_len = 9, number_len = 8;

    for (auto& it: count_map_) {
      descriptor_len = std::max(descriptor_len, it.first.length());
      size_t len = 0, size = it.second.first;

      while (size) {
        len += 1;
        size /= 10;
      }

      if (len == 0) len = 1;

      number_len = std::max(number_len, len);
    }

    size_t padding = 4;
    std::string padding_str(padding, ' ');

    size_t total_line_width = descriptor_len + number_len*2 + padding*2;

    std::cout << "Task name" <<
      std::string(descriptor_len-strlen("Task name"),' ') << padding_str <<
      " Waiting" << padding_str << "Finished" << std::endl;
    std::cout << std::string(total_line_width, '-') << std::endl;

    for (auto& it: count_map_) {
      size_t finished = it.second.second;
      size_t waiting = it.second.first - finished;

      char waiting_str[number_len], finished_str[number_len];
      sprintf(waiting_str, "%*lu", (int)number_len, waiting);
      sprintf(finished_str, "%*lu", (int)number_len, finished);

      std::cout << it.first << std::string(descriptor_len-it.first.length(),' ');
      std::cout << padding_str << waiting_str;
      std::cout << padding_str << finished_str;
      std::cout << std::endl;
    }

    std::cout << std::endl;
  }

  void TaskManager::invalidate(std::string name) {
    if (name_map_.find(name) == name_map_.end())
      return;

    for (auto& hash: name_map_.at(name))
      invalidate(get(hash));
  }

  void TaskManager::invalidate(BaseTask* task) {
    if (task->on_disk_) {
      task->on_disk_ = false;
      size_t id = task->get_id();
      archive_.remove(id);

      count_map_.at(task->get_name()).second--;
    }

    // Invalidate all others that depend on this one
    for (auto& it: task->children_)
      invalidate(it);
  }

  void TaskManager::clean() {
    archive_.clear();
  }*/

  /*ArchiveKey TaskManager::new_object_key() {
    ArchiveKey key;
    key.node_id = id();
    key.obj_id = next_free_obj_id_++;
    return key;
  }

  ArchiveKey TaskManager::get_task(std::string const& unit_key,
      std::string const& args_key,
      std::string const& unit_str,
      std::string const& args_str) {
    std::string map_key = unit_key + args_key;

    ArchiveKey task_key;
    bool found_previous_task = false;

    if (map_typenames_to_tasks_.count(map_key) > 0) {
      auto range = map_typenames_to_tasks_.equal_range(map_key);
      for (auto it = range.first;
          it != range.second && !found_previous_task;
          ++it) {
        task_key = it->second;
        TaskEntry task_entry;
        archive_.load(task_key, task_entry, false);

        std::string data;

        archive_.load_raw(task_entry.computing_unit, data, false);
        if (data != unit_str)
          continue;

        archive_.load_raw(task_entry.arguments, data, false);
        if (data != args_str)
          continue;

        found_previous_task = true;
      }
    }

    if (!found_previous_task) {
      task_key = new_object_key();
      TaskEntry task_entry;

      task_entry.computing_unit =
        get_component(unit_key, unit_str, map_unit_names_to_units_);
      task_entry.arguments =
        get_component(args_key, args_str, map_arg_names_to_args_);

      archive_.insert(task_key, task_entry, true);
      map_typenames_to_tasks_.insert({map_key, task_key});
    }

    return task_key;
  }

  ArchiveKey TaskManager::get_component(std::string const& map_key,
          std::string const& str,
          std::unordered_multimap<std::string, ArchiveKey>& map) {
    ArchiveKey key;
    bool found_previous = false;

    if (map.count(map_key) > 0) {
      auto range = map.equal_range(map_key);
      for (auto it = range.first;
          it != range.second && !found_previous;
          ++it) {
        key = it->second;

        std::string data;

        archive_.load_raw(key, data, false);
        if (data != str)
          continue;

        found_previous = true;
      }
    }

    if (!found_previous) {
      key = new_object_key();
      archive_.insert_raw(key, str, false);
      map.insert({map_key, key});
    }

    return key;
  }*/

  Key TaskManager::new_key(Key::Type type) {
    return
#if ENABLE_MPI
      Key::new_key(world_, type);
#else
      Key::new_key(type);
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
