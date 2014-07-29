#include "task_manager.hpp"

namespace TaskDistribution {
  TaskManager::TaskManager():
    next_free_obj_id_(0) { }

  TaskManager::~TaskManager() {
    /*for (auto& it : hash_map_)
      delete it.second;*/
  }

  /*
  void TaskManager::add_free_task(BaseTask* task) {
    if (task->parents_active_ == 0 && !task->on_disk_ && task->should_save()) {
      bool include = true;
      for (auto &t: free_)
        // Compare pointers directly because they should have the same hash.
        if (t == task) {
          include = false;
          break;
        }

      if (include)
        free_.push_back(task);
    }
  }

  void TaskManager::run() {
#if !(NO_MPI)
    if (world_.size() > 1) {
      if (world_.rank() == 0)
        run_manager();
      else
        run_others();
    } else
#endif
      run_single();
  }

#if !(NO_MPI)
  void TaskManager::run_manager() {
    bool node_free[world_.size()-1];

    for (int i = 0; i < world_.size()-1; i++)
      node_free[i] = true;

    size_t n_running = 0;
    size_t current = 0;

    while (!free_.empty() || n_running != 0) {
      print_status();

      if (free_.empty() || n_running == world_.size()-1) {
        auto status = world_.probe();
        size_t task_hash;
        world_.recv(status.source(), 0, task_hash);

        BaseTask* task = hash_map_.at(task_hash);
        task->receive_value(world_, status.source());

        n_running--;
        node_free[status.source()-1] = true;
        current = status.source()-1;

        task_completed(task);
      }

      if (!free_.empty()) {
        BaseTask* t = free_.front();
        free_.pop_front();

        if (t->run_locally()) {
          t->compute();
          task_completed(t);
        } else {
          while (!node_free[current])
            current = (current+1) % (world_.size()-1);

          if (t->assign(world_, current+1)) {
            n_running++;
            node_free[current] = false;
          }
        }
      }
    }

    size_t invalid_hash = BaseComputingUnit::get_invalid_id();

    // Make other process end
    for (int i = 1; i < world_.size(); i++)
      world_.send(i, 0, invalid_hash);
  }

  void TaskManager::run_others() {
    while (1) {
      size_t callable_hash;
      world_.recv(0, 0, callable_hash);
      BaseComputingUnit const* callable = BaseComputingUnit::get_by_id(callable_hash);

      if (!callable)
        break;

      callable->execute(world_);
    }
  }
#endif

  void TaskManager::run_single() {
    while (!free_.empty()) {
      print_status();
      BaseTask* t = free_.front();
      free_.pop_front();
      t->compute();
      task_completed(t);
    }
  }

  void TaskManager::task_completed(BaseTask *task) {
    for (auto& it: task->children_) {
      it->parents_active_--;

      add_free_task(it);
    }

    for (auto& it: task->parents_) {
      it->children_active_--;

      if (it->children_active_ == 0)
        unload(it);
    }

    if (task->children_.empty())
        task->unload();

    if (task->should_save())
      count_map_.at(task->get_name()).second++;
  }

  void TaskManager::unload(BaseTask *task) {
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

  size_t TaskManager::id() const {
#if !(NO_MPI)
    return world_.rank();
#else
    return 0;
#endif
  }

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

  ArchiveKey TaskManager::new_object_key() {
    ArchiveKey key;
    key.node_id = id();
    key.obj_id = next_free_obj_id_++;
    return key;
  }
};
