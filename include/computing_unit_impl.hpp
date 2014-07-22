#ifndef __TASK_DISTRIBUTION__COMPUTING_UNIT_IMPL_HPP__
#define __TASK_DISTRIBUTION__COMPUTING_UNIT_IMPL_HPP__

#include "computing_unit.hpp"

#include "tuple_serialize.hpp"

#include <functional>

namespace TaskDistribution {
  template <class T>
  ComputingUnit<T>::ComputingUnit() {
    std::hash<std::string> hasher;
    id_ = hasher(typeid(T).name());

    // If we don't already have this kind of Callable, create a copy to store at
    // the map, so this one can be freed whenever the user chooses.
    if (map_.find(id_) == map_.end())
      map_[id_] = new ComputingUnit<T>(id_);
  }

#if !(NO_MPI)
  // In case of MPI, we must:
  // 1) receive the member data for the Callable;
  // 2) receive the task hash associated with this computation;
  // 3) receive the arguments provided during task creation;
  // 4) compute the result;
  // 5) send the task hash to link the result and task;
  // 6) send the result.
  template <class T>
  void ComputingUnit<T>::execute(boost::mpi::communicator& world) const {
    T obj;
    world.recv(0, 0, obj);

    size_t task_id;
    world.recv(0, 0, task_id);

    typename T::args_type args;
    world.recv(0, 0, args);

    typename T::result_type res;
    res = obj(args);

    world.send(0, 0, task_id);
    world.send(0, 0, res);
  }
#endif
};

#endif
