#include "task_manager.hpp"

#include <gtest/gtest.h>

TEST(TaskManager, IdentityTask) {
  TaskDistribution::TaskManager task_manager;
  TaskDistribution::Task<int> task = task_manager.new_identity_task(1);
  TaskDistribution::Task<float> task3 = task_manager.new_identity_task(1);
  auto task2 = task_manager.new_identity_task(task);
  TaskDistribution::Task<float> task4 = task2;
  TaskDistribution::Task<float> task5 = task4;

  if (task5.is_same_task(task4))
    printf("asd\n");
  if (task5.is_same_task(task))
    printf("qwe\n");
}
