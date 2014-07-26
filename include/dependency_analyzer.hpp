#ifndef __TASK_DISTRIBUTION__DEPENDENCY_ANALYZER__HPP__
#define __TASK_DISTRIBUTION__DEPENDENCY_ANALYZER__HPP__

#include <list>

#include "task.hpp"

namespace TaskDistribution {
  struct DependencyAnalyzer {
    mutable std::list<BaseTask*> dependencies;

    template <class T>
    void operator()(T const& v) const { }

    template <class T>
    void operator()(Task<T> const& v) const {
      dependencies.push_front(v.task);
    }
  };
};

#endif
