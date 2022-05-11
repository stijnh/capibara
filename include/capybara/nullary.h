#pragma once

#include <complex>
#include <random>

#include "expr.h"

namespace capybara {

template<typename F>
struct NullaryCursor;

template<typename F, typename D>
struct ExprTraits<NullaryExpr<F, D>> {
    static constexpr size_t rank = D::rank;
    static constexpr bool is_view = false;
    static constexpr bool is_readable = true;
    static constexpr bool is_writeable = false;
    using Cursor = NullaryCursor<F>;
    using Value = typename Cursor::Value;
};

template<typename F, typename D>
struct NullaryExpr: Expr<NullaryExpr<F, D>> {
    friend NullaryCursor<F>;

    NullaryExpr(F fun = {}, D dims = {}) :
        fun_(std::move(fun)),
        dims_(std::move(dims)) {}

    template<typename Axis>
    index_t dim_impl(Axis axis) const {
        return dims_[axis];
    }

  private:
    F fun_;
    D dims_;
};

template<typename F>
struct NullaryCursor {
    using Value = apply_t<F&>;

    template<typename D>
    NullaryCursor(NullaryExpr<F, D>&& e) : fun_(std::move(e.fun_)) {}

    template<typename D>
    NullaryCursor(const NullaryExpr<F, D>& e) : fun_(e.fun_) {}

    template<typename Axis, typename Steps>
    void advance(Axis axis, Steps steps) {}

    Value eval() {
        return fun_();
    }

  private:
    F fun_;
};

namespace functors {
    template<typename T>
    struct Value {
        Value() = default;
        Value(T val) : inner_(std::move(val)) {}

        T operator()() const {
            return inner_;
        }

      private:
        T inner_;
    };

    template<typename T = void>
    struct Zero {
        T operator()() const {
            return T {};
        }
    };

    template<>
    struct Zero<void> {
        operator bool() const {
            return false;
        }
        operator char() const {
            return 0;
        }
        operator unsigned char() const {
            return 0;
        }
        operator signed char() const {
            return 0;
        }
        operator unsigned short() const {
            return 0;
        }
        operator signed short() const {
            return 0;
        }
        operator unsigned int() const {
            return 0;
        }
        operator signed int() const {
            return 0;
        }
        operator unsigned long() const {
            return 0;
        }
        operator signed long() const {
            return 0;
        }
        operator float() const {
            return 0;
        }
        operator double() const {
            return 0;
        }
        operator std::complex<float>() const {
            return 0;
        }
        operator std::complex<double>() const {
            return 0;
        }
        operator nullptr_t() const {
            return nullptr;
        }

        Zero operator()() const {
            return *this;
        }
    };

    template<typename T = void>
    struct One {
        T operator()() const {
            return T {1};
        }
    };

    template<>
    struct One<void> {
        operator bool() const {
            return true;
        }
        operator char() const {
            return 1;
        }
        operator unsigned char() const {
            return 1;
        }
        operator signed char() const {
            return 1;
        }
        operator unsigned short() const {
            return 1;
        }
        operator signed short() const {
            return 1;
        }
        operator unsigned int() const {
            return 1;
        }
        operator signed int() const {
            return 1;
        }
        operator unsigned long() const {
            return 1;
        }
        operator signed long() const {
            return 1;
        }
        operator float() const {
            return 1;
        }
        operator double() const {
            return 1;
        }
        operator std::complex<float>() const {
            return 1;
        }
        operator std::complex<double>() const {
            return 1;
        }

        One operator()() const {
            return *this;
        }
    };
}  // namespace functors

template<typename T, typename D>
struct ExprTraits<ValueExpr<T, D>>:
    ExprTraits<NullaryExpr<functors::Value<T>>> {};

template<typename T, typename D>
struct ValueExpr: NullaryExpr<functors::Value<T>> {
    using Base = NullaryExpr<functors::Value<T>>;

    ValueExpr(T value) : Base(functors::Value<T> {std::move(value)}) {}
};

template<typename T = void, typename... Ds>
NullaryExpr<functors::Zero<T>, IntoDims<Ds...>> zeros(Ds&&... dim) {
    return {{}, into_dims(std::forward<Ds>(dim)...)};
}

template<typename T = void, typename... Ds>
NullaryExpr<functors::One<T>, IntoDims<Ds...>> ones(Ds&&... dim) {
    return {{}, into_dims(std::forward<Ds>(dim)...)};
}

template<typename T, typename... Ds>
NullaryExpr<functors::Value<T>, IntoDims<Ds...>> full(T value, Ds&&... dim) {
    return {{std::move(value)}, into_dims(std::forward<Ds>(dim)...)};
}

}  // namespace capybara