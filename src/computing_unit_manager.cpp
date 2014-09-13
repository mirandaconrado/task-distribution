#include "computing_unit_manager.hpp"

#include "computing_unit.hpp"

namespace TaskDistribution {
  ComputingUnitManager::ComputingUnitManager(
      ObjectArchive<Key>& archive):
    archive_(archive) { }

  void ComputingUnitManager::process_local(TaskEntry& task) {
    if (task.result_key.is_valid())
      return;

    if (task.should_save) {
      task.result_key = new_key(Key::Result);
    }

    // Assumes that the computing unit is defined. TODO: remove this assumption.
    BaseComputingUnit const* unit;

    // Gets the correct computing unit and executes it
    unit = BaseComputingUnit::get_by_key(task.computing_unit_id_key);
    if (unit == nullptr) {
      std::string computing_unit_id;
      archive_.load(task.computing_unit_id_key, computing_unit_id);
      // Assumes true is always returned.
      BaseComputingUnit::bind_key(computing_unit_id,
          task.computing_unit_id_key);
      unit = BaseComputingUnit::get_by_key(task.computing_unit_id_key);
    }

    unit->execute(archive_, task, *this);

    if (task.should_save)
      archive_.insert(task.task_key, task);
  }

  Key ComputingUnitManager::new_key(Key::Type type) {
    return Key::new_key(type);
  }
}
