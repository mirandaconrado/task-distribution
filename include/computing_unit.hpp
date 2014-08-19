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
// and provide the following inside the class:
// 1) the constructor should call ComputingUnit<MyUnit> with the desired unit's
// name as argument;
// 1) a method operator() that performs the computation desired.
//
// Caveats:
// 1) Only the return of operator() is considered as the result, so any changes
// to the argument are ignored.
// 2) All arguments and the return of operator() must be serializable through
// boost.
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

#include "object_archive.hpp"

#include <tuple>
#include <type_traits>
#include <unordered_map>

#include "archive_info.hpp"
#include "compile_utils.hpp"
#include "computing_unit_manager.hpp"

namespace TaskDistribution {
  // Abstract class to allow us to have pointers to units. Shouldn't be used
  // directly!
  class BaseComputingUnit {
    public:
      // Must the object be called on the master node? Defaults to false.
      virtual bool run_locally() const {
        return false;
      }

      // Should the computing results be saved? Defaults to true.
      virtual bool should_save() const {
        return true;
      }

      // Static method to fetch the correct kind of unit for an id.
      static BaseComputingUnit const* get_by_id(std::string const& id);

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
      // archive. Assumes every ArchiveKey provided is valid.
      virtual void execute(ObjectArchive<ArchiveKey>& archive,
          TaskEntry const& task, ComputingUnitManager& manager) const = 0;

    protected:
      // Allows access to id_
      template <class> friend class ComputingUnit;

      std::string const* id_;

      // Map between hashes and units.
      static std::unordered_map<std::string,BaseComputingUnit*> map_;
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

      virtual void execute(ObjectArchive<ArchiveKey>& archive,
          TaskEntry const& task, ComputingUnitManager& manager) const;

    private:
      // Internal constructor to avoid deadlock during unit register.
      ComputingUnit() { }
  };

  // Example ComputingUnit that just returns its argument. Check that it
  // satisfies all the requirements for a valid unit.
  template <class T>
  class IdentityComputingUnit: public ComputingUnit<IdentityComputingUnit<T>> {
    public:
      IdentityComputingUnit():
        ComputingUnit<IdentityComputingUnit<T>>("identity") { }

      virtual bool run_locally() const {
        return true;
      }

      virtual bool should_save() const {
        return false;
      }

      T operator()(T const& arg) const {
        return arg;
      }
  };

  template <class From, class To>
  class ConvertComputingUnit:
    public ComputingUnit<ConvertComputingUnit<From, To>> {
    public:
      ConvertComputingUnit():
        ComputingUnit<ConvertComputingUnit<From, To>>("convert") {
        static_assert(std::is_convertible<From,To>::value,
            "Invalid ConvertComputingUnit as types aren't convertible!");
        static_assert(!std::is_same<From,To>::value,
            "Invalid ConvertComputingUnit as types are the same!");
      }

      virtual bool run_locally() const {
        return true;
      }

      virtual bool should_save() const {
        return false;
      }

      To operator()(From const& arg) const {
        return arg;
      }
  };
};

#include "computing_unit_impl.hpp"

#endif
