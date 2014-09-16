#include "runnable.hpp"

#include <functional>

namespace TaskDistribution {
  Runnable::Runnable(int argc, char* argv[], ObjectArchive<Key>& archive,
      TaskManager& task_manager):
    archive_(archive),
    task_manager_(task_manager),
    cmd_args(""),
    help_args(""),
    invalidate_args("") {
      cmd_args.add_options()
        ("command", po::value<std::string>(), "")
        ;

      help_args.add_options()
        ("help,h", "show this help message")
        ;

      invalidate_args.add_options()
        ("invalid,i", po::value<std::string>(), "kind of task to invalidate")
        ;

      po::positional_options_description p;
      p.add("command", 1);

      po::options_description all("Allowed options");
      all.add(cmd_args).add(help_args).add(invalidate_args);

      po::store(po::command_line_parser(argc, argv).
          options(all).positional(p).run(), vm);
      po::notify(vm);

      task_manager_.set_task_creation_handler(
          std::bind(&Runnable::task_creation_handler, this,
            std::placeholders::_1, std::placeholders::_2));
      task_manager_.set_task_begin_handler(
          std::bind(&Runnable::task_begin_handler, this,
            std::placeholders::_1));
      task_manager_.set_task_end_handler(
          std::bind(&Runnable::task_end_handler, this,
            std::placeholders::_1));
    }

  int Runnable::process() {
    if (vm.count("command")) {
      std::string cmd = vm["command"].as<std::string>();

      if (cmd == "check") {
        if (vm.count("help")) {
          po::options_description allowed("Allowed options");
          allowed.add(help_args);
          std::cout << allowed << std::endl;
          return 1;
        }

        check();
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

  void Runnable::print_status() {
    for (auto it : map_units_to_tasks_) {
      printf("%s : %lu/%lu\n", it.first.c_str(), it.second.waiting, it.second.finished);
    }
  }

  void Runnable::check() {
    if (task_manager_.id() != 0)
      return;

    create_tasks();
    update_unit_map();
    print_status();
  }

  void Runnable::update_unit_map() {
    std::list<Key const*> key_list = archive_.available_objects();
    std::vector<Key const*> key_vector(key_list.begin(), key_list.end());

    std::sort(key_vector.begin(), key_vector.end(),
        [](Key const* k1, Key const* k2) { return *k1 < *k2; });

    for (auto& unit_it : map_units_to_tasks_) {
      auto unit_key_it = unit_it.second.keys.begin();
      auto unit_key_end = unit_it.second.keys.end();
      auto archive_key_it = key_vector.begin();
      auto archive_key_end = key_vector.end();
      while (unit_key_it != unit_key_end && archive_key_it != archive_key_end) {
        if (*unit_key_it < **archive_key_it)
          ++unit_key_it;
        else if (*unit_key_it > **archive_key_it)
          ++archive_key_it;
        else {
          TaskEntry task_entry;
          archive_.load(*unit_key_it, task_entry);
          if (task_entry.result_key.is_valid())
            unit_it.second.finished++;
          else
            unit_it.second.waiting++;

          ++unit_key_it;
          ++archive_key_it;
        }
      }
    }
  }

  void Runnable::task_creation_handler(std::string const& name,
      Key const& key) {
    map_units_to_tasks_[name].keys.insert(key);

  }
  void Runnable::task_begin_handler(Key const& key) {
  }
  void Runnable::task_end_handler(Key const& key) {
  }
};
