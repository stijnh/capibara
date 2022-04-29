#include <complex>
#pragma once

namespace capibara {

template<size_t N, typename I>
struct IndexedCursor;

template<typename D, typename I>
struct ExprTraits<IndexedExpr<D, I>> {
    static constexpr size_t rank = D::size();
    using value_type = std::array<size_t, rank>;
    using index_type = size_t;
    using cursor_type = IndexedCursor<rank, I>;
    using nested_type = IndexedExpr<D, I>;
};

template<typename D, typename I>
struct IndexedExpr: Expr<IndexedExpr<D, I>> {
    friend IndexedCursor<D::rank, I>;

    IndexedExpr(D dims) : dims_(std::move(dims)) {}

    template<typename Axis>
    CAPIBARA_INLINE auto dim(Axis axis) const {
        return dims_[axis];
    }

  private:
    D dims_;
};

template<size_t N, typename I>
struct IndexedCursor {
    using index_type = I;
    using value_type = std::array<index_type, N>;

    template<typename E>
    IndexedCursor(const E& e) {
        for (size_t i = 0; i < N; i++) {
            index_[i] = 0;
        }
    }

    template<typename Axis, typename Diff>
    CAPIBARA_INLINE void advance(Axis axis, Diff diff) {
        index_[axis] += diff;
    }

    CAPIBARA_INLINE
    value_type eval() const {
        return index_;
    }

  private:
    value_type index_;
};

template<typename... D>
IndexedExpr<dims_type<D...>> arange(D... dim) {
    return IndexedExpr<dims_type<D...>>(dims(dim...));
}

template<typename E>
auto indexed(const Expr<E>& expr) {
    return zip(arange(expr.dims()), expr);
}

}  // namespace capibara