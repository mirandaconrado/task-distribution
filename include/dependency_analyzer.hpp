// Tasks may depend on other tasks, which is reflected by their argument tuples.
// This file defines an auxiliary structure to get these dependencies.
//
// The class just iterates over the tuple of keys and, if a valid key is found,
// add as a dependency.

#ifndef __TASK_DISTRIBUTION__DEPENDENCY_ANALYZER__HPP__
#define __TASK_DISTRIBUTION__DEPENDENCY_ANALYZER__HPP__

#include <tuple>
#include <type_traits>

#include "key.hpp"
#include "task.hpp"

namespace TaskDistribution {
  template <size_t I, class Tuple, class Analyzer>
  typename std::enable_if<(I == std::tuple_size<Tuple>::value), void>::type
  tuple_analyze_detail(Tuple const&, Analyzer&) { }

  template <size_t I, class Tuple, class Analyzer>
  typename std::enable_if<(I < std::tuple_size<Tuple>::value), void>::type
  tuple_analyze_detail(Tuple const& t, Analyzer& a) {
    Key const& key = std::get<I>(t);
    if (key.is_valid())
      a.dependencies.push_back(key);
    tuple_analyze_detail<I + 1>(t, a);
  }

  struct DependencyAnalyzer {
    KeyList dependencies;

    template <class T>
    void analyze(T const& tuple) {
      tuple_analyze_detail<0>(tuple, *this);
    }
  };
};

#endif
