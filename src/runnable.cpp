#include "runnable.hpp"

#include <functional>

namespace TaskDistribution {
  Runnable::Runnable(int argc, char* argv[], ObjectArchive<Key>& archive,
      TaskManager& task_manager):
    archive_(archive),
    task_manager_(task_manager),
    cmd_args_(""),
    help_args_(""),
    invalidate_args_("") {
      // Create command line arguments
      cmd_args_.add_options()
        ("command", po::value<std::string>(), "")
        ;

      help_args_.add_options()
        ("help,h", "show this help message")
        ;

      invalidate_args_.add_options()
        ("invalid,i", po::value<std::string>(), "kind of task to invalidate")
        ;

      po::positional_options_description p;
      p.add("command", 1);

      po::options_description all("Allowed options");
      all.add(cmd_args_).add(help_args_).add(invalidate_args_);

      po::store(po::command_line_parser(argc, argv).
          options(all).positional(p).run(), vm_);
      po::notify(vm_);

      // Set-up handlers
      task_manager_.set_task_creation_handler(
          std::bind(&Runnable::task_creation_handler, this,
            std::placeholders::_1, std::placeholders::_2));
      task_manager_.set_task_begin_handler(
          std::bind(&Runnable::task_begin_handler, this,
            std::placeholders::_1));
      task_manager_.set_task_end_handler(
          std::bind(&Runnable::task_end_handler, this,
            std::placeholders::_1));

      // Loads the archive
      task_manager_.load_archive();
    }

  int Runnable::process() {
    if (vm_.count("command")) {
      std::string cmd = vm_["command"].as<std::string>();

      if (cmd == "check") {
        if (vm_.count("help")) {
          po::options_description allowed("Allowed options");
          allowed.add(help_args_);
          std::cout << allowed << std::endl;
          return 1;
        }

        check();
        return 0;
      }

      if (cmd == "clean") {
        if (vm_.count("help")) {
          po::options_description allowed("Allowed options");
          allowed.add(help_args_);
          std::cout << allowed << std::endl;
          return 1;
        }

        clean();
        return 0;
      }

      if (cmd == "invalidate") {
        if (vm_.count("help") || !vm_.count("invalid")) {
          po::options_description allowed("Allowed options");
          allowed.add(help_args_).add(invalidate_args_);
          std::cout << allowed << std::endl;
          return 1;
        }

        invalidate(vm_["invalid"].as<std::string>());
        return 0;
      }

      if (cmd == "run") {
        if (vm_.count("help")) {
          po::options_description allowed("Allowed options");
          allowed.add(help_args_);
          std::cout << allowed << std::endl;
          return 1;
        }

        run();
        return 0;
      }


      std::cout << "Invalid command \"" << cmd << "\"!" << std::endl <<
        std::endl;
    }

    std::cout << "Choose one of the following commands:" << std::endl;
    std::cout << "  check        Check the status of tasks, displaying the"
      " number of each kind." << std::endl;
    std::cout << "  clean        Removes tasks results if the tasks don't exist"
      " anymore." << std::endl;
    std::cout << "  invalidate   Invalidates one kind of task." << std::endl;
    std::cout << "  run          Compute all tasks and process the results." <<
      std::endl;

    return 1;
  }

  void Runnable::create_unit_map() {
    std::list<Key const*> key_list = archive_.available_objects();
    std::vector<Key const*> key_vector(key_list.begin(), key_list.end());

    std::sort(key_vector.begin(), key_vector.end(),
        [](Key const* k1, Key const* k2) { return *k1 < *k2; });

    // For each task, registers whether it has a result or not
    for (auto& unit_entry : map_units_to_tasks_) {
      for (auto& task_key : unit_entry.second.keys) {
        TaskEntry task_entry;
        archive_.load(task_key, task_entry);
        if (task_entry.result_key.is_valid())
          unit_entry.second.finished++;
        else
          unit_entry.second.waiting++;
      }
    }
  }

