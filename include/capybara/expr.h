#pragma once

#include "axes.h"
#include "slice.h"
#include "unary.h"

namespace capybara {

template<typename Derived, size_t Rank>
struct ExprExtra {
    template<typename Index>
    CAPYBARA_INLINE auto operator[](Index index) {
        return ((const Derived*)this)->remove_axis(index);
    }
};

template<typename Derived>
struct ExprExtra<Derived, 0> {
    using Value = ExprValue<Derived>;

    CAPYBARA_INLINE
    Value operator()() {
        return ((const Derived*)this)->eval({});
    }
};

template<typename Derived>
struct ExprExtra<Derived, 1> {
    using Value = ExprValue<Derived>;

    template<typename Index>
    CAPYBARA_INLINE Value operator[](Index index) {
        return ((const Derived*)this)->remove_axis(axis0, index).eval({});
    }

    CAPYBARA_INLINE
    auto reverse() {
        return ((const Derived*)this)->reverse_axis(axis0);
    }
};

template<typename Derived>
struct ExprExtra<Derived, 2> {
    using Value = ExprValue<Derived>;

    CAPYBARA_INLINE
    auto row(size_t i) const {
        return ((const Derived*)this)->slice_axis(axis0, i);
    }

    CAPYBARA_INLINE
    auto col(size_t i) const {
        return ((const Derived*)this)->slice_axis(axis0, i);
    }

    CAPYBARA_INLINE
    size_t nrows(size_t i) const {
        return ((const Derived*)this)->dim(axis0);
    }

    CAPYBARA_INLINE
    size_t ncols(size_t i) const {
        return ((const Derived*)this)->dim(axis1);
    }

    template<typename Index>
    CAPYBARA_INLINE auto operator[](Index index) {
        return ((const Derived*)this)->remove_axis(axis0, index);
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

    CAPYBARA_INLINE
    const Self& self() const {
        return *static_cast<const Self*>(this);
    }

    CAPYBARA_INLINE
    Self& self() {
        return *static_cast<Self*>(this);
    }

    CAPYBARA_INLINE Cursor cursor() const {
        return Cursor(self());
    }

    NdIndex shape() const {
        NdIndex result;
        for (size_t i = 0; i < rank; i++) {
            result[i] = dim(i);
        }
        return result;
    }

    size_t size() const {
        size_t result = 1;
        for (size_t i = 0; i < rank; i++) {
            result *= dim(i);
        }
        return result;
    }

    bool empty() const {
        return size() == 0;
    }

    template <typename Axis>
    auto dim(Axis axis) const {
        return convert_Size(self().dim_impl(into_axis<rank>(axis)));
    }

    auto dims() const {
        return dims(axes::seq<rank> {});
    }

    template<size_t... I>
    auto dims(AxesOrder<I...>) const {
        return capybara::dims(dim(Axis<I> {})...);
    }

    template<typename F>
    CAPYBARA_INLINE UnaryExpr<F, Derived> map(F fun) const {
        return UnaryExpr<F, Derived>(fun, self());
    }

    template<typename R>
    auto cast() const {
        return map(unary_functors::cast<Value, R> {});
    }

    template<typename Derived2, typename Op>
    auto zip_with(const Expr<Derived2>& rhs, Op op) const {
        return BinaryExpr<Op, Derived, Derived2>(op, self(), rhs.self());
    }

    template<typename AxisA, typename AxisB>
    auto swap_axes(AxisA raw_a, AxisB raw_b) const {
        auto a = into_axis<rank>(raw_a);
        auto b = into_axis<rank>(raw_b);
        view::Swap<rank, decltype(a), decltype(b)> op {a, b};
        return make_mapping_expr(self(), op);
    }

    auto diagonal() const {
        CAPYBARA_TODO("unimplemented");
    }

    auto transpose() const {
        return make_mapping_expr(self(), view::Transpose<rank> {});
    }

    template<typename Axis>
    auto reverse_axis(Axis axis) const {
        auto axis_ = into_axis<rank>(axis);
        auto dim = this->dim(axis_);
        CAPYBARA_TODO("unimplemented");
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

    template<typename Axis, typename Idx>
    auto remove_axis(Axis axis, Idx index) const {
        return make_slice_expr(self(), axis, convert_integer<Index>(index));
    }

    template<typename Axis>
    auto remove_axis(Axis axis) const {
        return make_slice_expr(self(), axis, convert_integer<Index>(S0));
    }

    template<typename Axis, typename Size>
    auto insert_axis(Axis axis, Size size) const {
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
        return make_slices_expr(self(), std::forward<Slices>(slices)...);
    }

    template<typename... Slices>
    auto operator()(Slices... slices) const {
        return slice(std::forward<Slices>(slices)...);
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

    template<typename Derived2>
    Derived& operator=(const Expr<Derived2>& rhs) {
        return self();
    }
};

}  // namespace capybara