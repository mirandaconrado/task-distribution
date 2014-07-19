#include "computing_unit.hpp"

#include <gtest/gtest.h>

TEST(ComputingUnit, IdentityComputingUnit) {
  TaskDistribution::IdentityComputingUnit<int> unit;
  EXPECT_TRUE(unit.run_locally());
  EXPECT_FALSE(unit.should_save());

  for (int i = 0; i < 10; i++)
    EXPECT_EQ(i, unit(std::make_tuple(i)));
}
