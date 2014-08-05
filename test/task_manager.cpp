#include "task_manager.hpp"

#include <gtest/gtest.h>

TEST(TaskManager, IdentityTask) {
  TaskDistribution::TaskManager task_manager;
  auto task = task_manager.new_identity_task(1);
  TaskDistribution::Task<float> task3 = task_manager.new_identity_task(1);
  auto task2 = task_manager.new_identity_task(task);
  TaskDistribution::Task<float> task4 = task2;
}
