#pragma once
#include <type_traits>

#include "util.h"

namespace capybara {

template<typename D>
struct expr;

template<typename E>
static constexpr bool is_expr =
    std::is_base_of<expr<decay_t<E>>, decay_t<E>>::value;

template<typename L, typename S>
struct array_base;

template<typename F, typename... Es>
struct apply_expr;

template<typename F, size_t N = 0>
struct nullary_expr;

template<template<typename...> class R, typename... Es>
struct zip_expr;

template<typename C, typename... Es>
struct select_expr;

template<typename V, typename E>
struct view_expr;

template<typename D, typename = void>
struct expr_traits {};

template<typename D, typename = void>
struct expr_nested {
    using type = D;
};

template<typename D>
struct expr_nested<const D>: expr_nested<D> {};

template<typename D>
struct expr_nested<D&&>: expr_nested<D> {};

template<typename D>
struct expr_nested<D&>: expr_nested<D> {};

template<typename D>
struct expr_nested<const D&>: expr_nested<const D> {};

template<typename E>
using expr_nested_type = typename expr_nested<E>::type;

template<typename E, typename D, typename = void>
struct expr_cursor: expr_cursor<const E, D> {};

template<typename E, typename D>
using expr_cursor_type = typename expr_cursor<E, D>::type;

template<typename E, typename = void>
struct expr_conversion {};

template<typename E>
struct expr_conversion<E, enable_t<is_expr<E>>> {
    using type = expr_nested_type<E>;

    CAPYBARA_INLINE
    static type call(E&& expr) {
        return type(std::forward<E>(expr));
    }
};

template<typename... Es>
static constexpr size_t expr_rank =
    fun::max(expr_traits<typename expr_conversion<Es>::type>::rank...);

template<typename E, size_t N, typename = void>
struct expr_broadcast {};

template<typename E, size_t N>
struct expr_broadcast<E, N, enable_t<(N == expr_rank<E>)>> {
    using type = typename expr_conversion<E>::type;

    CAPYBARA_INLINE
    static type call(E&& expr) {
        return expr_conversion<E>::call(std::forward<E>(expr));
    }
};

namespace view {
    template<size_t N, size_t P>
    struct prepend_axes;
}

template<typename E, size_t N>
struct expr_broadcast<E, N, enable_t<(N > expr_rank<E>)>> {
    constexpr static size_t M = expr_rank<E>;
    using view_type = view::prepend_axes<M, N - M>;
    using type = view_expr<view_type, typename expr_conversion<E>::type>;

    CAPYBARA_INLINE
    static type call(E&& expr) {
        return type(
            view_type {},
            expr_conversion<E>::call(std::forward<E>(expr)));
    }
};

template<typename E, size_t N = expr_rank<E>>
using into_expr_type = typename expr_broadcast<E, N>::type;

template<size_t N, typename E>
CAPYBARA_INLINE into_expr_type<E, N> into_expr(E&& expr) {
    return expr_broadcast<E, N>::call(std::forward<E>(expr));
}

template<typename E>
CAPYBARA_INLINE into_expr_type<E> into_expr(E&& expr) {
    return expr_broadcast<E, expr_rank<E>>::call(std::forward<E>(expr));
}

template<typename E, typename... Es>
using broadcast_expr_type = into_expr_type<E, expr_rank<Es...>>;

template<typename E, typename... Es>
CAPYBARA_INLINE broadcast_expr_type<E, Es...> broadcast_expr(E&& expr) {
    return expr_broadcast<E, expr_rank<Es...>>::call(std::forward<E>(expr));
}

template<typename E>
using expr_value_type = typename expr_traits<into_expr_type<E>>::value_type;

struct device_seq {};

// Where should this go?
template<
    typename Tuple,
    typename Indices = std::make_index_sequence<std::tuple_size<Tuple>::value>>
struct dimension_broadcast;

template<typename Tuple>
struct dimension_broadcast<Tuple, std::index_sequence<>> {
    CAPYBARA_INLINE
    static index_t call(index_t axis, const Tuple& tuple) {
        return 1;
    }
};

template<typename Tuple, size_t I, size_t... Rest>
struct dimension_broadcast<Tuple, std::index_sequence<I, Rest...>> {
    CAPYBARA_INLINE
    static index_t call(index_t axis, const Tuple& tuple) {
        index_t d = std::get<I>(tuple).dimension(axis);
        if (d == 1) {
            return dimension_broadcast<Tuple, std::index_sequence<Rest...>>::
                call(axis, tuple);
        } else {
            return d;
        }
    }
};

}  // namespace capybara