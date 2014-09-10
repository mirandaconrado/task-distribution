// If each time of a tuple can be converted to its associated type in another
// tuple, performs this conversion.

#ifndef __TASK_DISTRIBUTION__TUPLE_CONVERT__HPP__
#define __TASK_DISTRIBUTION__TUPLE_CONVERT__HPP__

#include "is_tuple_convertible.hpp"
#include "sequence.hpp"

#include <tuple>
#include <type_traits>

namespace TaskDistribution {
  template <class To, class From, size_t... S>
  void tuple_convert_detail(To& to, From const& from,
      CompileUtils::sequence<S...>) {
    to = To(std::get<S>(from)...);
  }

  template <class To, class From>
  typename std::enable_if<CompileUtils::is_tuple_convertible<To,From>::value,
           void>::type
  tuple_convert(To& to, From const& from) {
    tuple_convert_detail(to, from,
        CompileUtils::tuple_sequence_generator<To>::type());
  }
};

#endif
