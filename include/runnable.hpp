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

      int process();
      void print_status();

      void check();

    protected:
      void task_creation_handler(std::string const& name, Key const& key);
      void task_begin_handler(Key const& key);
      void task_end_handler(Key const& key);

      ObjectArchive<Key>& archive_;
      TaskManager& task_manager_;

      po::variables_map vm;

      po::options_description cmd_args;
      po::options_description help_args;
      po::options_description invalidate_args;
  };
};

#endif
