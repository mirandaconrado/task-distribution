#ifndef __TASK_DISTRIBUTION__FUNCTION_TRAITS_HPP__
#define __TASK_DISTRIBUTION__FUNCTION_TRAITS_HPP__

#include <boost/integer/static_min_max.hpp>
#include <type_traits>
#include <tuple>

namespace TaskDistribution {
  // Mine
  template <class... Types>
  struct clean_tuple {
    using type = std::tuple<
      typename std::remove_const<
        typename std::remove_reference<Types>::type
      >::type...>;
  };

  // http://stackoverflow.com/questions/14852593/removing-the-first-type-of-a-stdtuple
  template <size_t I, typename T> struct remove_ith_type { };

  template <typename T, typename... Ts>
  struct remove_ith_type<0, std::tuple<T, Ts...>> {
    typedef std::tuple<Ts...> type;
  };

  template <size_t I, typename T, typename... Ts>
  struct remove_ith_type<I, std::tuple<T, Ts...>> {
    typedef decltype(
        std::tuple_cat(
          std::declval<std::tuple<T>>(),
          std::declval<typename remove_ith_type<I-1, std::tuple<Ts...>>::type>()
          )
        ) type;
  };

  // http://functionalcpp.wordpress.com/2013/08/05/function-traits/
  template <class F> struct function_traits;

  // function pointer
  template <class R, class... Args>
  struct function_traits<R(*)(Args...)>: public function_traits<R(Args...)> {};

  template <class R, class... Args>
  struct function_traits<R(Args...)> {
    using return_type = R;

    static constexpr size_t arity = sizeof...(Args);

    template <size_t N>
    struct argument {
      static_assert(N < arity, "error: invalid parameter index.");
      using type = typename std::tuple_element<N,std::tuple<Args...>>::type;
    };

    using arg_tuple_type = typename clean_tuple<Args...>::type;
  };

  // member function pointer
  template <class C, class R, class... Args>
  struct function_traits<R(C::*)(Args...)>:
    public function_traits<R(C&,Args...)> {};

  // const member function pointer
  template <class C, class R, class... Args>
  struct function_traits<R(C::*)(Args...) const>:
    public function_traits<R(C&,Args...)> {};

  // member object pointer
  template <class C, class R>
  struct function_traits<R(C::*)>: public function_traits<R(C&)> {};

  // functor
  template <class F>
  struct function_traits {
    private:
      using call_type = function_traits<decltype(&F::operator())>;
    public:
      using return_type = typename call_type::return_type;

      static constexpr size_t arity = call_type::arity - 1;

      template <size_t N>
      struct argument {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename call_type::template argument<N+1>::type;
      };

      using arg_tuple_type =
        typename remove_ith_type<0, typename call_type::arg_tuple_type>::type;
  };

  template <class F>
  struct function_traits<F&>: public function_traits<F> {};

  template <class F>
  struct function_traits<F&&>: public function_traits<F> {};

  // http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer/7858971#7858971
  template <size_t...> struct seq { };

  template <size_t N, size_t... S> struct gens: gens<N-1, N-1, S...> { };

  template <size_t... S> struct gens<0, S...> { typedef seq<S...> type; };

  // Mine
  template <size_t I, class From, class To>
  struct is_tuple_convertible_ {
    static constexpr bool value =
      std::is_convertible<
        typename std::tuple_element<I-1,From>::type,
        typename std::tuple_element<I-1,To>::type
      >::value && is_tuple_convertible_<I-1,From,To>::value;
  };

  template <class From, class To>
  struct is_tuple_convertible_<0, From, To> {
    static constexpr bool value = true;
  };

  template <class From, class To>
  struct is_tuple_convertible {
    static constexpr bool value =
      is_tuple_convertible_<
        boost::static_unsigned_min<
          std::tuple_size<From>::value,
          std::tuple_size<To>::value
        >::value,
        From, To>::value;
  };

  // Mine
  /*template <class T> class Task;

  template <size_t I, class To, class... From>
  struct convert_tuple_ { };

  template <size_t I, class To, class From1, class... From>
  struct convert_tuple_<I, To, std::tuple<From1, From...>> {
    typedef decltype(
        std::tuple_cat(
          std::tuple<typename std::tuple_element<I-1,To>::type>(),
          typename convert_tuple_<I-1, To, std::tuple<From...>>::type()
          )
        ) type;
  };

  template <size_t I, class To, class From1, class... From>
  struct convert_tuple_<I, To, std::tuple<Task<From1>, From...>> {
    typedef decltype(
        std::tuple_cat(
          std::tuple<Task<From1>>(),
          typename convert_tuple_<I-1, To, std::tuple<From...>>::type()
          )
        ) type;
  };

  template <class To, class From>
  struct convert_tuple_<1, To, std::tuple<From>> {
    typedef std::tuple<typename std::tuple_element<0,To>::type> type;
  };

  template <class To, class From>
  struct convert_tuple_<1, To, std::tuple<Task<From>>> {
    typedef std::tuple<Task<From>> type;
  };

  template <class To, class... From>
  struct convert_tuple {
    typedef
      typename convert_tuple_<std::tuple_size<To>::value, To, From...>::type
      type;
  };*/

  // Mine
  template <size_t N, class T, class... Ts>
  struct repeated_tuple_: repeated_tuple_<N-1, T, T, Ts...> { };

  template <class T, class... Ts>
  struct repeated_tuple_<0, T, Ts...> { typedef std::tuple<Ts...> type; };

  template <size_t I, class T>
  struct repeated_tuple {
    using type = typename repeated_tuple_<I, T>::type;
  };
};

#endif
