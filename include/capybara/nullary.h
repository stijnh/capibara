#pragma once

#include "types.h"

namespace capybara {

template<typename F, typename D, typename = void>
struct NullaryCursor;

template<typename F, typename D>
struct ExprTraits<NullaryExpr<F, D>> {
    static constexpr size_t rank = D::rank;
    using Value = typename std::result_of<F()>::type;
    using Index = size_t;  // TODO: Is this correct?
    using Cursor = NullaryCursor<F, D>;
    using Nested = NullaryExpr<F, D>;
};

template<typename F, typename D>
struct NullaryExpr: Expr<NullaryExpr<F, D>> {
    using Base = Expr<NullaryExpr<F, D>>;

    NullaryExpr(F fun, D dims) : fun_(fun), dims_(dims) {}

    template<typename Axis>
    auto dim(Axis axis) const {
        return dims_[axis];
    }

  private:
    F fun_;
    D dims_;
};

template<typename F, typename D, typename>
struct NullaryCursor {
    using Traits = ExprTraits<NullaryExpr<F, D>>;
    using Value = typename Traits::Value;

    NullaryCursor(F op) : op_(std::move(op)) {}

    template<typename Axis, typename Diff>
    void advance(Axis axis, Diff diff) {}

    Value eval() {
        return op_();
    }

  private:
    F op_;
};

#define IMPL_SENTINEL_METHOD(type_name, value)       \
    CAPYBARA_INLINE constexpr operator type_name() { \
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

struct EmptySentinel {
    IMPL_SENTINEL_METHODS({})
};

namespace nullary_functors {
    template<typename T>
    struct Value {
        Value(T value) : value_(std::move(value)) {}

        T operator()() const {
            return value_;
        }

      private:
        T value_;
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

}  // namespace capybara
