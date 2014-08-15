#include "test_mpi.hpp"
#include "test_computing_unit.hpp"

#include <gtest/gtest.h>

TEST(MPIComputingUnit, Execute) {
  MPIObjectArchive<int> archive(world);

  int mpi_tag = 20;
  TaskDistribution::BaseComputingUnit::mpi_tag = mpi_tag;

  if (world.rank() == 0) {
    TestComputingUnit unit(3);
    archive.insert(0, unit);

    int task_id = 123456;
    world.send(1, mpi_tag, 0);
    world.send(1, mpi_tag, task_id);
    world.send(1, mpi_tag, std::make_tuple(1));

    for (int i = 0; i < 1000; i++)
      archive.mpi_process();

    int received_task_id;
    world.recv(1, mpi_tag, received_task_id);
    EXPECT_EQ(task_id, received_task_id);

    int return_key;
    world.recv(1, mpi_tag, return_key);
    EXPECT_EQ(1, return_key);

    int received_return;
    archive.load(return_key, received_return);
    EXPECT_EQ(3, received_return);
  }
  else {
    TestComputingUnit unit;
    unit.execute(world, archive, 1);

    for (int i = 0; i < 1000; i++)
      archive.mpi_process();
  }
}
