#include "test_computing_unit.hpp"

#include <gtest/gtest.h>

TEST(ComputingUnit, ID) {
  std::hash<std::string> hasher;
  size_t id = hasher(typeid(TestComputingUnit).name());

  EXPECT_EQ(nullptr, TestComputingUnit::get_by_id(id));

  TestComputingUnit unit;

  EXPECT_EQ(id, unit.get_id());

  EXPECT_NE(&unit, TestComputingUnit::get_by_id(id));

  EXPECT_EQ(id, TestComputingUnit::get_by_id(id)->get_id());
}

TEST(ComputingUnit, Defaults) {
  TestComputingUnit unit;
  EXPECT_FALSE(unit.run_locally());
  EXPECT_TRUE(unit.should_save());
}

TEST(ComputingUnit, Identity) {
  TaskDistribution::IdentityComputingUnit<int> unit;
  EXPECT_TRUE(unit.run_locally());
  EXPECT_FALSE(unit.should_save());

  for (int i = 0; i < 10; i++)
    EXPECT_EQ(i, unit(std::make_tuple(i)));
}
