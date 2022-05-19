#pragma once

#include <cmath>
#include <complex>

#include "apply.h"

namespace capybara {

#define CAPYBARA_IMPL_BINARY_GENERAL(op, name, return_type)            \
    namespace functors {                                               \
        template<typename A, typename B>                               \
        struct name {                                                  \
            using type = return_type;                                  \
                                                                       \
            CAPYBARA_INLINE                                            \
            type operator()(A lhs, B rhs) const {                      \
                return lhs op rhs;                                     \
            }                                                          \
        };                                                             \
    }                                                                  \
                                                                       \
    template<                                                          \
        typename A,                                                    \
        typename B,                                                    \
        typename = enable_t<is_expr<A> || is_expr<B>>>                 \
    expr_map_type<                                                     \
        functors::name<expr_value_type<A>, expr_value_type<B>>,        \
        A,                                                             \
        B>                                                             \
    operator op(A&& lhs, B&& rhs) {                                    \
        return map(                                                    \
            functors::name<expr_value_type<A>, expr_value_type<B>> {}, \
            std::forward<A>(lhs),                                      \
            std::forward<B>(rhs));                                     \
    }

#define CAPYBARA_IMPL_BINARY_OP(op, name) \
    CAPYBARA_IMPL_BINARY_GENERAL(         \
        op,                               \
        name,                             \
        decltype(std::declval<A>() op std::declval<B>()))
CAPYBARA_IMPL_BINARY_OP(+, add)
CAPYBARA_IMPL_BINARY_OP(-, subtract)
CAPYBARA_IMPL_BINARY_OP(/, divide)
CAPYBARA_IMPL_BINARY_OP(*, multiply)
CAPYBARA_IMPL_BINARY_OP(%, modulo)
CAPYBARA_IMPL_BINARY_OP(&, bitwise_and)
CAPYBARA_IMPL_BINARY_OP(|, bitwise_or)
CAPYBARA_IMPL_BINARY_OP(^, bitwise_xor)

#define CAPYBARA_IMPL_BINARY_CMP(op, name) \
    CAPYBARA_IMPL_BINARY_GENERAL(op, name, bool)
CAPYBARA_IMPL_BINARY_CMP(==, equal)
CAPYBARA_IMPL_BINARY_CMP(!=, not_equal)
CAPYBARA_IMPL_BINARY_CMP(<, less)
CAPYBARA_IMPL_BINARY_CMP(>, greater)
CAPYBARA_IMPL_BINARY_CMP(<=, less_equal)
CAPYBARA_IMPL_BINARY_CMP(>=, greater_equal)

#undef CAPYBARA_IMPL_BINARY_OP
#undef CAPYBARA_IMPL_BINARY_CMP
#undef CAPYBARA_IMPL_BINARY_GENERAL

namespace functors {
    template<typename From, typename To>
    struct cast {
        using type = To;

