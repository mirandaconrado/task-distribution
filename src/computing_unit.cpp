#include "computing_unit.hpp"

namespace TaskDistribution {
  BaseComputingUnit const* BaseComputingUnit::get_by_id(std::string const& id) {
    auto it = id_map_.find(id);
    if (it == id_map_.end())
      return nullptr;

    return it->second;
  }

  BaseComputingUnit const* BaseComputingUnit::get_by_key(Key const& key) {
    auto it = key_map_.find(key);
    if (it == key_map_.end())
      return nullptr;

    return it->second;
  }

  bool BaseComputingUnit::bind_key(std::string const& id, Key const& key) {
    BaseComputingUnit const* unit = get_by_id(id);
    if (unit == nullptr)
      return false;
    key_map_.emplace(key, unit);
    return true;
  }

  std::unordered_map<std::string,BaseComputingUnit const*>
    BaseComputingUnit::id_map_;
  std::unordered_map<Key,BaseComputingUnit const*>
    BaseComputingUnit::key_map_;
};
