#include "task_manager.hpp"


class Multiply:
  public TaskDistribution::ComputingUnit<Multiply> {
  public:
    Multiply(): ComputingUnit<Multiply>("multiply") { }

    virtual bool run_locally() const {
      return true;
    }

    virtual bool should_save() const {
      return false;
    }

    float operator()(float arg1, float arg2) const {
      return arg1 * arg2;
    }
};

int main() {
  TaskDistribution::TaskManager task_manager;
  printf("creating task1\n");
  auto task_1 = task_manager.new_task(Multiply(), 1, 2);
  printf("creating task2\n");
  auto task_2 = task_manager.new_task(Multiply(), 1, 2);
  printf("creating task3\n");
  auto task_3 = task_manager.new_task(Multiply(), task_1, task_2);
  return task_3;
}
