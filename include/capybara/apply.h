#pragma once
#include <tuple>

#include "expr.h"

namespace capybara {
template<typename F, typename... Cs>
struct apply_cursor;

template<typename F, typename... Es>
struct expr_traits<apply_expr<F, Es...>> {
    static constexpr size_t rank =
        std::tuple_element<0, std::tuple<Es...>>::type::rank;
    static_assert(fun::all(Es::rank == rank...), "invalid rank of operands");

    static constexpr bool is_writable = false;
    static constexpr bool is_view = false;
    using value_type =
        typename invoke_result<F, typename expr_traits<Es>::value_type...>::
            type;
};

template<typename F, typename... Es, typename D>
struct expr_cursor<const apply_expr<F, Es...>, D> {
    using type = apply_cursor<F, expr_cursor_type<const Es, D>...>;
    static constexpr size_t rank = expr_rank<Es...>;

    template<size_t... Is>
    static type call_helper(
        const apply_expr<F, Es...>& expr,
        dshape<rank> shape,
        D device,
        std::index_sequence<Is...>) {
        return type(
            expr.functor(),
            std::get<Is>(expr.operands()).cursor(shape, device)...);
    }

    static type
    call(const apply_expr<F, Es...>& expr, dshape<rank> shape, D device) {
        return call_helper(
            expr,
            shape,
            device,
            std::index_sequence_for<Es...> {});
    }
};

template<typename F, typename... Es>
struct apply_expr: expr<apply_expr<F, Es...>> {
    apply_expr(F function, Es... operands) :
        function_(std::move(function)),
        operands_(std::move(operands)...) {
        //
    }

    CAPYBARA_INLINE
    index_t dimension_impl(index_t axis) const {
        return dimension_broadcast<std::tuple<Es...>>::call(axis, operands_);
    }

    const F& functor() const {
        return function_;
    }

    const std::tuple<Es...>& operands() const {
        return operands_;
    }

  private:
    F function_;
    std::tuple<Es...> operands_;
};

//
template<typename F, typename... Cs>
struct apply_cursor {
    using value_type =
        typename invoke_result<F, decltype(std::declval<Cs>().load())...>::type;

    apply_cursor(F function, Cs... cursor) :
        function_(std::move(function)),
        operands_(std::move(cursor)...) {}

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

  private:
    template<size_t... Is>
    CAPYBARA_INLINE value_type load_helper(std::index_sequence<Is...>) {
        return function_(std::get<Is>(operands_).load()...);
    }

    F function_;
    std::tuple<Cs...> operands_;
};

template<typename F, typename... Es>
using expr_map_type = apply_expr<F, broadcast_expr_type<Es, Es...>...>;

template<typename F, typename... Es>
CAPYBARA_INLINE expr_map_type<F, Es...> map(F fun, Es&&... args) {
    return expr_map_type<F, Es...>(
        std::move(fun),
        broadcast_expr<Es, Es...>(std::forward<Es>(args))...);
}

}  // namespace capybara