        To operator()(From value) const {
            return To {value};
        }
    };
}  // namespace functors

template<typename T, typename E>
CAPYBARA_INLINE expr_map_type<functors::cast<expr_value_type<E>, T>, E>
cast(E&& expr) {
    return map(
        functors::cast<expr_value_type<E>, T> {},
        into_expr(std::forward<E>(expr)));
}

#define CAPYBARA_IMPL_MATH_OP(name, fun)                                       \
    namespace functors {                                                       \
        template<typename T>                                                   \
        struct name {                                                          \
            using type = decltype(fun(std::declval<T>()));                     \
                                                                               \
            CAPYBARA_INLINE                                                    \
            type operator()(T value) const {                                   \
                return fun(value);                                             \
            }                                                                  \
        };                                                                     \
    }                                                                          \
                                                                               \
    template<typename E>                                                       \
    CAPYBARA_INLINE expr_map_type<functors::name<expr_value_type<E>>, E> name( \
        E&& expr) {                                                            \
        return map(                                                            \
            functors::name<expr_value_type<E>> {},                             \
            into_expr(std::forward<E>(expr)));                                 \
    }

CAPYBARA_IMPL_MATH_OP(sqrt, std::sqrt)
CAPYBARA_IMPL_MATH_OP(sin, std::sin)
CAPYBARA_IMPL_MATH_OP(cos, std::cos)
CAPYBARA_IMPL_MATH_OP(tan, std::tan)
CAPYBARA_IMPL_MATH_OP(asin, std::asin)
CAPYBARA_IMPL_MATH_OP(acos, std::acos)
CAPYBARA_IMPL_MATH_OP(atan, std::atan)
CAPYBARA_IMPL_MATH_OP(sinh, std::sinh)
CAPYBARA_IMPL_MATH_OP(cosh, std::cosh)
CAPYBARA_IMPL_MATH_OP(tanh, std::tanh)

CAPYBARA_IMPL_MATH_OP(exp, std::exp)
CAPYBARA_IMPL_MATH_OP(exp2, std::exp2)
CAPYBARA_IMPL_MATH_OP(log, std::log)
CAPYBARA_IMPL_MATH_OP(log10, std::log10)

CAPYBARA_IMPL_MATH_OP(ceil, std::ceil)
CAPYBARA_IMPL_MATH_OP(floor, std::floor)
CAPYBARA_IMPL_MATH_OP(round, std::round)
CAPYBARA_IMPL_MATH_OP(trunc, std::trunc)
CAPYBARA_IMPL_MATH_OP(abs, std::abs)

CAPYBARA_IMPL_MATH_OP(isinf, std::isinf)
CAPYBARA_IMPL_MATH_OP(isnan, std::isnan)
CAPYBARA_IMPL_MATH_OP(isnormal, std::isnormal)

CAPYBARA_IMPL_MATH_OP(real, std::real)
CAPYBARA_IMPL_MATH_OP(imag, std::imag)
CAPYBARA_IMPL_MATH_OP(conj, std::conj)
CAPYBARA_IMPL_MATH_OP(norm, std::norm)

#undef CAPYBARA_IMPL_MATH_OP

namespace functors {
    template<typename... Ts>
    struct maximum;

    template<typename A, typename B, typename... Ts>
    struct maximum<A, B, Ts...> {
        using type = typename std::common_type<A, B, Ts...>::type;

        CAPYBARA_INLINE
        type operator()(A first, B second, Ts... rest) const {
            type m = first > second ? first : second;
            return maximum<type, Ts...>()(m, rest...);
        }
    };

    template<typename T>
    struct maximum<T> {
        using type = T;

        CAPYBARA_INLINE
        type operator()(T result) const {
            return result;
        }
    };

    template<typename... Ts>
    struct minimum;

    template<typename A, typename B, typename... Ts>
    struct minimum<A, B, Ts...> {
        using type = typename std::common_type<A, B, Ts...>::type;

        CAPYBARA_INLINE
        type operator()(A first, B second, Ts... rest) const {
            type m = first < second ? first : second;
            return maximum<type, Ts...>()(m, rest...);
        }
    };

    template<typename T>
    struct minimum<T> {
        using type = T;

        CAPYBARA_INLINE
        type operator()(T result) const {
            return result;
        }
    };
}  // namespace functors

template<typename... Es>
CAPYBARA_INLINE expr_map_type<functors::minimum<expr_value_type<Es>...>, Es...>
minimum(Es&&... exprs) {
    return map(
        functors::minimum<expr_value_type<Es>...> {},
        into_expr(std::forward<Es>(exprs))...);
}

template<typename... Es>
CAPYBARA_INLINE expr_map_type<functors::maximum<expr_value_type<Es>...>, Es...>
maximum(Es&&... exprs) {
    return map(
        functors::maximum<expr_value_type<Es>...> {},
        into_expr(std::forward<Es>(exprs))...);
}

}  // namespace capybara
