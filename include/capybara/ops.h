#pragma once

#include "apply.h"
#include "forwards.h"
#include "nullary.h"
#include "util.h"

namespace capybara {

#define CAPYBARA_IMPL_UNARY_OP(name, op)                                \
    namespace functors {                                                \
        template<typename A>                                            \
        struct name {                                                   \
            using type = decltype(op std::declval<A>());                \
                                                                        \
            type operator()(const A& arg) {                             \
                return op arg;                                          \
            }                                                           \
        };                                                              \
    }                                                                   \
                                                                        \
    template<typename A, typename = enable_t<is_expr<A>>>               \
    ApplyExpr<functors::name<typename IntoExpr<A>::Value>, IntoExpr<A>> \
    operator op(A&& arg) {                                              \
        return {std::forward<A>(arg)};                                  \
    }

CAPYBARA_IMPL_UNARY_OP(Not, !)
CAPYBARA_IMPL_UNARY_OP(BitNot, ~)
CAPYBARA_IMPL_UNARY_OP(Negate, -)
CAPYBARA_IMPL_UNARY_OP(Plus, +)
#undef CAPYBARA_IMPL_UNARY_OP

#define CAPYBARA_IMPL_BINARY_OP(name, op)                                   \
    namespace functors {                                                    \
        template<typename A, typename B>                                    \
        struct name {                                                       \
            using type = decltype(std::declval<A>() op std::declval<B>());  \
                                                                            \
            type operator()(const A& lhs, const B& rhs) {                   \
                return lhs op rhs;                                          \
            }                                                               \
        };                                                                  \
    }                                                                       \
                                                                            \
    template<                                                               \
        typename A,                                                         \
        typename B,                                                         \
        typename = enable_t<is_expr<A> || is_expr<B>>>                      \
    ApplyExpr<                                                              \
        functors::                                                          \
            name<typename IntoExpr<A>::Value, typename IntoExpr<B>::Value>, \
        BroadcastTo<A, A, B>,                                               \
        BroadcastTo<B, A, B>>                                               \
    operator op(A&& lhs, B&& rhs) {                                         \
        using F = functors::                                                \
            name<typename IntoExpr<A>::Value, typename IntoExpr<B>::Value>; \
        return map(F {}, std::forward<A>(lhs), std::forward<B>(rhs));       \
    }

CAPYBARA_IMPL_BINARY_OP(Add, +)
CAPYBARA_IMPL_BINARY_OP(Sub, -)
CAPYBARA_IMPL_BINARY_OP(Mul, *)
CAPYBARA_IMPL_BINARY_OP(Div, /)
CAPYBARA_IMPL_BINARY_OP(BitOr, |)
CAPYBARA_IMPL_BINARY_OP(BitXor, ^)
CAPYBARA_IMPL_BINARY_OP(BitAnd, &)
#undef CAPYBARA_IMPL_BINARY_OP

#define CAPYBARA_IMPL_BINARY_CMP(name, op, fun)                             \
    namespace functors {                                                    \
        template<typename A, typename B>                                    \
        struct name {                                                       \
            using type = bool;                                              \
                                                                            \
            type operator()(const A& lhs, const B& rhs) {                   \
                return fun(lhs, rhs);                                       \
            }                                                               \
        };                                                                  \
    }                                                                       \
                                                                            \
    template<                                                               \
        typename A,                                                         \
        typename B,                                                         \
        typename = enable_t<is_expr<A> || is_expr<B>>>                      \
    ApplyExpr<                                                              \
        functors::                                                          \
            name<typename IntoExpr<A>::Value, typename IntoExpr<B>::Value>, \
        BroadcastTo<A, A, B>,                                               \
        BroadcastTo<B, A, B>>                                               \
    operator op(A&& lhs, B&& rhs) {                                         \
        using F = functors::                                                \
            name<typename IntoExpr<A>::Value, typename IntoExpr<B>::Value>; \
        return map(F {}, std::forward<A>(lhs), std::forward<B>(rhs));       \
    }

CAPYBARA_IMPL_BINARY_CMP(Equals, ==, cmp_equal)
CAPYBARA_IMPL_BINARY_CMP(NotEquals, !=, cmp_not_equal)
CAPYBARA_IMPL_BINARY_CMP(Less, <, cmp_less)
CAPYBARA_IMPL_BINARY_CMP(Greater, >, cmp_greater)
CAPYBARA_IMPL_BINARY_CMP(LessEqual, <=, cmp_less_equal)
CAPYBARA_IMPL_BINARY_CMP(GreaterEqual, >=, cmp_greater_equal)
#undef CAPYBARA_IMPL_BINARY_CMP

}  // namespace capybara