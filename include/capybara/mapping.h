#pragma once

#include "defines.h"
#include "expr.h"
#include "forwards.h"
#include "mapping/combine.h"
#include "mapping/identity.h"
#include "mapping/insert_remove.h"
#include "mapping/permutate.h"
#include "mapping/slice.h"
#include "mapping/split_merge.h"
#include "types.h"

namespace capybara {

template<typename M, typename E>
struct MappingCursor;

template<typename M, typename E>
struct MappingExpr: Expr<MappingExpr<M, E>> {
    friend MappingCursor<M, E>;
    static constexpr size_t rank = M::new_rank;
    using Base = Expr<MappingExpr<M, E>>;
    using Nested = typename Base::Nested;

    MappingExpr(M mapping, Nested expr) :
        mapping_(std::move(mapping)),
        inner_(expr) {}

    template<typename Axis>
    CAPYBARA_INLINE auto dim(Axis axis) const {
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

        return MappingCursor<decltype(mapping), E>(
            std::move(cursor),
            std::move(mapping));
    }

  private:
    M mapping_;
    Nested inner_;
};

template<typename M, typename E>
struct MappingCursor {
    static constexpr size_t rank = M::new_rank;
    using Value = typename E::Value;
    using Cursor = typename E::Cursor;

    MappingCursor(Cursor cursor, M mapping) :
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
MappingExpr<M, E> make_mapping_expr(const Expr<E>& expr, M mapping) {
    return MappingExpr<M, E>(mapping, expr.self());
}

}  // namespace capybara