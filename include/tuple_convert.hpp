#ifndef __TASK_DISTRIBUTION__TUPLE_CONVERT__HPP__
#define __TASK_DISTRIBUTION__TUPLE_CONVERT__HPP__

#include <tuple>
#include <type_traits>

namespace TaskDistribution {
  template <std::size_t I, class T1, class T2>
  typename std::enable_if<(I == std::tuple_size<T1>::value ||
                           I == std::tuple_size<T2>::value), void>::type
  tuple_convert_detail(T1&, T2 const&) { }

  template <std::size_t I, class T1, class T2>
  typename std::enable_if<(I < std::tuple_size<T1>::value &&
                           I < std::tuple_size<T2>::value), void>::type
  tuple_convert_detail(T1& t1, T2 const& t2) {
    std::get<I>(t1) = std::get<I>(t2);
    tuple_convert_detail<I + 1>(t1, t2);
  }

  template <class T1, class T2>
  typename std::enable_if<std::tuple_size<T1>::value ==
                          std::tuple_size<T2>::value, void>::type
  tuple_convert(T1& t1, T2 const& t2) {
    tuple_convert_detail<0>(t1,t2);
  }

  template <size_t I, class T1, class T2>
  struct is_tuple_convertible {
    static constexpr bool value =
      std::is_convertible<
        typename std::tuple_element<I-1,T1>::type,
        typename std::tuple_element<I-1,T2>::type
      >::value && is_tuple_convertible<I-1,T1,T2>::value;
  };

  template <class T1, class T2>
  struct is_tuple_convertible<0, T1, T2> {
    static constexpr bool value = true;
  };

};

#endif
