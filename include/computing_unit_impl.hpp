#ifndef __TASK_DISTRIBUTION__COMPUTING_UNIT_IMPL_HPP__
#define __TASK_DISTRIBUTION__COMPUTING_UNIT_IMPL_HPP__

#include "clean_tuple.hpp"
#include "repeated_tuple.hpp"

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

  template <std::size_t I, class T1, class T2>
  typename std::enable_if<(I == std::tuple_size<T1>::value ||
                           I == std::tuple_size<T2>::value), void>::type
  load_tasks_arguments_detail(T1&, T2 const&, ObjectArchive<Key>&,
      ComputingUnitManager&) { }

  template <std::size_t I, class T1, class T2>
  typename std::enable_if<(I < std::tuple_size<T1>::value &&
                           I < std::tuple_size<T2>::value), void>::type
  load_tasks_arguments_detail(T1& t1, T2 const& t2,
      ObjectArchive<Key>& archive, ComputingUnitManager& manager) {
    Key const& task_key = std::get<I>(t2);
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
      ObjectArchive<Key>& archive, ComputingUnitManager& manager) {
    load_tasks_arguments_detail<0>(t1,t2,archive, manager);
  }

  template <class T>
  void ComputingUnit<T>::execute(ObjectArchive<Key>& archive,
      TaskEntry const& task, ComputingUnitManager& manager) const {
    // Loads computing unit
    T obj;
    if (task.computing_unit_key.is_valid())
      archive.load(task.computing_unit_key, obj);

    // Loads arguments
    typename CompileUtils::clean_tuple_from_tuple<
      typename CompileUtils::function_traits<T>::arg_tuple_type>::type args;
    if (task.arguments_key.is_valid())
      archive.load(task.arguments_key, args);

    // Loads tasks arguments
    if (task.arguments_tasks_key.is_valid()) {
      typename CompileUtils::repeated_tuple<
        std::tuple_size<decltype(args)>::value, Key>::type tasks_tuple;
      archive.load(task.arguments_tasks_key, tasks_tuple);

      load_tasks_arguments(args, tasks_tuple, archive, manager);
    }

    // Performs the computation
    typename CompileUtils::function_traits<T>::return_type res(
      apply(obj, args,
        typename CompileUtils::sequence_generator<
        CompileUtils::function_traits<T>::arity>::type()));

    // Stores result, even if result_key.is_valid() == false
    archive.insert(task.result_key, res);
  }
};

#endif
