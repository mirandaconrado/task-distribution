#include "test_mpi.hpp"

#include "test_computing_unit.hpp"

#include "mpi_object_archive.hpp"

#include <gtest/gtest.h>

TEST(MPIComputingUnit, Execute) {
  MPIObjectArchive<TaskDistribution::ArchiveKey> archive(world);
  TaskDistribution::ComputingUnitManager unit_manager(world, archive);

  int mpi_tag = 20;

  TestComputingUnit unit;

  TaskDistribution::TaskEntry task;

  if (world.rank() == 0) {
    task.task_key = {0, 1};
    task.computing_unit_key = {0, 2};
    task.arguments_key = {0, 3};
    task.result_key = {0, 4};
    task.computing_unit_id_key = {0, 5};

    archive.insert(task.computing_unit_key, TestComputingUnit(3));
    archive.insert(task.arguments_key, std::make_tuple(1));
    archive.insert(task.computing_unit_id_key, unit.get_id());

    world.send(1, mpi_tag, task);

    // Arbitrary timing to fit data race
    for (int i = 0; i < 10000; i++)
      archive.mpi_process();

    int received_return = 0;
    archive.load(task.result_key, received_return);
    EXPECT_EQ(3, received_return);
  }
  else {
    world.recv(0, mpi_tag, task);

    unit.execute(archive, task, unit_manager);

    // Arbitrary timing to fit data race
    for (int i = 0; i < 1000000; i++)
      archive.mpi_process();
  }

  world.barrier();
}

TEST(MPIComputingUnit, ExecuteWithTaskArgument) {
  MPIObjectArchive<TaskDistribution::ArchiveKey> archive(world);
  TaskDistribution::ComputingUnitManager unit_manager(world, archive);

  int mpi_tag = 20;

  TestComputingUnit unit;

  TaskDistribution::TaskEntry task, task_arg;

  if (world.rank() == 0) {
    task_arg.task_key = {0, 1};
    task_arg.result_key = {0, 2};

    archive.insert(task_arg.task_key, task_arg);
    archive.insert(task_arg.result_key, (int)1);

    task.task_key = {0, 3};
    task.computing_unit_key = {0, 4};
    task.arguments_tasks_key = {0, 5};
    task.result_key = {0, 6};
    task.computing_unit_id_key = {0, 7};

    archive.insert(task.computing_unit_key, TestComputingUnit(3));
    archive.insert(task.arguments_tasks_key,
        std::make_tuple(task_arg.task_key));
    archive.insert(task.computing_unit_id_key, unit.get_id());

    world.send(1, mpi_tag, task);

    // Arbitrary timing to fit data race
    for (int i = 0; i < 100000; i++)
      archive.mpi_process();

    int received_return = 0;
    archive.load(task.result_key, received_return);
    EXPECT_EQ(3, received_return);
  }
  else {
    world.recv(0, mpi_tag, task);
    TestComputingUnit unit;

    unit.execute(archive, task, unit_manager);

    // Arbitrary timing to fit data race
    for (int i = 0; i < 1000000; i++)
      archive.mpi_process();
  }

  world.barrier();
}

TEST(MPIComputingUnit, ExecuteWithIdentity) {
  MPIObjectArchive<TaskDistribution::ArchiveKey> archive(world);
  TaskDistribution::ComputingUnitManager unit_manager(world, archive);

  int mpi_tag = 20;

  TestComputingUnit unit;
  TaskDistribution::IdentityComputingUnit<int> identity_unit;

  TaskDistribution::TaskEntry task, task_identity;

  if (world.rank() == 0) {
    task_identity.task_key = {0, 1};
    task_identity.arguments_key = {0, 2};
    task_identity.computing_unit_id_key = {0, 3};
    task_identity.should_save = false;

    archive.insert(task_identity.task_key, task_identity);
    archive.insert(task_identity.arguments_key, std::make_tuple((int)1));
    archive.insert(task_identity.computing_unit_id_key, identity_unit.get_id());

    task.task_key = {0, 4};
    task.computing_unit_key = {0, 5};
    task.arguments_tasks_key = {0, 6};
    task.result_key = {0, 7};
    task.computing_unit_id_key = {0, 8};

    archive.insert(task.computing_unit_key, TestComputingUnit(3));
    archive.insert(task.arguments_tasks_key,
        std::make_tuple(task_identity.task_key));
    archive.insert(task.computing_unit_id_key, unit.get_id());

    world.send(1, mpi_tag, task);

    // Arbitrary timing to fit data race
    for (int i = 0; i < 100000; i++)
      archive.mpi_process();

    int received_return = 0;
    archive.load(task.result_key, received_return);
    EXPECT_EQ(3, received_return);
  }
  else {
    world.recv(0, mpi_tag, task);
    TestComputingUnit unit;

    unit.execute(archive, task, unit_manager);

    // Arbitrary timing to fit data race
    for (int i = 0; i < 1000000; i++)
      archive.mpi_process();
  }

  world.barrier();
}
