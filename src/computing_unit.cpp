#include "computing_unit.hpp"

namespace TaskDistribution {
  BaseComputingUnit const* BaseComputingUnit::get_by_id(size_t id) {
    if (map_.find(id) == map_.end())
      return nullptr;

    return map_[id];
  }

  size_t BaseComputingUnit::get_invalid_id() {
    size_t hash = 0;

    while (map_.find(hash) != map_.end())
      hash++;

    return hash;
  }

  std::unordered_map<size_t,BaseComputingUnit*> BaseComputingUnit::map_;
};
