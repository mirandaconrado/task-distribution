#ifndef __TASK_DISTRIBUTION__TUPLE_SERIALIZE_HPP__
#define __TASK_DISTRIBUTION__TUPLE_SERIALIZE_HPP__

#include <tuple>

namespace TaskDistribution {
  template<size_t I, class Archive, class Tuple >
  typename std::enable_if<(I == std::tuple_size<Tuple>::value), void>::type
  serialize_tuple(Archive& ar, Tuple& t, const unsigned int version) { }

  template<size_t I, class Archive, class Tuple>
  typename std::enable_if<(I < std::tuple_size<Tuple>::value), void>::type
  serialize_tuple(Archive& ar, Tuple& t, const unsigned int version) {
    ar & std::get<I>(t);
    serialize_tuple<I + 1>(ar, t, version);
  }
};

namespace boost { namespace serialization {
  template<class Archive, class... Types>
  void serialize(Archive& ar, std::tuple<Types...>& t,
      const unsigned int version) {
    TaskDistribution::serialize_tuple<0>(ar, t, version);
   }
}; };

#endif
