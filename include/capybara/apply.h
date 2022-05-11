#pragma once
#include <type_traits>

#include "broadcast.h"
#include "expr.h"
#include "forwards.h"
#include "functional.h"
#include "tuple.h"
#include "util.h"

namespace capybara {

namespace detail {
    template<size_t i, typename Es, typename = void>
    struct CollectConstDims;

    template<size_t i, typename First, typename... Rest>
    struct CollectConstDims<
        i,
        Tuple<First, Rest...>,
        enable_t<is_expr_dim_const<First, i>>>: ExprConstDim<First, i> {};

    template<size_t i, typename First, typename... Rest>
    struct CollectConstDims<
        i,
        Tuple<First, Rest...>,
        enable_t<!is_expr_dim_const<First, i>>>:
        CollectConstDims<i, Tuple<Rest...>> {};

    template<size_t i>
    struct CollectConstDims<i, Tuple<>> {};

}  // namespace detail

template<typename F, typename... Es>
struct ApplyExpr;

template<typename F, typename... Cs>
struct ApplyCursor;

template<size_t i, typename F, typename... Es>
struct ExprConstDim<ApplyExpr<F, Es...>, i>:
    detail::CollectConstDims<i, Tuple<Es...>> {};

template<typename F, typename... Es>
struct ExprTraits<ApplyExpr<F, Es...>> {
    static constexpr size_t rank =
        ExprTraits<detail::TupleElementAt<0, Es...>>::rank;
    static_assert(
        functional::all((ExprTraits<Es>::rank == rank)...),
        "rank mismatch");

    static constexpr bool is_view = false;
    static constexpr bool is_readable =
        functional::all(ExprTraits<typename Es::Nested>::is_readable...);
    static constexpr bool is_writeable = false;
    using Cursor =
        ApplyCursor<F, typename ExprTraits<typename Es::Nested>::Cursor...>;
    using Value = typename Cursor::Value;
};

template<typename F, typename... Es>
struct ApplyExpr: Expr<ApplyExpr<F, Es...>> {
    template<typename F2, typename... Cs>
    friend struct ApplyCursor;

    using Base = Expr<ApplyExpr<F, Es...>>;
    using Base::rank;

    ApplyExpr(F fun, Es... args) :
        fun_(std::move(fun)),
        args_(std::move(args)...) {
        auto dims = std::get<0>(args_).dims();

        bool same_dimensions = seq::all_n<rank>([&](auto axis) {
            return seq::all(args_, [&](const auto& arg) {
                return arg.dim(axis) == dims[axis];
            });
        });

        if (!same_dimensions) {
            throw std::runtime_error("dimensions mismatch");
        }
    }

    ApplyExpr(Es... args) : ApplyExpr({}, std::move(args)...) {}

    template<typename Axis>
    CAPYBARA_INLINE index_t dim_impl(Axis axis) const {
        return std::get<0>(args_).dim(axis);
    }

  private:
    F fun_;
    Tuple<const Es...> args_;
};

template<typename F, typename... Cs>
struct ApplyCursor {
    using Value = apply_t<F, decltype(std::declval<Cs&>().eval())...>;

    template<typename... Es>
    CAPYBARA_INLINE ApplyCursor(const ApplyExpr<F, Es...>& e) :
        fun_(e.fun_),
        args_(seq::map(e.args_, [](const auto& e) { return e.cursor(); })) {}

    template<typename Axis, typename Steps>
    CAPYBARA_INLINE void advance(Axis axis, Steps steps) {
        args_.visit_all([=](auto& c) { c.advance(axis, steps); });
    }

    CAPYBARA_INLINE
    Value eval() {
        return eval_helper(std::index_sequence_for<Cs...> {});
    }

  private:
    template<size_t... indices>
    CAPYBARA_INLINE Value eval_helper(std::index_sequence<indices...>) {
        return fun_(std::get<indices>(args_).eval()...);
    }

  private:
    F fun_;
    Tuple<Cs...> args_;
};

template<typename F, typename... Es>
ApplyExpr<F, BroadcastTo<Es, Es...>...> map(F fun, Es&&... exprs) {
    auto dims = largest_dims(exprs...);
    return {fun, broadcast(std::forward<Es>(exprs), dims)...};
}

}  // namespace capybara