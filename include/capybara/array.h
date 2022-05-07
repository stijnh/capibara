#pragma once

#include <memory>

#include "axis.h"
#include "dimensions.h"
#include "expr.h"
#include "forwards.h"

namespace capybara {

template<typename T, size_t N>
using Array = ArrayBase<T, DimensionsN<N>>;

template<typename T, size_t... sizes>
using FixedArray = ArrayBase<T, Dimensions<ConstIndex<sizes>...>>;

template <typename T, size_t I>
struct ExprStaticDimension;

template <typename E, typename Index, typename = void>
struct DimHelper;

template <typename E, size_t I>
struct DimHelper<E, ConstIndex<I>>: ExprStaticDimension<E, I> {
    static length_t call(const E& expr, ConstIndex<I>) {
        return ExprStaticDimension<E, I>::value;
    }
};

template <typename E>
struct DimHelper<E, DynAxis<E::rank>> {
    static length_t call(const E& expr, DynAxis<E::rank> index) {
        return expr.dim_impl(index);
    }
};


template<typename T, typename D>
struct ArrayRef;

template<typename T, typename D>
struct ArrayCursor;

template<typename T, typename D>
struct ExprTraits<ArrayBase<T, D>> {
    static constexpr size_t rank = D::rank;
    static constexpr AccessMode mode =
        std::is_const<T>::value ? AccessMode::ReadOnly : AccessMode::ReadWrite;
    static constexpr bool is_view = true;
    using Value = T;
    using Nested = ArrayRef<T, D>;
    using NestedConst = ArrayRef<const T, D>;
};

template<typename T, typename D>
struct ArrayBase: Expr<ArrayBase<T, D>> {
    static constexpr size_t rank = D::rank;
    using Base = Expr<ArrayBase<T, D>>;
    using ExprAssign<ArrayBase<T, D>>::operator=;

    ArrayBase(D dims) : dims_(dims) {
        base_.reset(new T[this->size()]);
    }

    ArrayBase& operator=(const ArrayBase& input) {
        this->assign(input);
        return *this;
    }

    template<typename Axis>
    stride_t stride(Axis axis) const {
        auto axis_ = into_axis<rank>(axis);
        stride_t result = 1;

        for (size_t i = axis_ + 1; i < rank; i++) {
            result *= dims_[i];
        }

        return result;
    }

    template<typename Axis>
    auto dim_impl(Axis axis) const {
        return dims_[axis];
    }

    T* data() {
        return base_.get();
    }

    const T* data() const {
        return base_.get();
    }

    template<typename Device>
    ArrayCursor<T, D> cursor_impl(Device device) {
        return this->nested().cursor_impl(device);
    }

    template<typename Device>
    ArrayCursor<const T, D> cursor_impl(Device device) const {
        return this->nested().cursor_impl(device);
    }

  private:
    std::unique_ptr<T[]> base_;
    D dims_;
};

template<typename T, typename D>
struct ExprTraits<ArrayRef<T, D>>: ExprTraits<ArrayBase<T, D>> {};

template<typename T, typename D>
struct ArrayRef: Expr<ArrayRef<T, D>> {
    ArrayRef(const ArrayRef<T, D>&) = default;
    ArrayRef(ArrayBase<T, D>& parent) : parent_(parent) {}

    ArrayBase<T, D>& parent() const {
        return parent_;
    }

    T* data() {
        return parent_.data();
    }

    const T* data() const {
        return parent_.data();
    }

    template<typename Axis>
    auto stride_impl(Axis axis) const {
        return parent_.stride_impl(axis);
    }

    template<typename Axis>
    auto dim_impl(Axis axis) const {
        return parent_.dim_impl(axis);
    }

    template<typename Device>
    ArrayCursor<T, D> cursor_impl(Device device) const {
        return ArrayCursor<T, D>(parent_);
    }

  private:
    ArrayBase<T, D>& parent_;
};

template<typename T, typename D>
struct ExprTraits<ArrayRef<const T, D>>: ExprTraits<ArrayBase<T, D>> {
    static constexpr AccessMode mode = AccessMode::ReadOnly;
    using Nested = ArrayRef<const T, D>;
};

template<typename T, typename D>
struct ArrayRef<const T, D>: Expr<ArrayRef<const T, D>> {
    ArrayRef(const ArrayBase<T, D>& parent) : parent_(parent) {}
    ArrayRef(const ArrayRef<const T, D>&) = default;
    ArrayRef(const ArrayRef<T, D>& d) : parent_(d.parent()) {}

    const ArrayBase<T, D>& parent() const {
        return parent_;
    }

    const T* data() const {
        return parent_.data();
    }

    template<typename Axis>
    auto stride_impl(Axis axis) const {
        return parent_.stride_impl(axis);
    }

    template<typename Axis>
    auto dim_impl(Axis axis) const {
        return parent_.dim_impl(axis);
    }

    template<typename Device>
    ArrayCursor<const T, D> cursor_impl(Device device) const {
        return ArrayCursor<const T, D>(parent_);
    }

  private:
    const ArrayBase<T, D>& parent_;
};

template<typename T, typename D>
struct ArrayCursor {
    ArrayCursor(ArrayBase<T, D>& parent) :
        parent_(parent),
        ptr_(parent.data()) {}

    template<typename Axis, typename Steps>
    void advance(Axis axis, Steps steps) {
        ptr_ += parent_.stride(axis) * steps;
    }

    T evaluate() {
        return *ptr_;
    }

    void store(T item) {
        *ptr_ = std::move(item);
    }

  private:
    const ArrayBase<T, D>& parent_;
    T* ptr_;
};

template<typename T, typename D>
struct ArrayCursor<const T, D> {
    ArrayCursor(const ArrayBase<T, D>& parent) :
        parent_(parent),
        ptr_(parent.data()) {}

    template<typename Axis, typename Steps>
    void advance(Axis axis, Steps steps) {
        ptr_ += parent_.stride(axis) * steps;
    }

    T evaluate() {
        return *ptr_;
    }

  private:
    const ArrayBase<T, D>& parent_;
    const T* ptr_;
};

}  // namespace capybara