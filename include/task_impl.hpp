#ifndef __TASK_DISTRIBUTION__TASK_IMPL_HPP__
#define __TASK_DISTRIBUTION__TASK_IMPL_HPP__

#include "task.hpp"

#include "task_manager.hpp"

namespace TaskDistribution {
  template <class T>
  Task<T>::operator T() const {
    BOOST_ASSERT_MSG(task_key_.is_valid(), "invalid task key");
    T ret;
    task_manager_->get_result(task_key_, ret);
    return ret;
  }
};

#endif
