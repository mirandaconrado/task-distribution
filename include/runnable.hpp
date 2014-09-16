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

      void process();

    protected:
      ObjectArchive<Key>& archive_;
      TaskManager& task_manager_;

      po::variables_map vm;

      po::options_description cmd_args;
      po::options_description help_args;
      po::options_description invalidate_args;
  };
};

#endif
