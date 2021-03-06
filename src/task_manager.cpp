#include "task_manager.hpp"

namespace TaskDistribution {
  TaskManager::TaskManager(ObjectArchive<Key>& archive,
      ComputingUnitManager& unit_manager):
    archive_(archive),
    unit_manager_(unit_manager) { }

  TaskManager::~TaskManager() { }

  void TaskManager::run() {
    run_single();
  }

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
    return 0;
  }

  std::string TaskManager::load_string_to_hash(Key const& key) {
    std::string data_str;
    if (key.type != Key::Task) {
      archive_.load_raw(key, data_str);
    }
    else {
      TaskEntry entry;
      archive_.load(key, entry);
      // Special case of tasks different from the identity
      if (entry.computing_unit_id_key.is_valid()) {
        entry.result_key = Key();
        entry.parents_key = Key();
        entry.children_key = Key();
        entry.active_parents = 0;
      }
      entry.task_key = Key();

      data_str = ObjectArchive<Key>::serialize(entry);
    }

    return data_str;
  }

  void TaskManager::update_used_keys(std::map<int, size_t> const& used_keys) {
    auto it = used_keys.find(id());
    if (it != used_keys.end())
      Key::next_obj = std::max(Key::next_obj, it->second + 1);
  }

  void TaskManager::load_archive() {
    if (archive_.available_objects().empty())
      return;

    if (id() != 0)
      return;

    std::hash<std::string> hasher;

    // Keeps track of keys used inside the archive to avoid collision
    std::map<int, size_t> used_keys;

    for (auto key : archive_.available_objects()) {
      if (!key->is_valid())
        continue;

      used_keys[key->node_id] = std::max(used_keys[key->node_id], key->obj_id);

      std::string data_str = load_string_to_hash(*key);
      if (data_str == "")
        continue;
      size_t hash = hasher(data_str);
      map_hash_to_key_.emplace(hash, *key);
    }

    update_used_keys(used_keys);
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
    return Key::new_key(type);
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

    // Creates map used to check if tasks are ready to run
    map_task_to_children_[parent_key].insert(child_entry.task_key);

    // Only add as active if it's a new parent
    if (parents.find(parent_key) == parents.end()) {
      parents.insert(parent_key);
      if (!parent_entry.result_key.is_valid())
        child_entry.active_parents++;
    }

    children.insert(child_entry.task_key);

    archive_.insert(parent_entry.children_key, children);
  }
};
