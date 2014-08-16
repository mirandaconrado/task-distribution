#include "computing_unit.hpp"

namespace TaskDistribution {
  BaseComputingUnit const* BaseComputingUnit::get_by_id(std::string const& id) {
    auto it = map_.find(id);
    if (it == map_.end())
      return nullptr;

    return it->second;
  }

  std::unordered_map<std::string,BaseComputingUnit*> BaseComputingUnit::map_;
};
