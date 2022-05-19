#pragma once
#include <memory>
#include <sstream>

#include "expr.h"

namespace capybara {

namespace storage {
    template<typename T>
    struct heap {
        using value_type = T;
        using const_value_type = const T;

        heap() = default;

        CAPYBARA_INLINE
        void resize(size_t n) {
            data_.reset(new T[n]);
        }

        CAPYBARA_INLINE
        T* data() {
            return data_.get();
        }

        CAPYBARA_INLINE
        const T* data() const {
            return data_.get();
        }

      private:
        std::unique_ptr<T[]> data_;
    };

    template<typename T, size_t Max = 1>
    struct stack {
        using value_type = T;
        using const_value_type = const T;

        stack() = default;

        CAPYBARA_INLINE
        void resize(size_t n) {
            if (n > Max) {
                throw std::runtime_error("exceeds maximum size");
            }
        }

        CAPYBARA_INLINE
        T* data() {
            return data_.data();
        }

        CAPYBARA_INLINE
        const T* data() const {
            return data_.data();
        }

      private:
        std::array<T, Max> data_;
    };

    template<typename T>
    struct span {
        using value_type = T;
        using const_value_type = T;

        span(stack<T>& v) : data_(v.data()) {}
        span(heap<T>& v) : data_(v.data()) {}
        span(T* ptr) : data_(ptr) {}

        void resize(size_t n) {
            // TODO???
        }

        CAPYBARA_INLINE
        T* data() const {
            return data_;
        }

      private:
        T* data_;
    };

    template<typename T>
    struct span<const T> {
        using value_type = const T;
        using const_value_type = const T;

        span(const stack<T>& v) : data_(v.data()) {}
        span(const heap<T>& v) : data_(v.data()) {}
        span(const T* ptr) : data_(ptr) {}

        void resize(size_t n) {
            // TODO???
        }

        CAPYBARA_INLINE
        const T* data() const {
            return data_;
        }

      private:
        T* data_;
    };
}  // namespace storage

namespace layout {
    template<size_t N>
    struct row_major {
        static constexpr size_t rank = N;
        using shape_type = dshape<N>;

        row_major() = default;
        row_major(shape_type shape) {
            resize(shape);
        }

        void resize(shape_type shape) {
            shape_ = shape;
        }

        CAPYBARA_INLINE
        index_t dimension(index_t axis) const {
            return shape_[axis];
        }

        CAPYBARA_INLINE
        stride_t stride(index_t axis) const {
            stride_t result = 1;

            for (index_t i = axis + 1; i < static_cast<index_t>(rank); i++) {
                result *= static_cast<stride_t>(shape_[i]);
            }

            return result;
        }

      private:
        shape_type shape_;
    };

    template<size_t N>
    using default_layout = row_major<N>;
}  // namespace layout

template<typename T, size_t N>
struct array_cursor;

template<typename L, typename S>
struct expr_traits<array_base<L, S>> {
    static constexpr size_t rank = L::rank;
    using value_type = typename S::value_type;
    static constexpr bool is_writable = !std::is_const<value_type>::value;
    static constexpr bool is_view = true;
};

template<typename L, typename S>
struct expr_traits<const array_base<L, S>>: expr_traits<array_base<L, S>> {
    using value_type = typename S::const_value_type;
    static constexpr bool is_writable = !std::is_const<value_type>::value;
};

template<typename L, typename S>
struct expr_nested<array_base<L, S>> {
    using type = array_base<L, storage::span<typename S::value_type>>;
};

template<typename L, typename S>
struct expr_nested<const array_base<L, S>> {
    using type = array_base<L, storage::span<typename S::const_value_type>>;
};

template<size_t N>
void assert_same_shape(dshape<N> lhs, dshape<N> rhs) {
    if (lhs != rhs) {
        std::stringstream ss;
        ss << "cannot broadcast shape " << lhs << " to " << rhs;
        throw std::runtime_error(ss.str());
    }
}

template<typename L, typename S, typename D>
struct expr_cursor<array_base<L, S>, D> {
    using type = array_cursor<typename S::value_type, L::rank>;

    static type call(array_base<L, S>& expr, dshape<L::rank> shape, D device) {
        if (expr.shape() != shape) {
            assert_same_shape(expr.shape(), shape);
        }

        return type(expr.data(), expr.strides());
    }
};

template<typename L, typename S, typename D>
struct expr_cursor<const array_base<L, S>, D> {
    using type = array_cursor<typename S::const_value_type, L::rank>;

    CAPYBARA_INLINE
    static type
    call(const array_base<L, S>& expr, dshape<L::rank> shape, D device) {
        if (expr.shape() != shape) {
            assert_same_shape(expr.shape(), shape);
        }

        return type(expr.data(), expr.strides());
    }
};

template<typename L, typename S>
struct array_base: expr<array_base<L, S>> {
    template<typename L2, typename S2>
    friend struct array_base;

    using base_type = expr<array_base<L, S>>;
    using base_type::rank;
    using typename base_type::shape_type;
    using layout_type = L;
    using storage_type = S;
    using value_type = typename S::value_type;
    using const_value_type = typename S::const_value_type;

    template<typename L2, typename S2>
    array_base(const array_base<L2, S2>& r) :
        layout_(r.layout_),
        storage_(r.storage_) {}

    template<typename L2, typename S2>
    array_base(array_base<L2, S2>& r) :
        layout_(r.layout_),
        storage_(r.storage_) {}

    array_base(layout_type layout = {}, storage_type storage = {}) :
        layout_(std::move(layout)),
        storage_(std::move(storage)) {
        storage_.resize(this->size());
    }

    array_base(shape_type shape) {
        resize(shape);
    }

    CAPYBARA_INLINE
    void resize(shape_type shape) {
        layout_.resize(shape);
        storage_.resize(this->size());
    }

    CAPYBARA_INLINE
    index_t dimension_impl(index_t axis) const {
        return layout_.dimension(axis);
    }

    CAPYBARA_INLINE
    stride_t stride_impl(index_t axis) const {
        return layout_.stride(axis);
    }

    CAPYBARA_INLINE
    value_type* data() {
        return storage_.data();
    }

    CAPYBARA_INLINE
    const_value_type* data() const {
        return storage_.data();
    }

  private:
    L layout_;
    S storage_;
};

template<typename T, size_t N>
struct array_cursor {
    static constexpr size_t rank = N;
    using strides_type = std::array<stride_t, rank>;

    array_cursor(T* data, strides_type strides) :
        data_(data),
        strides_(strides) {}

    void advance(index_t axis, index_t steps) {
        data_ += strides_[axis] * steps;
    }

    T load() const {
        return *data_;
    }

    void store(T value) {
        *data_ = std::move(value);
    }

  private:
    T* data_;
    strides_type strides_;
};

template<typename T, size_t N>
struct array_cursor<const T, N> {
    static constexpr size_t rank = N;
    using strides_type = std::array<stride_t, rank>;

    array_cursor(const T* data, strides_type strides) :
        data_(data),
        strides_(strides) {}

    void advance(index_t axis, index_t steps) {
        data_ += strides_[axis] * steps;
    }

    T load() const {
        return *data_;
    }

  private:
    const T* data_;
    strides_type strides_;
};

template<typename T, size_t N>
using array = array_base<layout::default_layout<N>, storage::heap<T>>;

template<typename T, size_t N>
using array_ref = array_base<layout::default_layout<N>, storage::span<T>>;

template<typename T>
using array0 = array<T, 0>;
template<typename T>
using array1 = array<T, 1>;

}  // namespace capybara