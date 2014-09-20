#ifndef __TASK_DISTRIBUTION__RUNNABLE_HPP__
#define __TASK_DISTRIBUTION__RUNNABLE_HPP__

#include <boost/program_options.hpp>

#include "object_archive.hpp"
#include "task_manager.hpp"

namespace po = boost::program_options;

namespace TaskDistribution {
  class Runnable {
    public:
      Runnable(int argc, char* argv[], ObjectArchive<Key>& archive,
          TaskManager& task_manager);

      virtual void create_tasks() = 0;
      virtual void process_results() { }

      int process();
      void print_status();

      void check();
      void clean();
      void run();

      void update_unit_map();

    protected:
      void task_creation_handler(std::string const& name, Key const& key);
      void task_begin_handler(Key const& key);
      void task_end_handler(Key const& key);

      void clean_tasks();
      size_t remove_task(Key const& task_key,
          std::set<Key>& possible_removals);

      struct UnitEntry {
        UnitEntry(): waiting(0), running(0), finished(0) { }

        KeySet keys;
        size_t waiting, running, finished;
      };

      std::map<std::string, UnitEntry> map_units_to_tasks_;

      ObjectArchive<Key>& archive_;
      TaskManager& task_manager_;

      po::variables_map vm;

      po::options_description cmd_args;
      po::options_description help_args;
      po::options_description invalidate_args;
  };
};

#endif
