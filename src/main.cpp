#include "task_manager.hpp"

TaskDistribution::TaskManager task_manager;

class Multiply:
  public TaskDistribution::ComputingUnit<Multiply> {
  public:
    Multiply(): ComputingUnit<Multiply>("multiply") { }

    virtual bool run_locally() const {
      return true;
    }

    virtual bool should_save() const {
      return true;
    }

    float operator()(float arg1, float arg2) const {
      printf("called with %f and %f\n", arg1, arg2);
      return arg1 * arg2;
    }
};

class Fibonacci:
  public TaskDistribution::ComputingUnit<Fibonacci> {
  public:
    Fibonacci(): ComputingUnit<Fibonacci>("fibonacci") {}

    virtual bool run_locally() const {
      return true;
    }

    virtual bool should_save() const {
      return true;
    }

    int operator()(int n) const {
      if (n <= 1)
        return 1;

      printf("called with n=%d\n", n);
      auto param_1 = task_manager.new_task(Fibonacci(), n-1);
      auto param_2 = task_manager.new_task(Fibonacci(), n-2);

      return param_1 + param_2;
    }
};

class Fibonacci2:
  public TaskDistribution::ComputingUnit<Fibonacci2> {
  public:
    Fibonacci2(): ComputingUnit<Fibonacci2>("fibonacci2") {}

    virtual bool run_locally() const {
      return true;
    }

    virtual bool should_save() const {
      return true;
    }

    int operator()(int v1, int v2) const {
      printf("called with v1=%d and v2=%d\n", v1, v2);

      return v1 + v2;
    }
};

TaskDistribution::Task<int> create_fibonacci(int n) {
  if (n <= 1)
    return task_manager.new_identity_task(1);

  return task_manager.new_task(Fibonacci2(),
      create_fibonacci(n-1), create_fibonacci(n-2));
}

int main() {
  /*printf("creating task1\n");
  auto task_1 = task_manager.new_task(Multiply(), 1, 2);
  printf("creating task2\n");
  auto task_2 = task_manager.new_task(Multiply(), 1, 2);
  printf("creating task3\n");
  auto task_3 = task_manager.new_task(Multiply(), task_1, task_2);
  printf("%f\n", task_3());*/

  int n = 10;
  auto task_4 = task_manager.new_task(Fibonacci(), n);
  TaskDistribution::Task<int> task_5 = create_fibonacci(n);
  task_manager.run();

  printf("%d\n", task_4());
  printf("%d\n", task_5());

  return 0;
}
