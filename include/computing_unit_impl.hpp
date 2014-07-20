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

    if (map_.find(id_) == map_.end())
      map_[id_] = new ComputingUnit<T>(id_);
  }

#if !(NO_MPI)
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