  void Runnable::print_status() {
    size_t unit_name_len = 9, number_len = 8;

    // Finds the required width for the numbers
    for (auto& it : map_units_to_tasks_) {
      unit_name_len = std::max(unit_name_len, it.first.size());
      size_t len = 0, size = it.second.keys.size();

      while (size) {
        len += 1;
        size /= 10;
      }

      if (len == 0) len = 1;

      number_len = std::max(number_len, len);
    }

    // Default padding between fiels is 4 spaces
    size_t padding = 4;
    std::string padding_str(padding, ' ');

    // Line width
    size_t total_line_width = unit_name_len + number_len*3 + padding*3;

    // Prints header
    std::cout << "Unit name" <<
      std::string(unit_name_len-strlen("Unit name"),' ') << padding_str
      <<
      "Waiting" << std::string(number_len-strlen("Waiting"),' ') << padding_str
      <<
      "Finished" << std::string(number_len-strlen("Finished"),' ') << padding_str
      <<
      "Running" << std::endl;
    std::cout << std::string(total_line_width, '-') << std::endl;

    // Prints entries for each computing unit
    for (auto& it : map_units_to_tasks_) {
      char waiting_str[number_len+1], finished_str[number_len+1],
           running_str[number_len+1];
      sprintf(waiting_str, "%*lu", (int)number_len, it.second.waiting);
      sprintf(finished_str, "%*lu", (int)number_len, it.second.finished);
      sprintf(running_str, "%*lu", (int)number_len, it.second.running);

      std::cout << it.first << std::string(unit_name_len-it.first.length(),' ');
      std::cout << padding_str << waiting_str;
      std::cout << padding_str << finished_str;
      std::cout << padding_str << running_str;
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

  void Runnable::check() {
    if (task_manager_.id() != 0)
      return;

    create_tasks();
    create_unit_map();
    print_status();
  }

  void Runnable::clean() {
    if (task_manager_.id() != 0)
      return;

    create_tasks();
    clean_tasks();
    create_unit_map();
    print_status();
  }

  void Runnable::invalidate(std::string const& unit_name) {
    if (unit_name == "identity") {
      printf("Can't invalidate identity tasks.\n");
      return;
    }

    if (task_manager_.id() != 0)
      return;

    create_tasks();
    invalidate_unit(unit_name);
    create_unit_map();
    print_status();
  }

  void Runnable::run() {
    create_tasks();
    create_unit_map();

    task_manager_.run();

    if (task_manager_.id() == 0)
      process_results();
  }

  void Runnable::task_creation_handler(std::string const& name,
      Key const& key) {
    map_units_to_tasks_[name].keys.insert(key);
  }

  void Runnable::task_begin_handler(Key const& key) {
    // Finds the task and move it from waiting to running
    for (auto& unit_it : map_units_to_tasks_) {
      for (auto& it : unit_it.second.keys)
        if (it == key) {
          unit_it.second.waiting--;
          unit_it.second.running++;
          print_status();
          return;
        }
    }
  }

  void Runnable::task_end_handler(Key const& key) {
    // Finds the task and move it from running to finished
    for (auto& unit_it : map_units_to_tasks_) {
      for (auto& it : unit_it.second.keys)
        if (it == key) {
          unit_it.second.running--;
          unit_it.second.finished++;
          print_status();
          return;
        }
    }
  }

  void Runnable::clean_tasks() {
    // Gets the tasks associated with all computing units
    KeySet created_tasks;
    for (auto& it : map_units_to_tasks_)
      created_tasks.insert(it.second.keys.begin(), it.second.keys.end());

    // Gets all tasks in archive
    KeySet available_keys;
    for (auto& it : archive_.available_objects())
      if (it->type == Key::Task)
        available_keys.insert(*it);

    size_t tasks_removed = 0;
    KeySet possible_removals;

    auto task_key_it = created_tasks.begin();
    auto task_key_end = created_tasks.end();
    auto archive_key_it = available_keys.begin();
    auto archive_key_end = available_keys.end();

    while (task_key_it != task_key_end && archive_key_it != archive_key_end) {
      if (*task_key_it < *archive_key_it)
        ++task_key_it;
      else if (*task_key_it > *archive_key_it) {
        // As both sets are ordered, if the archive moved slower, it must have
        // an old task
        tasks_removed += remove_task(*archive_key_it, possible_removals);
        ++archive_key_it;
      }
      else {
        ++task_key_it;
        ++archive_key_it;
      }
    }

    // Makes sure tasks at the end are removed also
    while (archive_key_it != archive_key_end) {
      tasks_removed += remove_task(*archive_key_it, possible_removals);
      ++archive_key_it;
    }

    // Removes from the set of possible stuff to remove if they are used by the
    // created tasks
    clean_possible_removals(possible_removals, created_tasks);

    printf("Removed %lu tasks and %lu other entries.\n\n", tasks_removed,
        possible_removals.size());

    // Removes every entry possible
    for (auto& removal : possible_removals)
      archive_.remove(removal);

    // Fills the empty places of old keys
    relocate_keys();
  }

  void Runnable::invalidate_unit(std::string const& unit_name) {
    if (map_units_to_tasks_.find(unit_name) == map_units_to_tasks_.end()) {
      printf("No tasks with name \"%s\" found!", unit_name.c_str());
      return;
    }

    UnitEntry& unit_entry = map_units_to_tasks_[unit_name];

    // Removes result of each task associated with the unit
    for (auto& task_key : unit_entry.keys)
      remove_result(task_key);
  }

  void Runnable::remove_result(Key const& task_key) {
    TaskEntry entry;
    archive_.load(task_key, entry);

    // If result is invalid, children must also have invalid result
    if (entry.result_key.is_valid()) {
      archive_.remove(entry.result_key);
      entry.result_key = Key();
      archive_.insert(task_key, entry);

      if (entry.children_key.is_valid()) {
        KeySet children;
        archive_.load(entry.children_key, children);
        for (auto& child_key : children)
          remove_result(child_key);
      }
    }
  }

  size_t Runnable::remove_task(Key const& task_key,
      std::set<Key>& possible_removals) {
    if (!archive_.is_available(task_key))
      return 0;

    TaskEntry entry;
    archive_.load(task_key, entry);

    size_t tasks_removed = 0;

    if (entry.computing_unit_key.is_valid())
      possible_removals.insert(entry.computing_unit_key);

    if (entry.arguments_key.is_valid())
      possible_removals.insert(entry.arguments_key);

    if (entry.arguments_tasks_key.is_valid())
      possible_removals.insert(entry.arguments_tasks_key);

    if (entry.result_key.is_valid())
      archive_.remove(entry.result_key);

    if (entry.computing_unit_id_key.is_valid())
      possible_removals.insert(entry.computing_unit_id_key);

    if (entry.parents_key.is_valid())
      archive_.remove(entry.parents_key);

    if (entry.children_key.is_valid()) {
      KeySet children;
      archive_.load(entry.children_key, children);
      for (auto& child_key : children)
        tasks_removed += remove_task(child_key, possible_removals);
      archive_.remove(entry.children_key);
    }

    archive_.remove(task_key);

    return tasks_removed + 1;
  }

  void Runnable::clean_possible_removals(KeySet& possible_removals,
      KeySet const& created_tasks) {
    for (auto& task_key : created_tasks) {
      TaskEntry entry;
      archive_.load(task_key, entry);
      possible_removals.erase(entry.computing_unit_key);
      possible_removals.erase(entry.arguments_key);
      possible_removals.erase(entry.arguments_tasks_key);
      possible_removals.erase(entry.computing_unit_id_key);
    }
  }

  void Runnable::relocate_keys() {
    // Gets all keys used and which tasks use them
    std::map<Key, KeySet> map_key_to_task_key;
    KeySet used_keys;
    for (auto& it : archive_.available_objects()) {
      if (it->type == Key::Task) {
        Key const& task_key = *it;
        used_keys.insert(task_key);

        TaskEntry entry;
        archive_.load(task_key, entry);

#define CHECK(key) \
        if (entry.key.is_valid()) { \
          used_keys.insert(entry.key); \
          map_key_to_task_key[entry.key].insert(task_key); \
        }

        CHECK(computing_unit_key);
        CHECK(arguments_key);
        CHECK(arguments_tasks_key);
        CHECK(result_key);
        CHECK(computing_unit_id_key);
        CHECK(parents_key);
        CHECK(children_key);
      }
    }

    size_t last_obj_id = 0; // One less than an avaliable key
    size_t current_node_id = 0;

    auto used_keys_it = used_keys.begin();
    auto used_keys_end = used_keys.end();
    while (used_keys_it != used_keys_end) {
      // Checks if changed node id
      if (used_keys_it->node_id != current_node_id) {
        current_node_id = used_keys_it->node_id;
        last_obj_id = 0;
      }

      // Tasks can't be moved
      if (used_keys_it->type == Key::Task) {
        ++used_keys_it;
        continue;
      }

      // If there's a gap
      if (used_keys_it->obj_id - last_obj_id > 1) {
        // If the key is already in use, continue
        if (used_keys.find(Key({current_node_id, last_obj_id+1, Key::Task})) !=
            used_keys.end()) {
          last_obj_id++;
          continue;
        }

        // Moves the data from the old key to the new key
        Key const& current_key = *used_keys_it;
        Key new_key({current_node_id, last_obj_id+1, current_key.type});
        archive_.change_key(current_key, new_key);

        // Replaces the key in evey task entry used by it
        for (auto& task_key : map_key_to_task_key.at(current_key))
          replace_key(task_key, current_key, new_key);

        auto used_keys_it_bak = used_keys_it;

        last_obj_id++;
        ++used_keys_it;

        // Changes the key in the list of used ones
        used_keys.erase(used_keys_it_bak);
        used_keys.insert(new_key);
      }
      else
        ++used_keys_it;
    }
  }

  void Runnable::replace_key(Key const& task_key, Key const& current_key,
      Key const& new_key) {
    TaskEntry entry;
    archive_.load(task_key, entry);

#define CHANGE_KEY(key) \
    if (entry.key == current_key) \
      entry.key = new_key;

    CHANGE_KEY(computing_unit_key);
    CHANGE_KEY(arguments_key);
    CHANGE_KEY(arguments_tasks_key);
    CHANGE_KEY(result_key);
    CHANGE_KEY(computing_unit_id_key);
    CHANGE_KEY(parents_key);
    CHANGE_KEY(children_key);

    archive_.insert(task_key, entry);
  }
};
