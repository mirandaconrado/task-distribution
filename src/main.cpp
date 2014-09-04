#include "task_manager.hpp"

#if ENABLE_MPI
boost::mpi::environment env;
boost::mpi::communicator world;
MPIHandler handler(world);
TaskDistribution::TaskManager task_manager(world, handler);
#else
TaskDistribution::TaskManager task_manager;
#endif

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

    int operator()(int v1, int v2) const {
      return v1 + v2;
    }
};

TaskDistribution::Task<int> create_fibonacci(int n) {
  if (n <= 1)
    return task_manager.new_identity_task(1);

  return task_manager.new_task(Fibonacci(),
      create_fibonacci(n-1), create_fibonacci(n-2));
}

int main() {
  int n = 10;
  TaskDistribution::Task<int> task = create_fibonacci(n);
  task_manager.run();

  if (world.rank() == 0) {
    printf("result = %d\n", task());
  }

  return 0;
}
