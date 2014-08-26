#ifndef __TASK_DISTRIBUTION__TASK_IMPL_HPP__
#define __TASK_DISTRIBUTION__TASK_IMPL_HPP__

#include "task.hpp"

#include "task_manager.hpp"

/*#include "computing_unit.hpp"
#include "task_manager.hpp"
#include "tuple_convert.hpp"

#include <boost/functional/hash.hpp>*/

namespace TaskDistribution {
  /*template<class Archive>
  void BaseTask::serialize(Archive& ar, const unsigned int version) {
    ar & parents_active_;
    ar & parents_;
    ar & children_active_;
    ar & children_;
    ar & task_key_;
  }*/
/*  template <class Unit, class Args>
  boost::any RealTask<Unit,Args>::call() {
    if (on_memory_ && result_ != nullptr)
      return result_;

    if (on_disk_) {
      if (result_ != nullptr)
        delete result_;

      result_ = new typename Unit::result_type();
      on_memory_ = task_manager_->load(this, *result_);

      if (on_memory_)
        return result_;
    }

    compute();

    return result_;
  }

  template <class Unit, class Args>
  RealTask<Unit,Args>*
  RealTask<Unit,Args>::get(TaskManager* task_manager, Unit const& unit,
      Args const& args) {
    size_t id = 0;
    boost::hash_combine(id, std::string(typeid(Args).name()));
    boost::hash_combine(id, args);
    boost::hash_combine(id, std::string(typeid(Unit).name()));
    boost::hash_combine(id, unit);

    if (task_manager->get(id) == nullptr)
      task_manager->insert(id, Unit::name,
          new RealTask<Unit,Args>(task_manager, unit, args, id));

    return static_cast<RealTask<Unit,Args>*>(task_manager->get(id));
  }

  template <class Unit, class Args>
  RealTask<Unit,Args>::
  RealTask(TaskManager* task_manager, Unit const& unit, Args const& args,
      size_t id):
    BaseTask(id, task_manager),
    computing_unit_(unit),
    args_(args),
    result_(nullptr) {
    if (task_manager_->check(this))
      on_disk_ = true;
  }

  template <class Unit, class Args>
  RealTask<Unit,Args>::~RealTask() {
    if (result_ != nullptr)
      delete result_;
  }

#if !(NO_MPI)
  template <class Unit, class Args>
  bool RealTask<Unit,Args>::assign(boost::mpi::communicator& world,
      size_t node) {
    if (on_memory_ || on_disk_)
      return false;

    world.send(node, 0, ComputingUnit<Unit>().get_id());
    world.send(node, 0, computing_unit_);
    world.send(node, 0, id_);

    typename Unit::args_type new_args;
    tuple_convert(new_args, args_);
    world.send(node, 0, new_args);
    return true;
  }

  template <class Unit, class Args>
  void RealTask<Unit,Args>::receive_result(boost::mpi::communicator& world,
      size_t node) {
    if (result_ != nullptr)
      delete result_;

    result_ = new typename Unit::result_type();

    world.recv(node, 0, *result_);

    on_memory_ = true;
    if (computing_unit_.should_save())
      on_disk_ = task_manager_->save(this, *result_);
  }
#endif

  template <class Unit, class Args>
  void RealTask<Unit,Args>::compute() {
    typename Unit::args_type new_args;
    tuple_convert(new_args, args_);

    if (result_ != nullptr)
      delete result_;

    result_ = new typename Unit::result_type(computing_unit_(new_args));

    on_memory_ = true;
    if (computing_unit_.should_save())
      on_disk_ = task_manager_->save(this, *result_);
  }

  template <class Unit, class Args>
  void RealTask<Unit,Args>::unload() {
    on_memory_ = false;
    if (result_ != nullptr) {
      delete result_;
      result_ = nullptr;
    }
  }*/

  template <class T>
  Task<T>::operator T() const {
    BOOST_ASSERT_MSG(task_key_.is_valid(), "invalid task key");
    T ret;
    task_manager_->get_result(task_key_, ret);
    return ret;
  }
};

#endif
