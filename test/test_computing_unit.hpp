#ifndef __TASK_DISTRIBUTION__TEST_COMPUTING_UNIT_HPP__
#define __TASK_DISTRIBUTION__TEST_COMPUTING_UNIT_HPP__

#include "computing_unit.hpp"

class TestComputingUnit: public
                         TaskDistribution::ComputingUnit<TestComputingUnit> {
  public:
    TestComputingUnit():
      TestComputingUnit(2) { }

    TestComputingUnit(int val):
      TaskDistribution::ComputingUnit<TestComputingUnit>("TestComputingUnit"),
      val_(val) { }

    int operator()(int const& new_val) const {
      return val_ * new_val;
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & val_;
    }

  private:
    int val_;
};

#endif
