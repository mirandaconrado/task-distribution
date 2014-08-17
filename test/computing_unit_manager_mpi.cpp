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
  task.task = {0, 1};
  task.computing_unit = {0, 2};
  task.arguments = {0, 3};
  task.result = {0, 4};
  task.computing_unit_id = "TestComputingUnit";

  if (world.rank() == 0) {
    archive.insert(task.task, task);
    archive.insert(task.computing_unit, TestComputingUnit(3));
    archive.insert(task.arguments, std::make_tuple(1));

    unit_manager.send_remote(task, 1);
  }

  world.barrier();

  if (world.rank() == 0) {
    // Arbitrary timing to fit data race
    for (int i = 0; i < 100000; i++)
      archive.mpi_process();
  }
  else
    for (int i = 0; i < 100; i++) {
      archive.mpi_process();
      unit_manager.process_remote();
    }

  world.barrier();

  if (world.rank() == 0) {
    int received_return = 0;
    archive.load(task.result, received_return);
    EXPECT_EQ(3, received_return);
  }
  else {
    // Arbitrary timing to fit data race
    for (int i = 0; i < 1000000; i++)
      archive.mpi_process();
  }
}
