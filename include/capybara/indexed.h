#include <complex>
#pragma once

namespace capybara {

template<size_t N, typename I>
struct IndexedCursor;

template<typename D, typename I>
struct ExprTraits<IndexedExpr<D, I>> {
    static constexpr size_t rank = D::size();
    using Value = std::array<size_t, rank>;
    using Index = size_t;
    using Cursor = IndexedCursor<rank, I>;
    using Nested = IndexedExpr<D, I>;
};

template<typename D, typename I>
struct IndexedExpr: Expr<IndexedExpr<D, I>> {
    friend IndexedCursor<D::rank, I>;

    IndexedExpr(D dims) : dims_(std::move(dims)) {}

    template<typename Axis>
    CAPYBARA_INLINE auto dim_impl(Axis axis) const {
        return dims_[axis];
    }

  private:
    D dims_;
};

template<size_t N, typename I>
struct IndexedCursor {
    using Index = I;
    using Value = std::array<Index, N>;

    template<typename E>
    IndexedCursor(const E& e) {
        for (size_t i = 0; i < N; i++) {
            index_[i] = 0;
        }
    }

    template<typename Axis, typename Diff>
    CAPYBARA_INLINE void advance(Axis axis, Diff diff) {
        index_[axis] += diff;
    }

    CAPYBARA_INLINE
    Value eval() const {
        return index_;
    }

  private:
    Value index_;
};

template<typename... D>
IndexedExpr<dims_type<D...>> arange(D... dim) {
    return IndexedExpr<dims_type<D...>>(dims(dim...));
}

template<typename E>
auto indexed(const Expr<E>& expr) {
    return zip(arange(expr.dims()), expr);
}

}  // namespace capybara