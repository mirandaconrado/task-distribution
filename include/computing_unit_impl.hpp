#ifndef __TASK_DISTRIBUTION__COMPUTING_UNIT_IMPL_HPP__
#define __TASK_DISTRIBUTION__COMPUTING_UNIT_IMPL_HPP__

#include "computing_unit.hpp"

#include "tuple_serialize.hpp"

#include <functional>

namespace TaskDistribution {
  template <class T>
  ComputingUnit<T>::ComputingUnit(std::string const& name) {
    // If we don't already have this kind of Callable, create a copy to store at
    // the map, so this one can be freed whenever the user chooses.
    if (map_.find(name) == map_.end()) {
      auto it = map_.emplace(name, new ComputingUnit<T>()).first;
      id_ = &it->first;
      it->second->id_ = id_;
    }
  }

  template <class F, class Tuple, size_t... S>
  typename function_traits<F>::return_type
  apply(F&& f, Tuple&& args, seq<S...>) {
    return std::forward<F>(f)(std::get<S>(std::forward<Tuple>(args))...);
  }

  template <class T>
  void ComputingUnit<T>::execute(ObjectArchive<ArchiveKey>& archive,
      TaskEntry const& task) const {
    T obj;
    archive.load(task.computing_unit, obj);

    typename function_traits<T>::arg_tuple_type args;
    archive.load(task.arguments, args);

    typename function_traits<T>::return_type res;
    res = apply(obj, args, typename gens<function_traits<T>::arity>::type());

    archive.insert(task.result, res);
  }
};

#endif
