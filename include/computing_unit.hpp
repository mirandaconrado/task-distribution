// Each task is composed of a computing unit, which performs the desired
// computation, and the arguments provided by the user. This file defines how
// the computing units are created.
//
// Each computing unit is identified by an id that should be unique, so that
// the class used to compute can be identified. Currently the id is given by the
// name provided by to the constructor. The class BaseComputingUnit is used only
// to provide abstract pointers to units and thus should NEVER be used directly.
//
// To create a new computing unit, inherit from ComputingUnit<T> like this:
// class MyUnit: public ComputingUnit<MyUnit>
// and ensure the following rules:
// 1) a constructor without arguments must be provided;
// 2) the constructor must call ComputingUnit<MyUnit> with the desired unit's
// name as argument;
// 3) a const method operator() that performs the computation desired;
// 4) if the object has any internal parameters, the user must create the method
// serialize.
//
// Caveats:
// 1) Only the return of operator() is considered as the result, so any changes
// to the argument are ignored;
// 2) All arguments and the return of operator() must be serializable through
// boost;
// 3) operator() must be declared as const, to ensure no modifications are
// performed.
//
// Besides these requirements, the user has control over whether the object must
// run on the master node, chosen through the method "run_locally()". This
// should be used in case there is some restriction, like file requirements or
// the operator() method is so fast that the communication overhead isn't worth.
// The unit is allowed to run anywhere by default.
//
// For an example of how to implement an unit, check code on
// example/example.cpp.

#ifndef __TASK_DISTRIBUTION__COMPUTING_UNIT_HPP__
#define __TASK_DISTRIBUTION__COMPUTING_UNIT_HPP__

#include "function_traits.hpp"
#include "object_archive.hpp"
#include "sequence.hpp"

#include <tuple>
#include <type_traits>
#include <unordered_map>

#include "computing_unit_manager.hpp"
#include "key.hpp"
#include "task_entry.hpp"

namespace TaskDistribution {
  // Abstract class to allow us to have pointers to units. Shouldn't be used
  // directly!
  class BaseComputingUnit {
    public:
      // Must the object be called on the master node? Defaults to false.
      virtual bool run_locally() const {
        return false;
      }

      // Static method to fetch the correct kind of unit for an id. Returns NULL
      // if not found.
      static BaseComputingUnit const* get_by_id(std::string const& id);

      // Static method to fetch the correct kind of unit for a key. Returns NULL
      // if not found.
      static BaseComputingUnit const* get_by_key(Key const& key);

      // Binds a given id with a key. Multiple keys may be bound to the same id.
      // Returns true if the id was found.
      static bool bind_key(std::string const& id, Key const& key);

      // Gets the id associated with this kind of unit.
      std::string const& get_id() const {
        return *id_;
      }

      // Provides an invalid unit id.
      static std::string get_invalid_id() {
        return "";
      }

      // Callables need to be serializable. The default behavior is to transmit
      // nothing and may be overloaded.
      template<class Archive>
      void serialize(Archive& ar, const unsigned int version) { }

      // Loads the computing unit and arguments and stores the result in the
      // archive. Assumes every Key provided is valid.
      virtual void execute(ObjectArchive<Key>& archive,
          TaskEntry const& task, ComputingUnitManager& manager) const = 0;

    protected:
      // Expands the tuple and calls the functor
      template <class F, class Tuple, size_t... S>
      static typename CompileUtils::function_traits<F>::return_type
      apply(F&& f, Tuple&& args, CompileUtils::sequence<S...>) {
        return std::forward<F>(f)(std::get<S>(std::forward<Tuple>(args))...);
      }

      // Name of the computing unit given at construction
      std::string const* id_;

      // Map between ids and units.
      static std::unordered_map<std::string,BaseComputingUnit const*> id_map_;
      // Map between keys and units.
      static std::unordered_map<Key,BaseComputingUnit const*> key_map_;
  };

  // Class that should be inherited by the user's units. For an example on how
  // to do this, check the top of the file or the IdentityComputingUnit example.
  // Each kind of ComputingUnit used must be instantiated at least once and may
  // be destroyed before running the tasks.
  template <class T>
  class ComputingUnit: public BaseComputingUnit {
    public:
      // Registers the unit by placing a new copy into the units' map.
      explicit ComputingUnit(std::string const& name);

      virtual void execute(ObjectArchive<Key>& archive,
          TaskEntry const& task, ComputingUnitManager& manager) const;

    private:
      // Internal constructor to avoid deadlock during unit register.
      ComputingUnit() { }
  };
};

#include "computing_unit_impl.hpp"

#endif
