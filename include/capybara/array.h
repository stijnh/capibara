#pragma once

#include <array>
#include <memory>

#include "expr.h"

namespace capybara {

template<typename T, typename D>
struct ArrayRef;

template<typename T, typename D>
struct ArrayCursor;

template<typename T, typename D>
struct ExprTraits<ArrayBase<T, D>> {
    static constexpr size_t rank = std::tuple_size<D>::value;
    static constexpr bool is_view = true;
    static constexpr bool is_readable = true;
    static constexpr bool is_writeable = true;
    using Value = decay_t<T>;
    using Cursor = ArrayCursor<T, D>;
};

template<typename T, typename D>
struct ExprTraits<const ArrayBase<T, D>>: ExprTraits<ArrayBase<T, D>> {
    static constexpr bool is_writeable = false;
    using Cursor = ArrayCursor<const T, D>;
};

template<typename T, typename D>
struct ExprNested<ArrayBase<T, D>> {
    using type = ArrayRef<T, D>;
};

template<typename T, typename D>
struct ExprNested<const ArrayBase<T, D>> {
    using type = ArrayRef<const T, D>;
};

namespace detail {
    template<
        size_t i,
        typename D,
        typename = typename std::tuple_element<i, D>::type>
    struct DimConstHelper {};

    template<size_t i, typename D, index_t size>
    struct DimConstHelper<i, D, ConstIndex<size>>: ConstIndex<size> {};
}  // namespace detail

template<size_t i, typename T, typename D>
struct ExprConstDim<ArrayBase<T, D>, i>: detail::DimConstHelper<i, D> {};

template<size_t i, typename T, typename D>
struct ExprConstDim<ArrayRef<T, D>, i>: ExprConstDim<ArrayBase<T, D>, i> {};

template<size_t i, typename T, typename D>
struct ExprConstDim<ArrayRef<const T, D>, i>:
    ExprConstDim<ArrayRef<T, D>, i> {};

template<typename T, typename D>
struct ArrayBase: Expr<ArrayBase<T, D>> {
    ArrayBase(D dims) : dims_(dims) {
        size_t size = this->size();
        base_.reset(new T[size]);
    }

    ArrayBase(const std::initializer_list<index_t>& list) :
        ArrayBase(D(list.begin(), list.end())) {}

    ArrayBase() : ArrayBase(D {}) {}

    template<typename Axis>
    index_t dim_impl(Axis axis) const {
        return dims_[axis];
    }

    T* data() {
        return base_.get();
    }

    const T* data() const {
        return base_.get();
    }

  private:
    std::unique_ptr<T[]> base_;
    D dims_;
};

template<typename T, typename D>
struct ExprTraits<ArrayRef<T, D>>: ExprTraits<ArrayBase<T, D>> {};

template<typename T, typename D>
struct ArrayRef: Expr<ArrayRef<T, D>> {
    ArrayRef(const ArrayRef<T, D>& a) = default;
    ArrayRef(ArrayBase<T, D>& a) : inner_(a) {}

    template<typename Axis>
    index_t dim_impl(Axis axis) const {
        return inner_.dim(axis);
    }

    template<typename Axis>
    index_t stride_impl(Axis axis) const {
        return inner_.stride(axis);
    }

    T* data() const {
        return inner_.data();
    }

    ArrayBase<T, D>& parent() const {
        return inner_;
    }

  private:
    ArrayBase<T, D>& inner_;
};

template<typename T, typename D>
struct ExprTraits<ArrayRef<const T, D>>: ExprTraits<const ArrayBase<T, D>> {};

template<typename T, typename D>
struct ArrayRef<const T, D>: Expr<ArrayRef<const T, D>> {
    ArrayRef(const ArrayRef<T, D>& a) : inner_(a.inner_) {}
    ArrayRef(const ArrayRef<const T, D>& a) : inner_(a.inner_) {}
    ArrayRef(const ArrayBase<T, D>& a) : inner_(a) {}

    template<typename Axis>
    index_t dim_impl(Axis axis) const {
        return inner_.dim(axis);
    }

    template<typename Axis>
    index_t stride_impl(Axis axis) const {
        return inner_.stride(axis);
    }

    T* data() {
        return inner_.get();
    }

    const T* data() const {
        return inner_.get();
    }

    const ArrayBase<T, D>& parent() const {
        return inner_;
    }

  private:
    const ArrayBase<T, D>& inner_;
};

template<typename T, typename D>
struct ArrayCursor {
    ArrayCursor(const ArrayRef<T, D>& a) :
        ptr_(a.data()),
        parent_(a.parent()) {}

    template<typename Axis, typename Steps>
    void advance(Axis axis, Steps steps) {
        ptr_ += steps * parent_.stride(axis);
    }

    void store(T value) {
        *ptr_ = std::move(value);
    }

    T eval() const {
        return *ptr_;
    }

  private:
    T* ptr_;
    const ArrayBase<T, D>& parent_;
};

template<typename T, typename D>
struct ArrayCursor<const T, D> {
    ArrayCursor(const ArrayRef<T, D>& a) :
        ptr_(a.data()),
        parent_(a.parent()) {}
    ArrayCursor(const ArrayRef<const T, D>& a) :
        ptr_(a.data()),
        parent_(a.parent()) {}

    template<typename Axis, typename Steps>
    void advance(Axis axis, Steps steps) {
        ptr_ += steps * parent_.stride(axis);
    }

    T eval() const {
        return *ptr_;
    }

  private:
    const T* ptr_;
    const ArrayBase<T, D>& parent_;
};

}  // namespace capybara