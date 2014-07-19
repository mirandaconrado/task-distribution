#ifndef __TASK_DISTRIBUTION__COMPUTING_UNIT_HPP__
#define __TASK_DISTRIBUTION__COMPUTING_UNIT_HPP__

#if !(NO_MPI)
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>
#endif
#include <tuple>
#include <unordered_map>

namespace TaskDistribution {
  class BaseComputingUnit {
    public:
#if !(NO_MPI)
      virtual void execute(boost::mpi::communicator& world) const=0;
#endif

      static BaseComputingUnit const* get_by_id(size_t id);

      static size_t get_invalid_id();

      virtual bool run_locally() const {
        return false;
      }

      virtual bool should_save() const {
        return true;
      }

      size_t get_id() const {
        return id_;
      }

      template<class Archive>
      void serialize(Archive& ar, const unsigned int version) { }

    protected:
      size_t id_;

      static std::unordered_map<size_t,BaseComputingUnit*> map_;
  };

  template <class T>
  class ComputingUnit: public BaseComputingUnit {
    public:
      ComputingUnit();

#if !(NO_MPI)
      virtual void execute(boost::mpi::communicator& world) const;
#endif

    private:
      ComputingUnit(bool) { }

      static ComputingUnit<T> internal_;
  };

  template <class T>
  class IdentityComputingUnit: public ComputingUnit<IdentityComputingUnit<T>> {
    public:
      typedef T result_type;
      typedef std::tuple<T> args_type;
      static const std::string name;

      virtual bool run_locally() const {
        return true;
      }

      virtual bool should_save() const {
        return false;
      }

      result_type operator()(args_type const& args) const {
        return std::get<0>(args);
      }
  };
  template <class T>
  const std::string IdentityComputingUnit<T>::name("identity");
};

#include "computing_unit_impl.hpp"

#endif
