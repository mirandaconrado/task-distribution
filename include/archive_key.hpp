// Everything is stores in an ObjectArchive and must use the same key. This file
// defines the key.
//
// Each key is composed on the node that created it (0 if MPI is disabled) and a
// unique object id, which can be obtained using the method "new_key".
//
// The key can also be used as key for std containers, allowing easy
// integration of many different storage classes.
//
// By default, a key is considered invalid if the object associated with it has
// id == 0.

#ifndef __TASK_DISTRIBUTION__ARCHIVE_KEY_HPP__
#define __TASK_DISTRIBUTION__ARCHIVE_KEY_HPP__

#if ENABLE_MPI
#include <boost/mpi/communicator.hpp>
#endif
#include <boost/functional/hash.hpp>
#include <functional>

namespace TaskDistribution {
  // General key for the archive used.
  struct ArchiveKey {
    enum Type {
      Task,
      ComputingUnit,
      Arguments,
      ArgumentsTasks,
      Result,
      ComputingUnitId,
      Unknown
    };

    size_t node_id;
    size_t obj_id;
    Type type;

    // Constructs invalid keys as default.
    ArchiveKey(): node_id(0), obj_id(0), type(Unknown) { }

    ArchiveKey(size_t node, size_t obj, Type _type):
      node_id(node),
      obj_id(obj),
      type(_type){ }

    bool is_valid() const { return obj_id != 0 && type != Unknown; }

    bool operator==(ArchiveKey const& other) const {
      return node_id == other.node_id &&
             obj_id == other.obj_id;
    }

    bool operator<(ArchiveKey const& other) const {
      if (node_id < other.node_id)
        return true;
      if (node_id > other.node_id)
        return false;
      return obj_id < other.obj_id;
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & node_id;
      ar & obj_id;
      ar & type;
    }

    // Creates a new unique key. The method has an internal counter to guarantee
    // that each key created is unique.
#if ENABLE_MPI
    static ArchiveKey new_key(boost::mpi::communicator& world, Type type) {
      static size_t next_obj = 1;
      ArchiveKey ret({0, next_obj++, type});
      ret.node_id = world.rank();
      return ret;
    }
#else
    static ArchiveKey new_key(Type type) {
      static size_t next_obj = 1;
      ArchiveKey ret({0, next_obj++, type});
      return ret;
    }
#endif
  };
};

// Enables faster MPI transmission.
#if ENABLE_MPI
BOOST_IS_MPI_DATATYPE(TaskDistribution::ArchiveKey);
#endif

// Allows ArchiveKey to be used as key in std::unordered_*.
namespace std {
  template<>
  struct hash<TaskDistribution::ArchiveKey> {
    typedef TaskDistribution::ArchiveKey argument_type;
    typedef size_t value_type;

    value_type operator()(TaskDistribution::ArchiveKey const& key) const {
      size_t seed = 0;
      std::hash<size_t> hasher;
      boost::hash_combine(seed, hasher(key.node_id));
      boost::hash_combine(seed, hasher(key.obj_id));

      return seed;
    }
  };
};

#endif
