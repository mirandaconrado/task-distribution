#include "runnable.hpp"

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
    }

  void Runnable::process() {
    if (vm.count("command")) {
      std::string cmd = vm["command"].as<std::string>();

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
  }
};
