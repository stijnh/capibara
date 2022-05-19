#pragma once
#include "expr.h"

namespace capybara {

template<typename F>
struct nullary_cursor;

template<typename F, size_t N>
struct expr_traits<nullary_expr<F, N>> {
    static constexpr size_t rank = N;
    using value_type = typename invoke_result<F>::type;
    static constexpr bool is_writable = false;
    static constexpr bool is_view = false;
};

template<size_t N>
CAPYBARA_INLINE bool is_broadcastable(dshape<N> input, dshape<N> output) {
    for (size_t i = 0; i < N; i++) {
        if (input[i] != output[i] && input[i] != 1) {
            return false;
        }
    }

    return true;
}

template<size_t N>
CAPYBARA_INLINE void assert_broadcastable(dshape<N> input, dshape<N> output) {
    if (!is_broadcastable(input, output)) {
        throw std::runtime_error("failed to broadcast shapes");
    }
}

template<typename F, size_t N, typename D>
struct expr_cursor<const nullary_expr<F, N>, D> {
    using type = nullary_cursor<F>;

    CAPYBARA_INLINE
    static type
    call(const nullary_expr<F, N>& expr, dshape<N> shape, D device) {
        assert_broadcastable(expr.shape(), shape);
        return type(expr.functor());
    }
};

template<typename F, size_t N>
struct nullary_expr: expr<nullary_expr<F, N>> {
    using base_type = expr<nullary_expr<F, N>>;
    using typename base_type::shape_type;

    CAPYBARA_INLINE
    nullary_expr(F fun = {}, shape_type shape = {}) :
        fun_(std::move(fun)),
        shape_(shape) {}

    CAPYBARA_INLINE
    index_t dimension_impl(index_t axis) const {
        return shape_[axis];
    }

    CAPYBARA_INLINE
    const F& functor() const {
        return fun_;
    }

  private:
    F fun_;
    shape_type shape_;
};

template<typename F>
struct nullary_cursor {
    using value_type = typename invoke_result<F>::type;

    nullary_cursor(F fun) : fun_(std::move(fun)) {}

    CAPYBARA_INLINE
    void advance(index_t axis, index_t steps) {}

    CAPYBARA_INLINE
    value_type load() const {
        return fun_();
    }

  private:
    F fun_;
};

namespace functors {
    template<typename T>
    struct value {
        value(T val = {}) : val_(std::move(val)) {}

        CAPYBARA_INLINE
        T operator()() const {
            return val_;
        }

      private:
        T val_;
    };

    template<typename T = void>
    struct zero {
        CAPYBARA_INLINE
        T operator()() const {
            return T {0};
        }

        CAPYBARA_INLINE
        operator T() const {
            return T {0};
        }
    };

    template<>
    struct zero<void> {
        CAPYBARA_INLINE
        operator int() const {
            return 0;
        }
        CAPYBARA_INLINE
        operator std::complex<float>() const {
            return 0;
        }
        CAPYBARA_INLINE
        operator std::complex<double>() const {
            return 0;
        }

        CAPYBARA_INLINE
        zero operator()() const {
            return *this;
        }
    };

    template<typename T = void>
    struct one {
        CAPYBARA_INLINE
        T operator()() const {
            return T {1};
        }

        CAPYBARA_INLINE
        operator T() const {
            return 1;
        }
    };

    template<>
    struct one<void> {
        CAPYBARA_INLINE
        operator int() const {
            return 1;
        }

        CAPYBARA_INLINE
        operator std::complex<float>() const {
            return 1;
        }

        CAPYBARA_INLINE
        operator std::complex<double>() const {
            return 1;
        }

        CAPYBARA_INLINE
        one operator()() const {
            return *this;
        }
    };
}  // namespace functors

template<typename T = void, size_t N = 0>
        CAPYBARA_INLINE
nullary_expr<functors::zero<T>, N> zeros(dshape<N> shape = {}) {
    return {{}, shape};
}

template<typename T = void, size_t N = 0>
        CAPYBARA_INLINE
nullary_expr<functors::one<T>, N> ones(dshape<N> shape = {}) {
    return {{}, shape};
}

template<typename T, size_t N = 0>
using scalar_type = nullary_expr<functors::value<decay_t<T>>, N>;

template<typename T, size_t N = 0>
        CAPYBARA_INLINE
scalar_type<T, N> full(T&& value, dshape<N> shape = {}) {
    return {{std::forward<T>(value)}, shape};
}

template<typename T>
CAPYBARA_INLINE
scalar_type<T, 0> scalar(T&& value) {
    return {{std::forward<T>(value)}};
}

}  // namespace capybara