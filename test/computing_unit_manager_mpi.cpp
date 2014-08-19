#include "test_mpi.hpp"
#include "test_computing_unit.hpp"

#include "computing_unit_manager.hpp"

#include "mpi_object_archive.hpp"

#include <gtest/gtest.h>

TEST(ComputingUnitManager, ProcessRemote) {
  MPIObjectArchive<TaskDistribution::ArchiveKey> archive(world);
  TaskDistribution::ComputingUnitManager unit_manager(world, archive);
  TestComputingUnit unit;

  world.barrier();
  for (int i = 0; i < 10; i++)
    archive.mpi_process();
  world.barrier();

  TaskDistribution::TaskEntry task;
  task.task_key = {0, 1};
  task.computing_unit_key = {0, 2};
  task.arguments_key = {0, 3};
  task.computing_unit_id_key = {0, 4};

  if (world.rank() == 0) {
    archive.insert(task.task_key, task);
    archive.insert(task.computing_unit_key, TestComputingUnit(3));
    archive.insert(task.arguments_key, std::make_tuple(1));
    archive.insert(task.computing_unit_id_key, unit.get_id());

    unit_manager.send_remote(task, 1);
  }

  world.barrier();

  for (int i = 0; i < 100000; i++)
    unit_manager.process_remote();

  world.barrier();

  if (world.rank() == 0) {
    int received_return = 0;
    archive.load(task.task_key, task);
    archive.load(task.result_key, received_return);
    EXPECT_EQ(3, received_return);
  }
  else {
    // Arbitrary timing to fit data race
    for (int i = 0; i < 1000000; i++)
      archive.mpi_process();
  }

  world.barrier();
}
