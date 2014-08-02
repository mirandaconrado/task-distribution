// Each task is composed of a computing unit, which performs the desired
// computation, and the arguments provided by the user. This file defines how
// the computing units are created.
//
// Each computing unit is identified by an id that should be unique, so that
// the class used to compute can be identified. Currently the id is given by the
// hash of the class name, as defined by "typeid(T).name()". The class
// BaseComputingUnit is used only to provide abstract pointers to units and thus
// should NEVER be used directly.
//
// To create a new computing unit, inherit from ComputingUnit<T> like this:
// class MyUnit: public ComputingUnit<MyUnit>
// and provide the following inside the class:
// 1) a method operator() that performs the computation desired;
// 2) a type named result_type that is the same as the return type for
// operator();
// 3) a type named args_type that is the same as the argument for operator();
// 4) a static variable "std::string name" to identify the unit.
//
// Caveats:
// 1) The operator() method currently can only have 1 argument. If more
// arguments are needed, then use a std::tuple.
// 2) Only the return of operator() is considered as the result, so any changes
// to the argument are ignored.
// 3) Both result_type and args_type must be serializable through boost.
//
// Besides these requirements, the user has control over the following options:
// 1) whether the object must run on the master node, chosen through the method
// "run_locally()". This should be used in case there is some restriction, like
// file requirements or the operator() method is so fast that the communication
// overhead isn't worth. The node is allowed to run anywhere by default;
// 2) whether the result should be saved to disk, through the method
// "should_save()". For very simple units, it may not be necessary or even
// desidered to save the result of the computation. The default is to save.
//
// For an example of how to implement an unit, check the IdentityComputingUnit
// below.

#ifndef __TASK_DISTRIBUTION__COMPUTING_UNIT_HPP__
#define __TASK_DISTRIBUTION__COMPUTING_UNIT_HPP__

#if !(NO_MPI)
#include <boost/mpi/communicator.hpp>
#endif
#include <tuple>
#include <unordered_map>

namespace TaskDistribution {
  // Abstract class to allow us to have pointers to units. Shouldn't be used
  // directly!
  class BaseComputingUnit {
    public:
#if !(NO_MPI)
      // If MPI is allowed, we can't just call operator(). This provides a
      // wrapper that must be run on the remote node to fetch all the
      // information required and send the results back.
      virtual void execute(boost::mpi::communicator& world) const=0;
#endif

      // Static method to fetch the correct kind of unit for an id.
      static BaseComputingUnit const* get_by_id(size_t id);

      // Must the object be called on the master node? Defaults to false.
      virtual bool run_locally() const {
        return false;
      }

      // Should the computing results be saved? Defaults to true.
      virtual bool should_save() const {
        return true;
      }

      // Gets the id associated with this kind of unit.
      size_t get_id() const {
        return id_;
      }

      // Provides an invalid unit id.
      static size_t get_invalid_id();

      // Callables need to be serializable in order to be used by MPI. The
      // default behavior is to transmit nothing and may be overloaded.
      template<class Archive>
      void serialize(Archive& ar, const unsigned int version) { }

    protected:
      size_t id_;

      // Map between hashes and units.
      static std::unordered_map<size_t,BaseComputingUnit*> map_;
  };

  // Class that should be inherited by the user's units. For an example on how
  // to do this, check the top of the file or the IdentityComputingUnit example.
  // Each kind of ComputingUnit used must be instantiated at least once and may
  // be destroyed before running the tasks.
  template <class T>
  class ComputingUnit: public BaseComputingUnit {
    public:
      // Registers the unit by placing a new copy into the units' map.
      ComputingUnit();

#if !(NO_MPI)
      // Implements the specific remote execution for type T.
      virtual void execute(boost::mpi::communicator& world) const;
#endif

      template<class Archive>
      void serialize(Archive& ar, const unsigned int version) {
        ar & boost::serialization::base_object<BaseComputingUnit>(*this);
        boost::serialization::void_cast_register<T, ComputingUnit<T>>(
            static_cast<T*>(NULL), static_cast<ComputingUnit<T>*>(NULL));
      }

    private:
      // Internal caller to avoid deadlock during unit register.
      ComputingUnit(size_t id) { id_ = id; }
  };

  // Example ComputingUnit that just returns its argument. Check that it
  // satisfies all the requirements for a valid unit.
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
