#ifndef __TASK_DISTRIBUTION__TUPLE_CONVERT__HPP__
#define __TASK_DISTRIBUTION__TUPLE_CONVERT__HPP__

#include "is_tuple_convertible.hpp"
#include "sequence.hpp"

#include <tuple>
#include <type_traits>

namespace TaskDistribution {
  template <class T1, class T2, size_t... S>
  void tuple_convert_detail(T1& t1, T2 const& t2,
      CompileUtils::sequence<S...>) {
    t1 = T1(std::get<S>(t2)...);
  }

  template <class T1, class T2>
  typename std::enable_if< CompileUtils::is_tuple_convertible<T1,T2>::value,
           void>::type
  tuple_convert(T1& t1, T2 const& t2) {
    tuple_convert_detail(t1, t2,
        CompileUtils::tuple_sequence_generator<T1>::type());
  }
};

#endif
