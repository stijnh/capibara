#pragma once

#include "defines.h"
#include "expr.h"
#include "forwards.h"
#include "types.h"
#include "view/combine.h"
#include "view/identity.h"
#include "view/insert_remove.h"
#include "view/permutate.h"
#include "view/slice.h"
#include "view/split_merge.h"

namespace capybara {

template<typename M, typename E>
struct ViewExpr;

template<typename M, typename E>
struct ViewCursor;

template <typename M, typename E>
struct ExprTraits<ViewExpr<M, E>> {
    static constexpr size_t rank = M::new_rank;
    using Value = typename ExprTraits<E>::Value;
    using Index = typename ExprTraits<E>::Index;
    using Nested = ViewExpr<M, E>;
    using Cursor = ViewCursor<M, E>;
};


template<typename M, typename E>
struct ViewExpr: Expr<ViewExpr<M, E>> {
    friend ViewCursor<M, E>;
    static constexpr size_t rank = M::new_rank;
    using Base = Expr<ViewExpr<M, E>>;
    using Nested = typename ExprTraits<E>::Nested;

    ViewExpr(M mapping, const E& expr) :
        mapping_(std::move(mapping)),
        inner_(expr) {}

    template<typename Axis>
    CAPYBARA_INLINE auto dim_impl(Axis axis) const {
        auto axis_ = into_axis<rank>(axis);
        auto delegate = [&](auto a) { return inner_.dim(a); };
        return mapping_.dim(delegate, axis_);
    }

    auto cursor() const {
        auto cursor = inner_.cursor();

        auto advance_delegate = [&](auto a, auto d) {
            return cursor.advance(a, d);
        };
        auto dim_delegate = [&](auto a) { return inner_.dim(a); };
        auto mapping = mapping_.cursor(advance_delegate, dim_delegate);

        return ViewCursor<decltype(mapping), E>(
            std::move(cursor),
            std::move(mapping));
    }

  private:
    M mapping_;
    Nested inner_;
};

template<typename M, typename E>
struct ViewCursor {
    static constexpr size_t rank = M::new_rank;
    using Value = typename E::Value;
    using Cursor = typename E::Cursor;

    ViewCursor(Cursor cursor, M mapping) :
        cursor_(std::move(cursor)),
        mapping_(std::move(mapping)) {}

    template<typename Axis, typename Diff>
    void advance(Axis axis, Diff steps) {
        auto axis_ = into_axis<rank>(axis);

        auto delegate = [&](auto a, auto d) { return cursor_.advance(a, d); };
        return mapping_.advance(delegate, axis_, steps);
    }

    Value eval() const {
        return cursor_.eval();
    }

  private:
    Cursor cursor_;
    M mapping_;
};

template<typename E, typename M>
ViewExpr<M, E> make_view_expr(const Expr<E>& expr, M mapping) {
    return ViewExpr<M, E>(mapping, expr.self());
}

}  // namespace capybara