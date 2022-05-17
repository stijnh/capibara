#pragma once

#include "apply.h"

namespace capybara {

#define CAPYBARA_IMPL_BINARY_GENERAL(op, name, return_type)            \
    namespace functors {                                               \
        template<typename A, typename B>                               \
        struct name {                                                  \
            using type = return_type;                                  \
                                                                       \
            type operator()(A lhs, B rhs) {                            \
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

}  // namespace capybara
