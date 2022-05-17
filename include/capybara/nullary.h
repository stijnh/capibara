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

template<typename F, size_t N, typename D>
struct expr_cursor<nullary_expr<F, N>, D> {
    using type = nullary_cursor<F>;
};

template<typename F, size_t N>
struct nullary_expr: expr<nullary_expr<F, N>> {
    friend struct nullary_cursor<F>;
    using base_type = expr<nullary_expr<F, N>>;
    using typename base_type::shape_type;

    nullary_expr(F fun = {}, shape_type shape = {}) :
        fun_(std::move(fun)),
        shape_(shape) {}

    index_t dimension_impl(index_t axis) const {
        return shape_[axis];
    }

  private:
    F fun_;
    shape_type shape_;
};

template<typename F>
struct nullary_cursor {
    using value_type = typename invoke_result<F>::type;

    template<size_t N>
    nullary_cursor(const nullary_expr<F, N>& expr) : fun_(expr.fun) {
        //
    }

    void advance(index_t axis, index_t steps) {}

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

        T operator()() const {
            return val_;
        }

      private:
        T val_;
    };

    template<typename T = void>
    struct zero {
        T operator()() const {
            return T {0};
        }

        operator T() const {
            return T {0};
        }
    };

    template<>
    struct zero<void> {
        operator int() const {
            return 0;
        }
        operator std::complex<float>() const {
            return 0;
        }
        operator std::complex<double>() const {
            return 0;
        }

        zero operator()() const {
            return *this;
        }
    };

    template<typename T = void>
    struct one {
        T operator()() const {
            return T {1};
        }

        operator T() const {
            return 1;
        }
    };

    template<>
    struct one<void> {
        operator int() const {
            return 1;
        }
        operator std::complex<float>() const {
            return 1;
        }
        operator std::complex<double>() const {
            return 1;
        }

        one operator()() const {
            return *this;
        }
    };
}  // namespace functors

template<typename T = void, size_t N>
nullary_expr<functors::zero<T>, N> zeros(std::array<index_t, N> shape = {}) {
    return {{}, shape};
}

template<typename T = void, size_t N>
nullary_expr<functors::one<T>, N> ones(std::array<index_t, N> shape = {}) {
    return {{}, shape};
}

template<typename T, size_t N = 0>
using scalar_type = nullary_expr<functors::value<decay_t<T>>, N>;

template<typename T, size_t N = 0>
scalar_type<T, N> full(T&& value, std::array<T, N> shape = {}) {
    return {{std::forward<T>(value)}, shape};
}

template<typename T, size_t N = 0>
scalar_type<T, N> scalar(T&& value) {
    return {{std::forward<T>(value)}};
}

}  // namespace capybara