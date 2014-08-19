#ifndef __TASK_DISTRIBUTION__ARCHIVE_INFO_HPP__
#define __TASK_DISTRIBUTION__ARCHIVE_INFO_HPP__

#if ENABLE_MPI
#include <boost/mpi/communicator.hpp>
#endif
#include <boost/functional/hash.hpp>
#include <functional>

namespace TaskDistribution {
  struct ArchiveKey {
    size_t node_id;
    size_t obj_id;

    ArchiveKey(): node_id(0), obj_id(0) { }

    ArchiveKey(size_t node, size_t obj):
      node_id(node),
      obj_id(obj) { }

    bool is_valid() const { return obj_id != 0; }

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
    }

    static size_t next_obj;

#if ENABLE_MPI
    static ArchiveKey new_key(boost::mpi::communicator& world) {
      ArchiveKey ret({0, next_obj++});
      ret.node_id = world.rank();
      return ret;
    }
#else
    static ArchiveKey new_key() {
      ArchiveKey ret({0, next_obj++});
      return ret;
    }
#endif
  };

  struct TaskEntry {
    ArchiveKey task_key;
    ArchiveKey computing_unit_key;
    ArchiveKey arguments_key;
    ArchiveKey arguments_tasks_key;
    ArchiveKey result_key;
    ArchiveKey computing_unit_id_key;
    bool should_save, run_locally;

    TaskEntry(): should_save(true), run_locally(false) { }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & task_key;
      ar & computing_unit_key;
      ar & arguments_key;
      ar & arguments_tasks_key;
      ar & result_key;
      ar & computing_unit_id_key;
    }
  };
};

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
