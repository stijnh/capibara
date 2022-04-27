#include <complex>
#pragma once

namespace capibara {

template<typename F, typename E>
struct UnaryCursor;

template<typename F, typename E>
struct ExprTraits<UnaryExpr<F, E>> {
    static constexpr size_t rank = ExprTraits<E>::rank;
    using value_type =
        typename std::result_of<F(typename ExprTraits<E>::value_type)>::type;
    using index_type = typename ExprTraits<E>::index_type;
    using cursor_type = UnaryCursor<F, E>;
    using nested_type = UnaryExpr<F, E>;
};

template<typename F, typename E>
struct UnaryExpr: Expr<UnaryExpr<F, E>> {
    friend UnaryCursor<F, E>;

    using base_type = Expr<UnaryExpr<F, E>>;
    using base_type::rank;
    using typename base_type::cursor_type;
    using typename base_type::value_type;

    UnaryExpr(F op, const E& inner) : op_(std::move(op)), inner_(inner) {}

    UnaryExpr(const E& inner) : op_({}), inner_(inner) {}

    template<typename Axis>
    CAPIBARA_INLINE auto dim(Axis i) const {
        return inner_.dim(i);
    }

  private:
    F op_;
    typename E::nested_type inner_;
};

template<typename F, typename E>
struct UnaryCursor {
    using expr_type = UnaryExpr<F, E>;
    using value_type = typename expr_type::value_type;

    UnaryCursor(const expr_type& e) : op_(e.op_), inner_(e.inner_.cursor()) {}

    template<typename Axis, typename Diff>
    void advance(Axis axis, Diff diff) {
        inner_.advance(axis, diff);
    }

    value_type eval() const {
        return op_(inner_.eval());
    }

  private:
    F op_;
    typename E::cursor_type inner_;
};

template<typename F, typename E>
CAPIBARA_INLINE UnaryExpr<F, E> map(const Expr<E>& expr, F fun) {
    return UnaryExpr<F, E>(fun, expr.self());
}

#define DEFINE_SIMPLE_UNARY(type_name, fun_name)                               \
    namespace unary_functors {                                                 \
        template<typename T>                                                   \
        struct type_name {                                                     \
            CAPIBARA_INLINE                                                    \
            auto operator()(T value) const {                                   \
                return fun_name(value);                                        \
            }                                                                  \
        };                                                                     \
    }                                                                          \
    template<                                                                  \
        typename E,                                                            \
        typename = decltype(fun_name(std::declval<typename E::value_type>()))> \
    CAPIBARA_INLINE auto type_name(const ::capibara::Expr<E>& e) {             \
        return e.map(unary_functors::type_name<typename E::value_type> {});    \
    }

DEFINE_SIMPLE_UNARY(isnan, std::isnan)
DEFINE_SIMPLE_UNARY(isinf, std::isinf)
DEFINE_SIMPLE_UNARY(isfinite, std::isfinite)
DEFINE_SIMPLE_UNARY(abs, std::abs)

DEFINE_SIMPLE_UNARY(sin, std::sin)
DEFINE_SIMPLE_UNARY(cos, std::cos)
DEFINE_SIMPLE_UNARY(tan, std::tan)
DEFINE_SIMPLE_UNARY(sinh, std::sinh)
DEFINE_SIMPLE_UNARY(cosh, std::cosh)
DEFINE_SIMPLE_UNARY(tanh, std::tanh)
DEFINE_SIMPLE_UNARY(asin, std::asin)
DEFINE_SIMPLE_UNARY(acos, std::acos)
DEFINE_SIMPLE_UNARY(atan, std::atan)
DEFINE_SIMPLE_UNARY(asinh, std::asinh)
DEFINE_SIMPLE_UNARY(acosh, std::acosh)
DEFINE_SIMPLE_UNARY(atanh, std::atanh)

DEFINE_SIMPLE_UNARY(exp, std::exp)
DEFINE_SIMPLE_UNARY(exp2, std::exp2)
DEFINE_SIMPLE_UNARY(log, std::log)
DEFINE_SIMPLE_UNARY(log2, std::log2)
DEFINE_SIMPLE_UNARY(log10, std::log10)

DEFINE_SIMPLE_UNARY(sqrt, std::sqrt)
DEFINE_SIMPLE_UNARY(ceil, std::ceil)
DEFINE_SIMPLE_UNARY(floor, std::floor)
DEFINE_SIMPLE_UNARY(trunc, std::trunc)

DEFINE_SIMPLE_UNARY(real, std::real)
DEFINE_SIMPLE_UNARY(imag, std::imag)
DEFINE_SIMPLE_UNARY(norm, std::norm)
DEFINE_SIMPLE_UNARY(conj, std::conj)

#undef DEFINE_SIMPLE_UNARY

namespace unary_functors {
    template<typename From, typename To>
    struct cast {
        CAPIBARA_INLINE
        To operator()(From value) const {
            return To(value);
        }
    };
}  // namespace unary_functors

template<typename R, typename E>
CAPIBARA_INLINE auto cast(const Expr<E>& e) {
    return e.map(unary_functors::cast<expr_value_type<E>, R> {});
}

namespace unary_functors {
    template<typename T>
    struct clamp {
        clamp(T low, T high) : lo_(low), hi_(high) {}

        T operator()(T value) const {
            if (value < lo_) {
                return lo_;
            } else if (value > hi_) {
                return hi_;
            } else {
                return value;
            }
        }

      private:
        T lo_;
        T hi_;
    };

}  // namespace unary_functors

template<typename E, typename T = expr_value_type<E>>
CAPIBARA_INLINE auto clamp(const Expr<E>& e, T lo, T hi) {
    return e.map(unary_functors::clamp<T> {});
}

namespace unary_functors {
    template<typename T, typename E>
    struct pow {
        pow(T expo) : expo_(expo) {}

        CAPIBARA_INLINE
        T operator()(T value) const {
            return pow(value, expo_);
        }

      private:
        E expo_;
    };

    template<typename T>
    struct pow2 {
        CAPIBARA_INLINE
        T operator()(T value) const {
            return value * value;
        }
    };

}  // namespace unary_functors

template<typename E, typename T>
CAPIBARA_INLINE auto pow(const Expr<E>& e, T expo) {
    return e.map(unary_functors::pow<expr_value_type<E>, T> {expo});
}

template<typename E, typename T>
CAPIBARA_INLINE auto pow2(const Expr<E>& e) {
    return e.map(unary_functors::pow2<expr_value_type<E>> {});
}

}  // namespace capibara