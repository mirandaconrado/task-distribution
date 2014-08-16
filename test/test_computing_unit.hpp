#ifndef __TASK_DISTRIBUTION__TEST_COMPUTING_UNIT_HPP__
#define __TASK_DISTRIBUTION__TEST_COMPUTING_UNIT_HPP__

#include "computing_unit.hpp"

class TestComputingUnit: public
                         TaskDistribution::ComputingUnit<TestComputingUnit> {
  public:
    TestComputingUnit():
      TestComputingUnit(2) { }

    TestComputingUnit(int _val):
      TaskDistribution::ComputingUnit<TestComputingUnit>("TestComputingUnit"),
      val(_val) { }

    int operator()(int const& new_val) const {
      return val * new_val;
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & val;
    }

    int val;
};

#endif
