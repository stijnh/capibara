#pragma once
#include <tuple>

#include "expr.h"

namespace capybara {
template<typename C, typename... Cs>
struct select_cursor;

template<typename C, typename... Es>
struct expr_traits<select_expr<C, Es...>> {
    static constexpr size_t rank = C::rank;
    static_assert(fun::all(Es::rank == rank...), "invalid rank of operands");

    static constexpr bool is_writable = false;
    static constexpr bool is_view = false;
    using value_type = typename std::common_type<expr_value_type<Es>...>::type;
};

template<typename C, typename... Es, typename D>
struct expr_cursor<select_expr<C, Es...>, D> {
    using type =
        select_cursor<expr_cursor_type<C, D>, expr_cursor_type<Es, D>...>;
};

template<typename C, typename... Es>
struct select_expr: expr<select_expr<C, Es...>> {
    select_expr(C selector, Es... operands) :
        selector_(std::move(selector)),
        operands_(std::move(operands)...) {
        //
    }

    CAPYBARA_INLINE
    index_t dimension_impl(index_t axis) const {
        return std::get<0>(operands_).dimension(axis);
    }

    C selector_;
    std::tuple<Es...> operands_;
};

namespace {
    template<typename T, typename Is>
    struct selector_helper;

    template<typename T, size_t I, size_t J, size_t... Rest>
    struct selector_helper<T, std::index_sequence<I, J, Rest...>> {
        template<typename S, typename Tuple>
        static T call(S selection, Tuple& tuple) {
            return selection == I
                ? std::get<I>(tuple).load()
                : selector_helper<T, std::index_sequence<J, Rest...>>::call(
                    selection,
                    tuple);
        }
    };

    template<typename T, size_t I>
    struct selector_helper<T, std::index_sequence<I>> {
        template<typename S, typename Tuple>
        static T call(S selection, Tuple& tuple) {
            return std::get<I>(tuple).load();
        }
    };
}  // namespace

//
template<typename C, typename... Cs>
struct select_cursor {
    using value_type =
        typename std::common_type<decltype(std::declval<Cs>().load())...>::type;

    template<typename E, typename... Es, typename D>
    select_cursor(const select_expr<E, Es...>& base, D device) :
        selector_(base.selector_.cursor(device)),
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
        static constexpr size_t N = sizeof...(Cs);
        return selector_helper<value_type, std::make_index_sequence<N>>::call(
            selector_.load(),
            operands_);
    }

  private:
    C selector_;
    std::tuple<Cs...> operands_;
};

template<typename C, typename... Es>
using expr_select_type = select_expr<into_expr_type<C>, into_expr_type<Es>...>;

template<typename C, typename... Es>
expr_select_type<C, Es...> select(C&& selector, Es&&... args) {
    return expr_select_type<C, Es...>(
        into_expr(std::forward<C>(selector)),
        into_expr(std::forward<Es>(args))...);
}

}  // namespace capybara