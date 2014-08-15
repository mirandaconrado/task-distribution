#include "computing_unit.hpp"

namespace TaskDistribution {
  BaseComputingUnit const* BaseComputingUnit::get_by_id(std::string const& id) {
    if (map_.find(id) == map_.end())
      return nullptr;

    return map_[id];
  }

  std::unordered_map<std::string,BaseComputingUnit*> BaseComputingUnit::map_;
};
