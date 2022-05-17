#pragma once

#include <limits>
#include <tuple>
#include <type_traits>

#include "defines.h"

namespace capybara {
using size_t = std::size_t;
using index_t = std::ptrdiff_t;
using stride_t = std::ptrdiff_t;

template<typename F, typename... Args>
using invoke_result = std::result_of<F(Args...)>;

template<typename T>
using decay_t = typename std::decay<T>::type;

template<bool C, typename T = void>
using enable_t = typename std::enable_if<C, T>::type;

template<typename...>
using void_t = void;

/// returns `lhs+rhs`, or `numeric_limits<T>::max` if the addition overflows.
template<typename T>
CAPYBARA_INLINE constexpr T saturating_add(T lhs, T rhs) {
    if (CAPYBARA_LIKELY(rhs <= std::numeric_limits<T>::max() - lhs)) {
        return lhs + rhs;
    } else {
        return std::numeric_limits<T>::max();
    }
}

namespace fun {
    CAPYBARA_INLINE
    constexpr bool all() {
        return true;
    }

    template<typename... Ts>
    CAPYBARA_INLINE constexpr bool all(bool first, Ts&&... rest) {
        return first & all(std::forward<Ts>(rest)...);
    }

    CAPYBARA_INLINE
    constexpr bool any() {
        return false;
    }

    template<typename... Ts>
    CAPYBARA_INLINE constexpr bool any(bool first, Ts&&... rest) {
        return first | all(std::forward<Ts>(rest)...);
    }

    template<typename T>
    CAPYBARA_INLINE constexpr T max(T val) {
        return val;
    }

    template<typename T, typename... Rest>
    CAPYBARA_INLINE constexpr T max(T first, T second, Rest... rest) {
        return max(first > second ? first : second, rest...);
    }
}  // namespace fun

namespace seq {
    template<
        template<typename...> class R = std::tuple,
        typename T,
        T... Vals,
        typename F>
    CAPYBARA_INLINE auto map(std::integer_sequence<T, Vals...>, F&& fun)
        -> R<decltype(fun(std::integral_constant<T, Vals>()))...> {
        return {fun(std::integral_constant<T, Vals>())...};
    }

    template<
        size_t N,
        template<typename...> class R = std::tuple,
        typename T = size_t,
        typename F>
    CAPYBARA_INLINE auto map_n(F&& fun) {
        return map<R>(
            std::make_integer_sequence<T, (T)N> {},
            std::forward<F>(fun));
    }
    template<
        template<typename...> class R = std::tuple,
        typename T,
        typename F,
        size_t N = std::tuple_size<decay_t<T>>::value>
    CAPYBARA_INLINE auto map(T&& tuple, F&& fun) {
        return map_n<N>(
            [&](auto i) { return fun(std::get<i>(std::forward<T>(tuple))); });
    }

    template<typename T, T... Vals, typename F>
    CAPYBARA_INLINE void for_each(std::integer_sequence<T, Vals...>, F&& fun) {
        std::initializer_list<int> {
            (fun(std::integral_constant<T, Vals>()), int())...};
    }

    template<size_t N, typename T = size_t, typename F>
    CAPYBARA_INLINE void for_each_n(F&& fun) {
        return for_each(
            std::make_integer_sequence<T, (T)N> {},
            std::forward<F>(fun));
    }

    template<
        typename T,
        typename F,
        size_t N = std::tuple_size<decay_t<T>>::value>
    CAPYBARA_INLINE void for_each(T&& tuple, F&& fun) {
        for_each_n<N>(
            [&](auto i) { fun(std::get<i>(std::forward<T>(tuple))); });
    }

    template<typename T, typename R, typename F>
    CAPYBARA_INLINE R fold(std::integer_sequence<T>, R init, F fun) {
        return init;
    }

    template<typename T, typename R, typename F, T First, T... Rest>
    CAPYBARA_INLINE R
    fold(std::integer_sequence<T, First, Rest...>, R init, F fun) {
        return fold(
            std::integer_sequence<T, Rest...> {},
            fun(std::move(init), std::integral_constant<T, First>()),
            fun);
    }

    template<size_t N, typename T = size_t, typename R, typename F>
    CAPYBARA_INLINE auto fold_n(R init, F fun) {
        return fold(
            std::make_integer_sequence<T, T(N)> {},
            std::move(init),
            fun);
    }

    template<
        typename T,
        typename R,
        typename F,
        size_t N = std::tuple_size<decay_t<T>>::value>
    CAPYBARA_INLINE auto fold(T&& tuple, R init, F fun) {
        return fold_n<N>(std::move(init), [&tuple, &fun](auto init, auto i) {
            return fun(std::move(init), std::get<i>(std::forward<T>(tuple)));
        });
    }

}  // namespace seq
}  // namespace capybara