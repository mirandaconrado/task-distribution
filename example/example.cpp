#include "runnable.hpp"

#if ENABLE_MPI
#include "task_manager_mpi.hpp"
#else
#include "task_manager.hpp"
#endif

class FibonacciPrint:
  public TaskDistribution::ComputingUnit<FibonacciPrint> {
  public:
    FibonacciPrint(): ComputingUnit<FibonacciPrint>("fibonacci_print") {}

    virtual bool run_locally() const {
      return true;
    }

    int operator()(int v) const {
      printf("result_fib = %d\n", v);
      return 0;
    }
};

class Fibonacci:
  public TaskDistribution::ComputingUnit<Fibonacci> {
  public:
    Fibonacci(): ComputingUnit<Fibonacci>("fibonacci") {}

    virtual bool run_locally() const {
      return true;
    }

    int operator()(int v1, int v2) const {
      printf("fibonacci\tv1 = %d\tv2 = %d\n", v1, v2);
      return v1 + v2;
    }
};

class Factorial:
  public TaskDistribution::ComputingUnit<Factorial> {
  public:
    Factorial(): ComputingUnit<Factorial>("factorial") {}

    double n;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & n;
    }

    virtual bool run_locally() const {
      return false;
    }

    double operator()(double v) const {
      printf("factorial\tn = %f\tv = %f\n", n, v);
      return n * v;
    }
};

TaskDistribution::Task<int> create_fibonacci(int n,
    TaskDistribution::TaskManager* task_manager) {
  if (n <= 1)
    return task_manager->new_identity_task(1);

  return task_manager->new_task(Fibonacci(),
      create_fibonacci(n-1, task_manager),
      create_fibonacci(n-2, task_manager));
}

TaskDistribution::Task<double> create_factorial(int n,
    TaskDistribution::TaskManager* task_manager) {
  if (n <= 1)
    return task_manager->new_identity_task(1.);

  Factorial f;
  f.n = n;

  return task_manager->new_task(f, create_factorial(n-1, task_manager));
}

class FactorialRunnable: public TaskDistribution::Runnable {
  public:
    template <class... Args>
      FactorialRunnable(Args&&... args):
        Runnable(std::forward<Args>(args)...) { }

    void create_tasks() {
      int n = 4;
      result_fact = create_factorial(n, &task_manager_);
      task_manager_.new_task(FibonacciPrint(),
          create_fibonacci(n, &task_manager_));
    }

    void process_results() {
      printf("result_fact = %f\n", result_fact());
    }

    TaskDistribution::Task<double> result_fact;
};

int main(int argc, char* argv[]) {
#if ENABLE_MPI
  boost::mpi::environment env(argc, argv);
  boost::mpi::communicator world;

  MPIHandler handler(world);
  MPIObjectArchive<TaskDistribution::Key> archive(world, handler);
  archive.init("example.archive");
  TaskDistribution::MPIComputingUnitManager unit_manager(world, handler,
      archive);
  TaskDistribution::MPITaskManager task_manager(world, handler, archive,
      unit_manager);
#else
  ObjectArchive<TaskDistribution::Key> archive;
  archive.init("example.archive");
  TaskDistribution::ComputingUnitManager unit_manager(archive);
  TaskDistribution::TaskManager task_manager(archive, unit_manager);
#endif

  FactorialRunnable runnable(argc, argv, archive, task_manager);

  runnable.process();

  return 0;
}
