#pragma once
#include <tuple>

#include "expr.h"

namespace capybara {
template<typename... Cs>
struct zip_cursor;

template<typename... Es>
struct expr_traits<zip_expr<Es...>> {
    static constexpr size_t rank =
        std::tuple_element<0, std::tuple<Es...>>::type::rank;
    static_assert(fun::all(Es::rank == rank...), "invalid rank of operands");

    static constexpr bool is_writable = all(expr_traits<Es>::is_writable...);
    static constexpr bool is_view = false;
    using value_type = std::tuple<expr_value_type<Es>...>;
};

template<typename... Es, typename D>
struct expr_cursor<zip_expr<Es...>, D> {
    using type = zip_cursor<expr_cursor_type<Es, D>...>;
};

template<typename... Es>
struct zip_expr: expr<zip_expr<Es...>> {
    template<typename... Cs>
    friend struct zip_cursor;

    zip_expr(Es... operands) : operands_(std::move(operands)...) {
        //
    }

    CAPYBARA_INLINE
    index_t dimension_impl(index_t axis) const {
        return std::get<0>(operands_).dimension(axis);
    }

    std::tuple<Es...> operands_;
};

//
template<typename... Cs>
struct zip_cursor {
    using value_type = std::tuple<decltype(std::declval<Cs>().load())...>;

    template<typename... Es, typename D>
    zip_cursor(const zip_expr<Es...>& base, D device) :
        operands_(seq::map(base.operands_, [&device](auto arg) {
            return arg.cursor(device);
        })) {
        //
    }

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

    std::tuple<Cs...> operands_;
};

template<typename... Es>
using expr_zip_type = zip_expr<into_expr_type<Es>...>;

template<typename... Es>
expr_zip_type<Es...> zip(Es&&... args) {
    return zip_expr<into_expr_type<Es>...>(
        into_expr(std::forward<Es>(args))...);
}

}  // namespace capybara