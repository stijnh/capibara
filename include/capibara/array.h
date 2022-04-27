#pragma once

#include <complex>
#include <memory>

#include "dimensions.h"
#include "expr.h"
#include "forwards.h"

namespace capibara {

template<typename T, typename D>
struct ArrayCursor;

template<typename T, typename D>
struct ExprTraits<ArrayBase<T, D>> {
    static constexpr size_t rank = D::rank;
    using value_type = T;
    using index_type = size_t;
    using cursor_type = ArrayCursor<T, D>;
    using nested_type = const ArrayBase<T, D>&;
};

template<typename T, typename D>
struct ArrayBase: View<ArrayBase<T, D>> {
    friend ArrayCursor<T, D>;

    using base_type = View<ArrayBase<T, D>>;
    using base_type::rank;
    using typename base_type::cursor_type;
    using typename base_type::index_type;
    using typename base_type::ndindex_type;
    using typename base_type::value_type;
    using stride_type = ptrdiff_t;
    using dims_type = D;

    using base_type::operator=;

    ArrayBase(dims_type dims) : dims_(std::move(dims)) {
        std::cout << this->size() << std::endl;
        base_.reset(new T[this->size()]);
    }

    ArrayBase() : ArrayBase(dims_type {}) {}

    size_t linearize_index(ndindex_type idx) const {
        size_t offset = 0;

        for (size_t i = 0; i < rank; i++) {
            offset = offset * dims_[i] + idx[i];
        }

        return offset;
    }

    value_type& access(ndindex_type idx) {
        return base_[linearize_index(idx)];
    }

    const value_type& access(ndindex_type idx) const {
        return base_[linearize_index(idx)];
    }

    template<typename Axis>
    auto stride(Axis i) const {
        stride_type stride = 1;

        for (size_t j = rank; j > i + 1; j--) {
            stride *= dims_[j - 1];
        }

        return stride;
    }

    template<typename Axis>
    auto dim(Axis axis) const {
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
    dims_type dims_;
};

template<typename T, size_t... Dims>
using Array = ArrayBase<T, DimensionsDyn<Dims...>>;

template<typename T, size_t N>
using ArrayN = ArrayBase<T, DimensionsN<N>>;

template<typename T>
using Array0 = ArrayN<T, 0>;
template<typename T>
using Array1 = ArrayN<T, 1>;
template<typename T>
using Array2 = ArrayN<T, 2>;
template<typename T>
using Array3 = ArrayN<T, 3>;
template<typename T>
using Array4 = ArrayN<T, 4>;
template<typename T>
using Array5 = ArrayN<T, 5>;
template<typename T>
using Array6 = ArrayN<T, 6>;

#define IMPL_ARRAY_FOR_TYPE(short_name, long_name)       \
    template<size_t... Dims>                             \
    using Array##short_name = Array<long_name, Dims...>; \
    using Array0##short_name = Array0<long_name>;        \
    using Array1##short_name = Array1<long_name>;        \
    using Array2##short_name = Array2<long_name>;        \
    using Array3##short_name = Array3<long_name>;        \
    using Array4##short_name = Array4<long_name>;        \
    using Array5##short_name = Array5<long_name>;        \
    using Array6##short_name = Array6<long_name>;

IMPL_ARRAY_FOR_TYPE(i, int)
IMPL_ARRAY_FOR_TYPE(f, float)
IMPL_ARRAY_FOR_TYPE(d, double)
IMPL_ARRAY_FOR_TYPE(l, long long)
IMPL_ARRAY_FOR_TYPE(b, bool)
IMPL_ARRAY_FOR_TYPE(c, std::complex<double>)
IMPL_ARRAY_FOR_TYPE(cf, std::complex<float>)
#undef IMPL_ARRAY_FOR_TYPE

namespace array_helpers {
    template<typename Axis, size_t N>
    struct Stride {
        using return_type = ptrdiff_t;

        template<typename D>
        static return_type call(Axis axis, const D& dims) {
            ptrdiff_t stride = 1;

            for (size_t i = N - 1; i > axis; i--) {
                stride *= dims[i];
            }

            return stride;
        }
    };

    template<size_t N>
    struct Stride<Axis<N - 1>, N> {
        using return_type = ConstDiff<1>;

        template<typename D>
        static return_type call(Axis<N - 1>, const D&) {
            return {};
        }
    };

    template<size_t K, size_t N>
    struct Stride<Axis<K>, N> {
        template<typename D>
        static auto call(Axis<K>, const D& dims) {
            return dims[Axis<K + 1> {}]
                * Stride<Axis<K + 1>, N>::call(Axis<K + 1> {}, dims);
        }
    };

    template<typename Axis>
    struct Stride<Axis, 1> {
        template<typename D>
        static auto call(Axis, const D& dims) {
            return dims[Axis0];
        }
    };
}  // namespace array_helpers

template<typename T, typename D>
struct ArrayCursor {
    using value_type = T;
    static constexpr size_t rank = D::rank;

    ArrayCursor(const ArrayBase<T, D>& inner) :
        inner_(inner),
        cursor_(inner.base_.get()) {}

    template<typename Axis>
    auto stride(Axis axis) {
        return array_helpers::Stride<Axis, rank>(axis, inner_.dims_);
    }

    template<typename Axis, typename Diff>
    void advance(Axis axis, Diff steps) {
        cursor_ += stride(axis) * steps;
    }

    value_type eval() const {
        return *cursor_;
    }

  private:
    value_type* cursor_;
    const ArrayBase<T, D>& inner_;
};

}  // namespace capibara