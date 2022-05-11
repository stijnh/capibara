#pragma once

#include "const_int.h"

namespace capybara {

template<index_t... sizes>
struct Dimensions;

enum AssignOp {};

template<typename T>
struct ExprTraits;

template<typename E>
struct ExprTraits<const E>: ExprTraits<E> {};

template<typename E, typename = void>
struct ExprNested {
    using type = E;
};

template<typename E>
struct ExprNested<const E>: ExprNested<E> {};

template<typename E>
struct ExprNested<E&&>: ExprNested<E> {};

template<typename E>
struct ExprNested<E&>: ExprNested<E> {};

template<typename Derived>
struct Expr;

template<typename T, typename D>
struct ArrayBase;

template<typename F, typename D = Dimensions<>>
struct ValueExpr;

template<typename F, typename D = Dimensions<>>
struct NullaryExpr;

template<typename Lhs, typename Rhs, AssignOp op>
struct AssignExpr;

template<size_t n, typename E>
struct BroadcastExpr;

template<typename E, size_t axis, typename = void>
struct ExprConstStride;

template<typename E, size_t axis>
struct ExprConstStride<const E, axis>: ExprConstStride<E, axis> {};

template<typename E, size_t axis, typename = void>
struct ExprConstDim;

template<typename E, size_t axis>
struct ExprConstDim<const E, axis>: ExprConstDim<E, axis> {};

namespace detail {
    template<size_t i, typename E, typename = void>
    struct ExprDimHelper {
        static constexpr bool is_constant = false;
        using type = index_t;
    };

    template<size_t i, typename E>
    struct ExprDimHelper<
        i,
        E,
        enable_t<(i < E::rank) && ExprConstDim<E, i>::value >= 0>> {
        static constexpr bool is_constant = true;
        static constexpr index_t value = ExprConstDim<E, i>::value;
        using type = ConstIndex<value>;
    };

    template<typename E>
    struct IsExprHelper {
        template<typename R>
        static R call(Expr<R>&);

        template<typename R>
        static const R call(const Expr<R>&);

        template<typename R>
        static R call(Expr<R>&&);

        template<typename R>
        static const R call(const Expr<R>&&);

        static void call(...);

        using type = decltype(call(std::declval<E>()));
        static constexpr bool value = !std::is_same<type, void>::value;
    };

    template<typename T, typename = void>
    struct IntoExprHelper {};

    template<typename T>
    struct IntoExprHelper<T, enable_t<std::is_arithmetic<T>::value>> {
        using type = ValueExpr<T>;

        static type call(T value) {
            return {std::move(value)};
        }
    };

    template<typename E>
    struct IntoExprHelper<
        const E,
        enable_t<std::is_arithmetic<const E>::value>>: IntoExprHelper<E> {};

    template<typename E>
    struct IntoExprHelper<E, enable_t<IsExprHelper<E>::value>> {
        using type = typename ExprNested<typename IsExprHelper<E>::type>::type;

        static type call(E& expr) {
            return expr.nested();
        }
    };

    template<typename E, typename = void>
    struct IntoExprConvertHelper: ConstFalse {};

    template<typename E>
    struct IntoExprConvertHelper<E, void_t<typename IntoExprHelper<E>::type>>:
        ConstTrue {};
}  // namespace detail

template<typename E, size_t axis>
static constexpr bool is_expr_dim_const =
    detail::ExprDimHelper<axis, E>::is_constant;

template<typename E, size_t axis>
static constexpr index_t expr_dim_const = detail::ExprDimHelper<axis, E>::value;

template<typename E, size_t axis>
using ExprDim = typename detail::ExprDimHelper<axis, E>::type;

template<typename E>
static constexpr bool is_expr = detail::IsExprHelper<E>::value;

template<typename E>
using IntoExpr = typename detail::IntoExprHelper<
    typename std::remove_reference<E>::type>::type;

template<typename E>
IntoExpr<E> into_expr(E&& expr) {
    return detail::IntoExprHelper<E>::call(std::forward<E>(expr));
}

}  // namespace capybara