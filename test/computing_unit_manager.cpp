#include "test_computing_unit.hpp"

#include "computing_unit_manager.hpp"

#include <gtest/gtest.h>

TEST(ComputingUnitManager, ProcessLocal) {
  ObjectArchive<TaskDistribution::ArchiveKey> archive;
  TaskDistribution::ComputingUnitManager unit_manager(archive);

  TestComputingUnit unit(3);
  TaskDistribution::ArchiveKey unit_key =
    TaskDistribution::ArchiveKey::new_key(
        TaskDistribution::ArchiveKey::ComputingUnit);
  archive.insert(unit_key, unit);

  TaskDistribution::ArchiveKey unit_id_key =
    TaskDistribution::ArchiveKey::new_key(
        TaskDistribution::ArchiveKey::ComputingUnitId);
  archive.insert(unit_id_key, unit.get_id());

  std::vector<TaskDistribution::TaskEntry> tasks;

  for (size_t i = 0; i < 5; i++) {
    TaskDistribution::TaskEntry task;
    task.task_key =
      TaskDistribution::ArchiveKey::new_key(TaskDistribution::ArchiveKey::Task);
    task.computing_unit_key = unit_key;
    task.arguments_key =
      TaskDistribution::ArchiveKey::new_key(TaskDistribution::ArchiveKey::Arguments);
    task.computing_unit_id_key = unit_id_key;

    archive.insert(task.task_key, task);
    archive.insert(task.arguments_key, std::make_tuple((int)i));

    tasks.push_back(task);
  }

  for (size_t i = 0; i < 5; i++)
    unit_manager.process_local(tasks[i]);

  for (size_t i = 0; i < 5; i++) {
    int result = 0;
    archive.load(tasks[i].result_key, result);
    EXPECT_EQ(3*i, result);
  }
}
