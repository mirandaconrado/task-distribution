#include "test_mpi.hpp"
#include "test_computing_unit.hpp"

#include <gtest/gtest.h>

TEST(MPIComputingUnit, Execute) {
  if (world.rank() == 0) {
    TestComputingUnit unit(3);
    size_t task_id = 123456;
    world.send(1, 0, unit);
    world.send(1, 0, task_id);
    world.send(1, 0, std::make_tuple(1));

    int received_return;
    size_t received_task_id;
    world.recv(1, 0, received_task_id);
    world.recv(1, 0, received_return);

    EXPECT_EQ(task_id, received_task_id);
    EXPECT_EQ(received_return, 3);
  }
  else {
    TestComputingUnit unit;
    unit.execute(world);
  }
}
