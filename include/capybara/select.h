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
struct expr_cursor<const select_expr<C, Es...>, D> {
    using type =
        select_cursor<expr_cursor_type<C, D>, expr_cursor_type<Es, D>...>;
    static constexpr size_t rank = expr_rank<C, Es...>;

    template<size_t... Is>
    CAPYBARA_INLINE static type call_helper(
        const select_expr<C, Es...>& expr,
        dshape<rank> shape,
        D device,
        std::index_sequence<Is...>) {
        return type(
            expr.selector().cursor(shape, device),
            std::get<Is>(expr.operands()).cursor(shape, device)...);
    }

    CAPYBARA_INLINE
    static type
    call(const select_expr<C, Es...>& expr, dshape<rank> shape, D device) {
        return call_helper(
            expr,
            shape,
            device,
            std::index_sequence_for<Es...> {});
    }
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
        index_t result = selector_.dimension(axis);

        if (result == 1) {
            result =
                dimension_broadcast<std::tuple<Es...>>::call(axis, operands_);
            ;
        }

        return result;
    }

    CAPYBARA_INLINE
    const C& selector() const {
        return selector_;
    }

    CAPYBARA_INLINE
    const std::tuple<Es...>& operands() const {
        return operands_;
    }

  private:
    C selector_;
    std::tuple<Es...> operands_;
};

namespace {
    template<typename T, typename Is>
    struct selector_helper;

    template<typename T, size_t I, size_t J, size_t... Rest>
    struct selector_helper<T, std::index_sequence<I, J, Rest...>> {
        template<typename S, typename Tuple>
        CAPYBARA_INLINE static T load(S selection, Tuple& tuple) {
            return selection == I
            ? std::get<I>(tuple).load()
            : selector_helper<T, std::index_sequence<J, Rest...>>::call(
                    selection,
                    tuple);
        }

        template<typename S, typename Tuple>
        CAPYBARA_INLINE static void store(S selection, Tuple& tuple, T value) {
            return selection == I
            ? std::get<I>(tuple).store(std::move(value))
            : selector_helper<T, std::index_sequence<J, Rest...>>::store(
                    selection,
                    tuple,
                    std::move(value));
        }
    };

    template<typename T, size_t I>
    struct selector_helper<T, std::index_sequence<I>> {
        template<typename S, typename Tuple>
        CAPYBARA_INLINE static T load(S selection, Tuple& tuple) {
            return std::get<I>(tuple).load();
        }

        template<typename S, typename Tuple>
        CAPYBARA_INLINE static void store(S selection, Tuple& tuple, T value) {
            return std::get<I>(tuple).store(std::move(value));
        }
    };
}  // namespace

//
template<typename C, typename... Cs>
struct select_cursor {
    using value_type =
        typename std::common_type<decltype(std::declval<Cs>().load())...>::type;

    select_cursor(C selector, Cs... operands) :
        selector_(std::move(selector)),
        operands_(std::move(operands)...) {}

    CAPYBARA_INLINE
    void advance(index_t axis, index_t steps) {
        seq::for_each(operands_, [axis, steps](auto cursor) {
            cursor.advance(axis, steps);
        });
    }

    CAPYBARA_INLINE
    value_type load() {
        return selector_helper<value_type, std::index_sequence_for<Cs...>>::load(
                selector_.load(),
                operands_);
    }

    CAPYBARA_INLINE
    value_type store(value_type v) {
        return selector_helper<value_type, std::index_sequence_for<Cs...>>::store(
                selector_.load(),
                operands_,
                std::move(v));
    }

  private:
    C selector_;
    std::tuple<Cs...> operands_;
};

template<typename C, typename... Es>
using expr_select_type = select_expr<
    broadcast_expr_type<C, C, Es...>,
    broadcast_expr_type<Es, C, Es...>...>;

template<typename C, typename... Es>
expr_select_type<C, Es...> select(C&& selector, Es&&... args) {
    return expr_select_type<C, Es...>(
        broadcast_expr<C, C, Es...>(std::forward<C>(selector)),
        broadcast_expr<Es, C, Es...>(std::forward<Es>(args))...);
}

}  // namespace capybara