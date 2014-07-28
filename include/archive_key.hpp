#ifndef __TASK_DISTRIBUTION__ARCHIVE_KEY_HPP__
#define __TASK_DISTRIBUTION__ARCHIVE_KEY_HPP__

#include <boost/functional/hash.hpp>
#include <functional>

namespace TaskDistribution {
  struct ArchiveKey {
    size_t node_id;
    size_t obj_id;

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
