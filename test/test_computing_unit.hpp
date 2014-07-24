#ifndef __TASK_DISTRIBUTION__TEST_COMPUTING_UNIT_HPP__
#define __TASK_DISTRIBUTION__TEST_COMPUTING_UNIT_HPP__

#include "computing_unit.hpp"

class TestComputingUnit: public
                         TaskDistribution::ComputingUnit<TestComputingUnit> {
  public:
    TestComputingUnit(): val_(2) { }
    TestComputingUnit(int val): val_(val) { }

    typedef int result_type;
    typedef std::tuple<int> args_type;
    static const std::string name;

    result_type operator()(args_type const& args) const {
      return val_*std::get<0>(args);
    }

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
      ar & val_;
    }

  private:
    int val_;
};
const std::string TestComputingUnit::name("TestComputingUnit");

#endif
