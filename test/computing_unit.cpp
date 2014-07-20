#include "computing_unit.hpp"

#include <gtest/gtest.h>

class TestComputingUnit: public TaskDistribution::ComputingUnit<TestComputingUnit> {
  public:
    typedef int result_type;
    typedef std::tuple<int> args_type;
    static const std::string name;

    result_type operator()(args_type const& args) const {
      return 2*std::get<0>(args);
    }
};
const std::string TestComputingUnit::name("TestComputingUnit");

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