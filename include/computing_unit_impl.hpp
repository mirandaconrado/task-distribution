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
    if (id_map_.find(name) == id_map_.end()) {
      ComputingUnit<T>* unit = new ComputingUnit<T>();
      auto it = id_map_.emplace(name, unit).first;
      id_ = &it->first;
      unit->id_ = id_;
    }
    id_ = &id_map_.find(name)->first;
  }

  template <class F, class Tuple, size_t... S>
  typename function_traits<F>::return_type
  apply(F&& f, Tuple&& args, seq<S...>) {
    return std::forward<F>(f)(std::get<S>(std::forward<Tuple>(args))...);
  }

  template <std::size_t I, class T1, class T2>
  typename std::enable_if<(I == std::tuple_size<T1>::value ||
                           I == std::tuple_size<T2>::value), void>::type
  load_tasks_arguments_detail(T1&, T2 const&, ObjectArchive<ArchiveKey>&,
      ComputingUnitManager&) { }

  template <std::size_t I, class T1, class T2>
  typename std::enable_if<(I < std::tuple_size<T1>::value &&
                           I < std::tuple_size<T2>::value), void>::type
  load_tasks_arguments_detail(T1& t1, T2 const& t2,
      ObjectArchive<ArchiveKey>& archive, ComputingUnitManager& manager) {
    ArchiveKey const& task_key = std::get<I>(t2);
    if (task_key.is_valid()) {
      TaskEntry entry;
      archive.load(task_key, entry);

      // If key is invalid, then compute locally the result and store at the
      // invalid position
      if (!entry.should_save || !entry.result_key.is_valid())
        manager.process_local(entry);

      // Loads result even is key is invalid, as it contains the temporary
      // result
      typename std::tuple_element<I, T1>::type result;
      archive.load(entry.result_key, result);

      // Removes the result if it was used only temporarily
      if (!entry.should_save || !entry.result_key.is_valid())
        archive.remove(entry.result_key);

      std::get<I>(t1) = result;
    }

    load_tasks_arguments_detail<I + 1>(t1, t2, archive, manager);
  }

  template <class T1, class T2>
  typename std::enable_if<std::tuple_size<T1>::value ==
                          std::tuple_size<T2>::value, void>::type
  load_tasks_arguments(T1& t1, T2 const& t2,
      ObjectArchive<ArchiveKey>& archive, ComputingUnitManager& manager) {
    load_tasks_arguments_detail<0>(t1,t2,archive, manager);
  }

  template <class T>
  void ComputingUnit<T>::execute(ObjectArchive<ArchiveKey>& archive,
      TaskEntry const& task, ComputingUnitManager& manager) const {
    // Loads computing unit
    T obj;
    if (task.computing_unit_key.is_valid())
      archive.load(task.computing_unit_key, obj);

    // Loads arguments
    typename function_traits<T>::arg_tuple_type args;
    if (task.arguments_key.is_valid())
      archive.load(task.arguments_key, args);

    // Loads tasks arguments
    if (task.arguments_tasks_key.is_valid()) {
      typename repeated_tuple<std::tuple_size<decltype(args)>::value,
               ArchiveKey>::type tasks_tuple;
      archive.load(task.arguments_tasks_key, tasks_tuple);

      load_tasks_arguments(args, tasks_tuple, archive, manager);
    }

    // Performs the computation
    typename function_traits<T>::return_type res;
    res = apply(obj, args, typename gens<function_traits<T>::arity>::type());

    // Stores result, even if result_key.is_valid() == false
    archive.insert(task.result_key, res);
  }
};

#endif
