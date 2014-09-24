// For better interface with the user and control of the tasks, a standard
// command line interface is defines in this file.
//
// A Runnable allows 4 kinds of commands:
// 1) check the list of tasks created;
// 2) clean the archive;
// 3) invalidate a given task set;
// 4) run the tasks.
//
// The "check" command only prints the status of each task type, with the number
// of tasks waiting execution and number of tasks finished.
//
// The "clean" command deletes everything from the archive that isn't used by
// the tasks provided. For instance, if some task isn't performed anymore, it
// may leave some debree inside the archive which occupies space, slows start-up
// and isn't useful. The command deletes these entries and re-arranges the keys
// to be occupy new free slots.
//
// The "invalidate" command MUST be used with an option "-i" to avoid mistakenly
// invalidating a task set. This command deletes every result of the set and
// every other task that depends on the original set. This should be used when
// the implementation of a task has changed.
//
// The "run" command just performs the computations, which can happen either
// with or without MPI.
//
// The current status of tasks is shown by commands "check", "clean" and
// "invalidate". The command "run" re-prints the table as each task is
// performed, allowing the user to keep track. The name used to print the table
// is the name associated with the computing unit.
//
// The user must inherit the class described here and provide:
// 1) the method "create_tasks()", which just creates all tasks to be computed;
// 2) the method "process_results()" is optional, and is called after all tasks
// are computed and every time the class is run.
//
// For an example on how to use it, check the file example/example.cpp.

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

      // Creates the tasks that must be computed.
      virtual void create_tasks() = 0;

      // Process the results of the tasks.
      virtual void process_results() { }

      // Interprets the command line arguments and returns 0 if everything was
      // OK.
      int process();

      // Creates the metadata displayed on the tasks table. This should be
      // called after create_tasks().
      void create_unit_map();

      // Prints the tasks status. This should be called after create_unit_map().
      void print_status();

      // Performs the check command.
      void check();

      // Performs the clean command.
      void clean();

      // Performs the invalidate command for a given unit name.
      void invalidate(std::string const& unit_name);

      // Performs the run command.
      void run();

    protected:
      // Handlers given to TaskManager to get feedback about what is happening
      // with the tasks.
      void task_creation_handler(std::string const& name, Key const& key);
      void task_begin_handler(Key const& key);
      void task_end_handler(Key const& key);

      // Removes everything from the archive that isn't used by the current
      // known tasks.
      void clean_tasks();

      // Invalidate all tasks of the given unit name.
      void invalidate_unit(std::string const& unit_name);

      // Removes the result associated with the given task and all its children.
      void remove_result(Key const& task_key);

      // Removes everything possible from a task and adds the rest to the set of
      // possible removals, as some data may be shared by multiple tasks.
      size_t remove_task(Key const& task_key, KeySet& possible_removals);

      // Remove everything from the set of possible removals that are used by
      // the known tasks.
      void clean_possible_removals(KeySet& possible_removals,
          KeySet const& created_tasks);

      // Changes keys to reduce sparsity.
      void relocate_keys();

      // Replaces a given key by another key in a task entry, described by the
      // task key.
      void replace_key(Key const& task_key, Key const& current_key,
          Key const& new_key);

      // Structure to hold every information required about a given unit.
      struct UnitEntry {
        UnitEntry(): waiting(0), running(0), finished(0) { }

        KeySet keys;
        // Number of tasks in each state.
        size_t waiting, running, finished;
      };

      // Map between unit names and their entries.
      std::map<std::string, UnitEntry> map_units_to_tasks_;

      ObjectArchive<Key>& archive_;
      TaskManager& task_manager_;

      // Command line interpretation variables.
      po::variables_map vm_;
      po::options_description cmd_args_;
      po::options_description help_args_;
      po::options_description invalidate_args_;
  };
};

#endif
