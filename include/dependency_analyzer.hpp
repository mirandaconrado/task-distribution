#ifndef __TASK_DISTRIBUTION__DEPENDENCY_ANALYZER__HPP__
#define __TASK_DISTRIBUTION__DEPENDENCY_ANALYZER__HPP__

#include <list>
#include <tuple>
#include <type_traits>

#include "archive_info.hpp"
#include "task.hpp"

namespace TaskDistribution {
  template <std::size_t I, class Tuple, class Analyzer>
  typename std::enable_if<(I == std::tuple_size<Tuple>::value), void>::type
  tuple_analyze_detail(Tuple const&, Analyzer&) { }

  template <std::size_t I, class Tuple, class Analyzer>
  typename std::enable_if<(I < std::tuple_size<Tuple>::value), void>::type
  tuple_analyze_detail(Tuple const& t, Analyzer& a) {
    a(std::get<I>(t));
    tuple_analyze_detail<I + 1>(t, a);
  }

  struct DependencyAnalyzer {
    std::list<ArchiveKey> dependencies;

    template <class T>
    void analyze(T const& tuple) {
      tuple_analyze_detail<0>(tuple, *this);
    }

    template <class T>
    void operator()(T const& v) const { }

    template <class T>
    void operator()(Task<T> const& v) const {
      dependencies.push_front(v.task_key_);
    }
  };
};

#endif
