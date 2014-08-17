#include "test_mpi.hpp"

#include "test_computing_unit.hpp"

#include "mpi_object_archive.hpp"

#include <gtest/gtest.h>

TEST(MPIComputingUnit, Execute) {
  MPIObjectArchive<TaskDistribution::ArchiveKey> archive(world);

  int mpi_tag = 20;

  TaskDistribution::TaskEntry task;

  if (world.rank() == 0) {
    task.task = {0, 1};
    task.computing_unit = {0, 2};
    task.arguments = {0, 3};
    task.result = {0, 4};
    task.computing_unit_id = "TestComputingUnit";

    archive.insert(task.computing_unit, TestComputingUnit(3));
    archive.insert(task.arguments, std::make_tuple(1));

    world.send(1, mpi_tag, task);

    // Arbitrary timing to fit data race
    for (int i = 0; i < 10000; i++)
      archive.mpi_process();

    int received_return = 0;
    archive.load(task.result, received_return);
    EXPECT_EQ(3, received_return);
  }
  else {
    world.recv(0, mpi_tag, task);
    TestComputingUnit unit;

    unit.execute(archive, task);

    // Arbitrary timing to fit data race
    for (int i = 0; i < 1000000; i++)
      archive.mpi_process();
  }
}
