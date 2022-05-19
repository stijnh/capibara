#pragma once
#include <tuple>

#include "expr.h"

namespace capybara {
template<template<typename...> class R, typename... Cs>
struct zip_cursor;

template<template<typename...> class R, typename... Es>
struct expr_traits<zip_expr<R, Es...>> {
    static constexpr size_t rank =
        std::tuple_element<0, std::tuple<Es...>>::type::rank;
    static_assert(fun::all(Es::rank == rank...), "invalid rank of operands");

    static constexpr bool is_writable = all(expr_traits<Es>::is_writable...);
    static constexpr bool is_view = false;
    using value_type = R<expr_value_type<Es>...>;
};

template<template<typename...> class R, typename... Es, typename D>
struct expr_cursor<const zip_expr<R, Es...>, D> {
    using type = zip_cursor<R, expr_cursor_type<const Es, D>...>;
    static constexpr size_t rank = expr_rank<Es...>;

    template<size_t... Is>CAPYBARA_INLINE
    static type call_helper(
        const zip_expr<R, Es...>& expr,
        dshape<rank> shape,
        D device,
        std::index_sequence<Is...>) {
        return type(std::get<Is>(expr.operands()).cursor(shape, device)...);
    }
    CAPYBARA_INLINE
    static type
    call(const zip_expr<R, Es...>& expr, dshape<rank> shape, D device) {
        return call_helper(
            expr,
            shape,
            device,
            std::index_sequence_for<Es...> {});
    }
};

template<template<typename...> class R, typename... Es>
struct zip_expr: expr<zip_expr<R, Es...>> {
    zip_expr(Es... operands) : operands_(std::move(operands)...) {
        //
    }

    CAPYBARA_INLINE
    index_t dimension_impl(index_t axis) const {
        return dimension_broadcast<std::tuple<Es...>>::call(axis, operands_);
    }

    CAPYBARA_INLINE
    const std::tuple<Es...>& operands() const {
        return operands_;
    }

  private:
    std::tuple<Es...> operands_;
};

//
template<template<typename...> class R, typename... Cs>
struct zip_cursor {
    using value_type = R<decltype(std::declval<Cs>().load())...>;

    zip_cursor(Cs... cursor) : operands_(std::move(cursor)...) {}

    CAPYBARA_INLINE
    void advance(index_t axis, index_t steps) {
        seq::for_each(operands_, [axis, steps](auto cursor) {
            cursor.advance(axis, steps);
        });
    }

    CAPYBARA_INLINE
    value_type load() {
        return load_helper(std::index_sequence_for<Cs...> {});
    }

    void store(value_type v) {
        store_helper(std::index_sequence_for<Cs...> {}, std::move(v));
    }

  private:
    template<size_t... Is>
    CAPYBARA_INLINE value_type load_helper(std::index_sequence<Is...>) {
        return value_type(std::get<Is>(operands_).load()...);
    }

    template<size_t... Is>
    CAPYBARA_INLINE void
    store_helper(std::index_sequence<Is...>, value_type v) {
        std::initializer_list<int> {
            (std::get<Is>(operands_).store(std::get<Is>(v)), int {})...};
    }

  private:
    std::tuple<Cs...> operands_;
};

template<template<typename...> class R, typename... Es>
using expr_zip_type = zip_expr<R, broadcast_expr_type<Es, Es...>...>;

template<template<typename...> class R = std::tuple, typename... Es>
expr_zip_type<R, Es...> zip(Es&&... args) {
    return expr_zip_type<R, Es...>(
        broadcast_expr<Es, Es...>(std::forward<Es>(args))...);
}

}  // namespace capybara