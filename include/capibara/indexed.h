#include <complex>
#pragma once

namespace capibara {

template<size_t N>
struct IndexedCursor;

template<typename D>
struct ExprTraits<IndexedExpr<D>> {
    static constexpr size_t rank = D::size();
    using value_type = std::array<size_t, rank>;
    using index_type = size_t;
    using cursor_type = IndexedCursor<rank>;
    using nested_type = IndexedExpr<D>;
};

template<typename D>
struct IndexedExpr: Expr<IndexedExpr<N>> {
    friend IndexedCursor<N>;

    IndexedExpr(D dims) : dims_(std::move(dims)) {}

    template<typename Axis>
    CAPIBARA_INLINE auto dim(Axis axis) const {
        return dims_[axis];
    }

  private:
    D dims_;
};

template<typename N>
struct IndexedCursor {
    using value_type = std::array<size_t, N>;

    IndexedCursor(const expr_type& e) {
        for (size_t i = 0; i < N; i++) {
            index_[i] = 0;
        }
    }

    template<typename Axis, typename Diff>
    CAPIBARA_INLINE void advance(Axis axis, Diff diff) {
        index[axis] += diff;
    }

    CAPIBARA_INLINE
    value_type eval() const {
        return index_;
    }

  private:
    value_type index_;
};

}  // namespace capibara