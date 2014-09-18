#include "runnable.hpp"
#include "task_manager_mpi.hpp"

#include <unistd.h>

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

    virtual bool should_save() const {
      return true;
    }

    double operator()(double v) const {
      printf("n = %f\tv=%f\n", n, v);
//      sleep(1);
      return n * v;
    }
};

/*TaskDistribution::Task<int> create_fibonacci(int n) {
  if (n <= 1)
    return task_manager.new_identity_task(1);

  return task_manager.new_task(Fibonacci(),
      create_fibonacci(n-1), create_fibonacci(n-2));
}*/

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
      int n = 5;
      result = create_factorial(n, &task_manager_);
    }

    void process_results() {
      printf("result = %f\n", result());
    }

    TaskDistribution::Task<double> result;
};

int main(int argc, char* argv[]) {
  boost::mpi::environment env(argc, argv);
  boost::mpi::communicator world;

  MPIHandler handler(world);
  MPIObjectArchive<TaskDistribution::Key> archive(world, handler);
  TaskDistribution::MPIComputingUnitManager unit_manager(world, handler,
      archive);
  TaskDistribution::MPITaskManager task_manager(world, handler, archive,
      unit_manager);
  FactorialRunnable runnable(argc, argv, archive, task_manager);

  runnable.process();

  /*TaskDistribution::Task<double> task;
  //int n = 10;
  int n = 2;

  {
    TaskDistribution::MPITaskManager task_manager(world, handler, archive,
        unit_manager);
    task_manager.load_archive();

    //TaskDistribution::Task<int> task = create_fibonacci(n);

    task_manager.set_task_creation_handler(
        [](std::string const& name, TaskDistribution::Key const& key) {
        printf("Creating \"%s\" with (%lu,%lu)\n", name.c_str(), key.node_id,
          key.obj_id);
        });

    task_manager.set_task_begin_handler(
        [](TaskDistribution::Key const& key) {
        printf("Begin (%lu,%lu)\n", key.node_id, key.obj_id);
        });

    task_manager.set_task_end_handler(
        [](TaskDistribution::Key const& key) {
        printf("End (%lu,%lu)\n", key.node_id, key.obj_id);
        });

    task = create_factorial(n, &task_manager);
  }

  world.barrier();
  handler.clear(TaskDistribution::MPITaskManager::Tags().finish);
  handler.clear(TaskDistribution::MPITaskManager::Tags().key_update);

  n = 3;

  {
    TaskDistribution::MPITaskManager task_manager(world, handler, archive,
        unit_manager);
    task_manager.load_archive();

    task_manager.set_task_creation_handler(
        [](std::string const& name, TaskDistribution::Key const& key) {
        printf("Creating \"%s\" with (%lu,%lu)\n", name.c_str(), key.node_id,
          key.obj_id);
        });

    task_manager.set_task_begin_handler(
        [](TaskDistribution::Key const& key) {
        printf("Begin (%lu,%lu)\n", key.node_id, key.obj_id);
        });

    task_manager.set_task_end_handler(
        [](TaskDistribution::Key const& key) {
        printf("End (%lu,%lu)\n", key.node_id, key.obj_id);
        });

    task = create_factorial(n, &task_manager);

    task_manager.run();
  }

  n = 4;

  world.barrier();
  handler.clear(TaskDistribution::MPITaskManager::Tags().finish);
  handler.clear(TaskDistribution::MPITaskManager::Tags().key_update);

  {
    TaskDistribution::MPITaskManager task_manager(world, handler, archive,
        unit_manager);
    task_manager.load_archive();

    task_manager.set_task_creation_handler(
        [](std::string const& name, TaskDistribution::Key const& key) {
        printf("Creating \"%s\" with (%lu,%lu)\n", name.c_str(), key.node_id,
          key.obj_id);
        });

    task_manager.set_task_begin_handler(
        [](TaskDistribution::Key const& key) {
        printf("Begin (%lu,%lu)\n", key.node_id, key.obj_id);
        });

    task_manager.set_task_end_handler(
        [](TaskDistribution::Key const& key) {
        printf("End (%lu,%lu)\n", key.node_id, key.obj_id);
        });

    task = create_factorial(n, &task_manager);

    task_manager.run();
  }

  world.barrier();

  if (world.rank() == 0) {
    //printf("result = %d\n", task());
    printf("result = %f\n", task());
  }
  */

  return 0;
}
