#pragma once

#include "slice.h"
#include "unary.h"

namespace capibara {

template<typename Derived, size_t Rank>
struct ExprExtra {
    template<typename Index>
    CAPIBARA_INLINE auto operator[](Index index) {
        return ((const Derived*)this)->remove_axis(index);
    }
};

template<typename Derived>
struct ExprExtra<Derived, 0> {
    using value_type = expr_value_type<Derived>;

    CAPIBARA_INLINE
    value_type operator()() {
        return ((const Derived*)this)->eval({});
    }
};

template<typename Derived>
struct ExprExtra<Derived, 1> {
    using value_type = expr_value_type<Derived>;

    template<typename Index>
    CAPIBARA_INLINE value_type operator[](Index index) {
        return ((const Derived*)this)->remove_axis(Axis0, index).eval({});
    }

    CAPIBARA_INLINE
    auto reverse() {
        return ((const Derived*)this)->reverse_axis(Axis0);
    }
};

template<typename Derived>
struct ExprExtra<Derived, 2> {
    using value_type = expr_value_type<Derived>;

    CAPIBARA_INLINE
    auto row(size_t i) const {
        return ((const Derived*)this)->slice_axis(Axis0, i);
    }

    CAPIBARA_INLINE
    auto col(size_t i) const {
        return ((const Derived*)this)->slice_axis(Axis1, i);
    }

    CAPIBARA_INLINE
    size_t nrows(size_t i) const {
        return ((const Derived*)this)->dim(Axis0);
    }

    CAPIBARA_INLINE
    size_t ncols(size_t i) const {
        return ((const Derived*)this)->dim(Axis1);
    }

    template<typename Index>
    CAPIBARA_INLINE auto operator[](Index index) {
        return ((const Derived*)this)->remove_axis(Axis0, index);
    }
};

template<typename Derived>
struct Expr<Derived, AccessMode::ReadOnly>:
    ExprExtra<Derived, ExprTraits<Derived>::rank> {
    using self_type = Derived;
    using expr_traits = ExprTraits<self_type>;
    static constexpr AccessMode access_mode = AccessMode::ReadOnly;
    static constexpr size_t rank = expr_traits::rank;
    using value_type = typename expr_traits::value_type;
    using index_type = typename expr_traits::index_type;
    using ndindex_type = std::array<index_type, rank>;

    CAPIBARA_INLINE
    const self_type& self() const {
        return *static_cast<const self_type*>(this);
    }

    CAPIBARA_INLINE
    self_type& self() {
        return *static_cast<self_type*>(this);
    }

    ndindex_type shape() const {
        ndindex_type result;
        for (size_t i = 0; i < rank; i++) {
            result[i] = self().dim(i);
        }
        return result;
    }

    bool empty() const {
        for (size_t i = 0; i < rank; i++) {
            if (self().dim(i) == 0)
                return true;
        }

        return false;
    }

    size_t size() const {
        size_t result = 1;
        for (size_t i = 0; i < rank; i++) {
            result *= self().dim(i);
        }
        return result;
    }

    auto dims() const {
        std::array<index_type, rank> output {};
        for (size_t i = 0; i < rank; i++) {
            output[i] = self().dim(i);
        }
        return output;
    }

    template<typename F>
    CAPIBARA_INLINE UnaryExpr<F, Derived> map(F fun) const {
        return UnaryExpr<F, Derived>(fun, self());
    }

    template<typename R>
    auto cast() const {
        return map(unary_functors::cast<value_type, R> {});
    }

    template<typename Derived2, typename Op>
    auto zipWith(const Expr<Derived2>& rhs, Op op) const {
        return BinaryExpr<Op, Derived, Derived2>(op, self(), rhs.self());
    }

    template<typename AxisA, typename AxisB>
    auto swap_axes(AxisA raw_a, AxisB raw_b) const {
        auto a = into_axis(raw_a);
        auto b = into_axis(raw_b);
        auto op = mapping::SwapAxes<decltype(a), decltype(b), rank>(a, b);
        return make_mapping_expr(op, self());
    }

    auto diagonal() const {
        return make_mapping_expr(mapping::Diagonal<rank>(), self());
    }

    auto transpose() const {
        return make_mapping_expr(mapping::ReverseAxes<rank>(), self());
    }

    template<typename Axis>
    auto reverse_axis(Axis axis) const {
        auto axis_ = into_axis(axis);
        auto dim = self().dim(axis_);
        auto op = mapping::ReverseAxis<decltype(axis_), decltype(dim), rank>(
            axis_,
            dim);
        return make_mapping_expr(op, self());
    }

    template<typename Axis, typename Slice>
    auto slice_axis(Axis axis, Slice slice) const {
        return make_slice_expr(self(), axis, slice);
    }

    template<typename Axis, typename Start, typename End>
    auto slice_axis(Axis axis, Start start, End end) const {
        return make_slice_expr(self(), axis, range(start, end));
    }

    template<typename Axis, typename Start, typename End, typename Stride>
    auto slice_axis(Axis axis, Start start, End end, Stride stride) const {
        return make_slice_expr(self(), axis, range(start, end, stride));
    }

    template<typename Axis, typename Index>
    auto remove_axis(Axis axis, Index index) const {
        return make_slice_expr(
            self(),
            axis,
            convert_integer<index_type>(index));
    }

    template<typename Axis>
    auto remove_axis(Axis axis) const {
        return make_slice_expr(self(), axis, convert_integer<index_type>(S0));
    }

    template<typename Axis, typename Index>
    auto insert_axis(Axis axis, Index size) const {
        return make_slice_expr(
            self(),
            axis,
            newaxis(convert_integer<index_type>(size)));
    }

    template<typename Axis>
    auto insert_axis(Axis axis) const {
        return make_slice_expr(self(), axis, newaxis);
    }

    template<typename... Slices>
    auto slice(Slices... slices) const {
        return make_slices_expr(self(), slices...);
    }
};

template<typename Derived>
struct Expr<Derived, AccessMode::ReadWrite>:
    Expr<Derived, AccessMode::ReadOnly> {
    static constexpr AccessMode access_mode = AccessMode::ReadWrite;
    using base_type = Expr<Derived>;
    using base_type::rank;
    using base_type::self;
    using typename base_type::index_type;
    using typename base_type::ndindex_type;
    using typename base_type::value_type;

    const value_type& operator()(ndindex_type idx) const {
        return self().access(idx);
    }

    template<typename... Is>
    const value_type& operator()(Is... idxs) const {
        ndindex_type idx = {idxs...};
        return self().access(idx);
    }

    value_type& operator()(ndindex_type idx) {
        return this->self().access(idx);
    }

    template<typename... Is>
    value_type& operator()(Is... idxs) {
        ndindex_type idx = {idxs...};
        return self().access(idx);
    }

    value_type eval(ndindex_type idx) const {
        return self().access(idx);
    }

    void store(ndindex_type idx, value_type v) {
        self().access(idx) = std::move(v);
    }

    template<typename Derived2>
    Derived& operator=(const Expr<Derived2>& rhs) {
        return self();
    }
};

}  // namespace capibara