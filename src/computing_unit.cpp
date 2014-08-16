#include "computing_unit.hpp"

namespace TaskDistribution {
  BaseComputingUnit const* BaseComputingUnit::get_by_id(std::string const& id) {
    auto it = map_.find(id);
    if (it == map_.end())
      return nullptr;

    return it->second;
  }

#if ENABLE_MPI
  int BaseComputingUnit::mpi_tag;
#endif
  std::unordered_map<std::string,BaseComputingUnit*> BaseComputingUnit::map_;
};
