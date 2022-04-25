#pragma once

#include "types.h"

namespace capibara {
template<typename F, typename Dims>
struct ExprTraits<NullaryExpr<F, Dims>>:
    ExprTraits<Array<
        typename std::result_of<F(std::array<size_t, Dims::size()>)>::type,
        Dims::size()>> {};

template<typename F, typename Dims>
struct NullaryExpr: Expr<NullaryExpr<F, Dims>> {
    using base_type = Expr<NullaryExpr<F, Dims>>;
    using base_type::rank;
    using typename base_type::index_type;
    using typename base_type::ndindex_type;
    using typename base_type::value_type;

    NullaryExpr(F fun, Dims dims) : fun_(fun), dims_(dims) {}

    value_type eval(ndindex_type index) const {
        return fun_(index);
    }

    template<typename Axis>
    auto dim(Axis axis) const {
        return dims_[axis];
    }

  private:
    F fun_;
    Dims dims_;
};

#define IMPL_SENTINEL_METHOD(type_name, value)       \
    CAPIBARA_INLINE constexpr operator type_name() { \
        return type_name {value};                    \
    }

#define IMPL_SENTINEL_METHODS(value)
#define IMPL_SENTINEL_METHODS2(value)                \
    IMPL_SENTINEL_METHOD(bool, value)                \
    IMPL_SENTINEL_METHOD(unsigned char, value)       \
    IMPL_SENTINEL_METHOD(signed char, value)         \
    IMPL_SENTINEL_METHOD(char, value)                \
    IMPL_SENTINEL_METHOD(unsigned short, value)      \
    IMPL_SENTINEL_METHOD(short, value)               \
    IMPL_SENTINEL_METHOD(unsigned int, value)        \
    IMPL_SENTINEL_METHOD(int, value)                 \
    IMPL_SENTINEL_METHOD(unsigned long, value)       \
    IMPL_SENTINEL_METHOD(long, value)                \
    IMPL_SENTINEL_METHOD(unsigned long long, value)  \
    IMPL_SENTINEL_METHOD(long long, value)           \
    IMPL_SENTINEL_METHOD(float, value)               \
    IMPL_SENTINEL_METHOD(double, value)              \
    IMPL_SENTINEL_METHOD(std::complex<float>, value) \
    IMPL_SENTINEL_METHOD(std::complex<double>, value)

struct ZeroSentinel {
    IMPL_SENTINEL_METHODS(0)
};

struct OneSentinel {
    IMPL_SENTINEL_METHODS(1)
};

struct BinarySentinel {
    BinarySentinel(bool v) : value_(v) {}

    IMPL_SENTINEL_METHODS(value_ ? 0 : 1)

  private:
    bool value_;
};

struct EmptySentinel {};

namespace nullary_functors {
    template<typename T>
    struct Value {
        Value(T value) : value_(std::move(value)) {}

        template<typename index_type, size_t Rank>
        T operator()(std::array<index_type, Rank>) {
            return value_;
        }

      private:
        T value_;
    };

    template<typename T>
    struct Eye {
        template<typename index_type, size_t Rank>
        T operator()(std::array<index_type, Rank> index) {
            bool is_diagonal = true;
            for (size_t i = 1; i < Rank; i++) {
                if (index[i] != index[0])
                    is_diagonal = false;
            }

            return T {is_diagonal};
        }
    };

}  // namespace nullary_functors

template<typename F, typename... Dims>
auto make_nullary_expr(F fun, Dims... d) {
    auto dims_ = dims(d...);
    return NullaryExpr<F, decltype(dims_)>(fun, dims_);
}

template<typename T, typename... Dims>
auto fill(T value, Dims... dims) {
    return make_nullary_expr(nullary_functors::Value<T> {value}, dims...);
}

template<typename T = ZeroSentinel, typename... Dims>
auto zeros(Dims... dims) {
    return fill(ZeroSentinel {}, dims...).template cast<T>();
}

template<typename T = OneSentinel, typename... Dims>
auto ones(Dims... dims) {
    return fill(OneSentinel {}, dims...).template cast<T>();
}

template<typename... Dims>
auto empty(Dims... dims) {
    return fill(EmptySentinel {}, dims...);
}

template<typename T = BinarySentinel, typename... Dims>
auto eye(Dims... dims) {
    return make_nullary_expr(nullary_functors::Eye<T> {}, dims...);
}

template<typename T, typename E>
auto fill_like(T value, const Expr<E>& expr) {
    return make_nullary_expr(nullary_functors::Value<T> {value}, expr.dims());
}

template<typename T = ZeroSentinel, typename E>
auto zeros_like(const Expr<E>& expr) {
    return fill_like(ZeroSentinel {}, expr).template cast<T>();
}

template<typename T = OneSentinel, typename E>
auto ones_like(const Expr<E>& expr) {
    return fill_like(OneSentinel {}, expr).template cast<T>();
}

template<typename E>
auto empty_like(const Expr<E>& expr) {
    return fill_like(EmptySentinel {}, expr);
}

template<typename T = BinarySentinel, typename E>
auto eye_like(const Expr<E>& expr) {
    return make_nullary_expr(nullary_functors::Eye<T> {}, expr.dims());
}

}  // namespace capibara
