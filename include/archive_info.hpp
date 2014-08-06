#ifndef __TASK_DISTRIBUTION__ARCHIVE_INFO_HPP__
#define __TASK_DISTRIBUTION__ARCHIVE_INFO_HPP__

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

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & node_id;
      ar & obj_id;
    }
  };

  struct TaskEntry {
    ArchiveKey task;
    ArchiveKey computing_unit;
    ArchiveKey arguments;
    ArchiveKey result;
    bool should_save, run_locally;

    TaskEntry(): should_save(false), run_locally(true) { }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & task;
      ar & computing_unit;
      ar & arguments;
      ar & result;
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
