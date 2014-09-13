#include "task_manager_mpi.hpp"

#include <unistd.h>

boost::mpi::environment env;
boost::mpi::communicator world;
MPIHandler handler(world);
MPIObjectArchive<TaskDistribution::Key> archive(world, handler);
TaskDistribution::MPIComputingUnitManager unit_manager(world, handler, archive);
TaskDistribution::MPITaskManager task_manager(world, handler, archive,
    unit_manager);


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
      printf("%d\n", world.rank());
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

TaskDistribution::Task<double> create_factorial(int n) {
  if (n <= 1)
    return task_manager.new_identity_task(1.);

  Factorial f;
  f.n = n;

  return task_manager.new_task(f, create_factorial(n-1));
}

int main() {
  int n = 10;
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

  TaskDistribution::Task<double> task = create_factorial(n);

  task_manager.run();

  if (world.rank() == 0) {
    //printf("result = %d\n", task());
    printf("result = %f\n", task());
  }

  return 0;
}
