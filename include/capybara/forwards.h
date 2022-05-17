#pragma once
#include <type_traits>

#include "util.h"

namespace capybara {

template<typename D>
struct expr;

template<typename L, typename S>
struct array_base;

template<typename F, typename... Es>
struct apply_expr;

template<typename F, size_t N = 0>
struct nullary_expr;

template<typename... Es>
struct zip_expr;

template<typename C, typename... Es>
struct select_expr;

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
struct expr_cursor;

template<typename E, typename D>
using expr_cursor_type = typename expr_cursor<expr_nested_type<E>, D>::type;

template<typename E>
static constexpr bool is_expr =
    std::is_base_of<expr<decay_t<E>>, decay_t<E>>::value;

template<typename E, typename = void>
struct expr_conversion {};

template<typename E>
struct expr_conversion<E, enable_t<is_expr<E>>> {
    using type = expr_nested_type<E>;

    static type call(E&& expr) {
        return type(std::forward<E>(expr));
    }
};

template<typename E>
using into_expr_type = typename expr_conversion<E>::type;

template<typename E>
into_expr_type<E> into_expr(E&& expr) {
    return expr_conversion<E>::call(std::forward<E>(expr));
}

template<typename E>
using expr_value_type = typename expr_traits<into_expr_type<E>>::value_type;

struct device_seq {};

}  // namespace capybara