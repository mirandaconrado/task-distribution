#include "test_computing_unit.hpp"

#include "computing_unit_manager.hpp"

#include <gtest/gtest.h>

TEST(ComputingUnitManager, ProcessLocal) {
  ObjectArchive<TaskDistribution::ArchiveKey> archive;
  TaskDistribution::ComputingUnitManager unit_manager(archive);

  TestComputingUnit unit(3);
  archive.insert({0, 0}, unit);

  for (size_t i = 1; i <= 10; i++) {
    TaskDistribution::TaskEntry task;
    task.task = {0, i};
    task.computing_unit = {0, 0};
    archive.insert({0, i+10}, std::make_tuple((int)i));
    task.arguments = {0, i+10};
    task.result = {0, i+20};
    task.computing_unit_id = unit.get_id();
    archive.insert(task.task, task);
  }

  for (size_t i = 1; i <= 10; i++) {
    TaskDistribution::TaskEntry task;
    archive.load({0, i}, task);
    unit_manager.process_local(task);
  }

  for (size_t i = 1; i <= 10; i++) {
    int result = 0;
    archive.load({0, i+20}, result);
    EXPECT_EQ(3*i, result);
  }
}
