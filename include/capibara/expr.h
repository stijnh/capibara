#pragma once

#include "axes.h"
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
    using Value = ExprValue<Derived>;

    CAPIBARA_INLINE
    Value operator()() {
        return ((const Derived*)this)->eval({});
    }
};

template<typename Derived>
struct ExprExtra<Derived, 1> {
    using Value = ExprValue<Derived>;

    template<typename Index>
    CAPIBARA_INLINE Value operator[](Index index) {
        return ((const Derived*)this)->remove_axis(Axis0, index).eval({});
    }

    CAPIBARA_INLINE
    auto reverse() {
        return ((const Derived*)this)->reverse_axis(Axis0);
    }
};

template<typename Derived>
struct ExprExtra<Derived, 2> {
    using Value = ExprValue<Derived>;

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
    using Self = Derived;
    using Traits = ExprTraits<Self>;
    static constexpr AccessMode access_mode = AccessMode::ReadOnly;
    static constexpr size_t rank = Traits::rank;

    using Value = typename Traits::Value;
    using Index = typename Traits::Index;
    using Cursor = typename Traits::Cursor;
    using Nested = typename Traits::Nested;
    using NdIndex = std::array<Index, rank>;

    CAPIBARA_INLINE
    const Self& self() const {
        return *static_cast<const Self*>(this);
    }

    CAPIBARA_INLINE
    Self& self() {
        return *static_cast<Self*>(this);
    }

    CAPIBARA_INLINE Cursor cursor() const {
        return Cursor(self());
    }

    NdIndex shape() const {
        NdIndex result;
        for (size_t i = 0; i < rank; i++) {
            result[i] = self().dim(i);
        }
        return result;
    }

    size_t size() const {
        size_t result = 1;
        for (size_t i = 0; i < rank; i++) {
            result *= self().dim(i);
        }
        return result;
    }

    bool empty() const {
        return size() == 0;
    }

    auto dims() const {
        return dims(axes::seq<rank> {});
    }

    template<size_t... I>
    auto dims(AxesOrder<I...>) const {
        return capibara::dims(self().dim(Axis<I> {})...);
    }

    template<typename F>
    CAPIBARA_INLINE UnaryExpr<F, Derived> map(F fun) const {
        return UnaryExpr<F, Derived>(fun, self());
    }

    template<typename R>
    auto cast() const {
        return map(unary_functors::cast<Value, R> {});
    }

    template<typename Derived2, typename Op>
    auto zipWith(const Expr<Derived2>& rhs, Op op) const {
        return BinaryExpr<Op, Derived, Derived2>(op, self(), rhs.self());
    }

    template<typename AxisA, typename AxisB>
    auto swap_axes(AxisA raw_a, AxisB raw_b) const {
        auto a = into_axis<rank>(raw_a);
        auto b = into_axis<rank>(raw_b);
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
        auto axis_ = into_axis<rank>(axis);
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
            convert_integer<Index>(index));
    }

    template<typename Axis>
    auto remove_axis(Axis axis) const {
        return make_slice_expr(self(), axis, convert_integer<Index>(S0));
    }

    template<typename Axis, typename Index>
    auto insert_axis(Axis axis, Index size) const {
        return make_slice_expr(
            self(),
            axis,
            newaxis(convert_integer<Index>(size)));
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
    using Base = Expr<Derived>;
    using Base::rank;
    using Base::self;
    using typename Base::NdIndex;
    using typename Base::Value;

    const Value& operator()(NdIndex idx) const {
        return self().access(idx);
    }

    template<typename... Is>
    const Value& operator()(Is... idxs) const {
        NdIndex idx = {idxs...};
        return self().access(idx);
    }

    Value& operator()(NdIndex idx) {
        return this->self().access(idx);
    }

    template<typename... Is>
    Value& operator()(Is... idxs) {
        NdIndex idx = {idxs...};
        return self().access(idx);
    }

    void store(NdIndex idx, Value v) {
        self().access(idx) = std::move(v);
    }

    template<typename Derived2>
    Derived& operator=(const Expr<Derived2>& rhs) {
        return self();
    }
};

}  // namespace capibara