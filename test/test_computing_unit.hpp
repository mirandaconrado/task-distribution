#ifndef __TASK_DISTRIBUTION__TEST_COMPUTING_UNIT_HPP__
#define __TASK_DISTRIBUTION__TEST_COMPUTING_UNIT_HPP__

#include "computing_unit.hpp"

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

#endif
