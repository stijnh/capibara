#pragma once

#include <algorithm>

namespace capibara {

template<typename F, typename L, typename R>
struct BinaryCursor;

template<typename F, typename L, typename R>
struct ExprTraits<BinaryExpr<F, L, R>> {
    static constexpr size_t rank = ExprTraits<L>::rank;
    using Cursor = BinaryCursor<F, L, R>;
    using Nested = BinaryExpr<F, L, R>;
    using Value = typename std::result_of<
        F(ExprValue<L>, ExprValue<R>)>::type;
    using Index = typename std::common_type<
        typename ExprTraits<L>::Index,
        typename ExprTraits<R>::Index>::type;
};

template<typename F, typename L, typename R>
struct BinaryExpr: Expr<BinaryExpr<F, L, R>> {
    using Base = Expr<BinaryExpr<F, L, R>>;
    using Base::rank;
    using typename Base::Cursor;
    using typename Base::Value;
    using LeftNested = typename L::Nested;
    using RightNested = typename R::Nested;
    friend Cursor;

    BinaryExpr(F op, LeftNested lhs, RightNested rhs) :
        op_(std::move(op)),
        lhs_(std::move(lhs)),
        rhs_(std::move(rhs)) {}

    template<typename Axis>
    CAPIBARA_INLINE auto dim(Axis i) const {
        return lhs_.dim(i);
    }

  private:
    F op_;
    LeftNested lhs_;
    RightNested rhs_;
};

template<typename F, typename L, typename R>
struct BinaryCursor {
    using Expr = BinaryExpr<F, L, R>;
    using Value = typename Expr::Value;

    BinaryCursor(const Expr& e) :
        op_(e.op_),
        lhs_(e.lhs_.cursor()),
        rhs_(e.rhs_.cursor()) {}

    template<typename Axis, typename Diff>
    void advance(Axis axis, Diff diff) {
        lhs_.advance(axis, diff);
        rhs_.advance(axis, diff);
    }

    Value eval() const {
        return op_(lhs_.eval(), rhs_.eval());
    }

  private:
    F op_;
    typename L::Cursor lhs_;
    typename R::Cursor rhs_;
};

template<typename F, typename L, typename R>
BinaryExpr<F, L, R> zip(const Expr<L>& lhs, const Expr<R>& rhs, F op) {
    return BinaryExpr<F, L, R> {op, lhs.self(), rhs.self()};
}

template<typename F, typename L, typename R>
BinaryExpr<F, L, R> zip(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip(lhs, rhs, F {});
}

namespace binary_functors {
    template<typename L, typename R>
    struct add {
        auto operator()(L left, R right) const -> decltype(left + right) {
            return left + right;
        }
    };

    template<typename L, typename R>
    struct sub {
        auto operator()(L left, R right) const -> decltype(left - right) {
            return left - right;
        }
    };

    template<typename L, typename R>
    struct mul {
        auto operator()(L left, R right) const -> decltype(left * right) {
            return left * right;
        }
    };

    template<typename L, typename R>
    struct div {
        auto operator()(L left, R right) const -> decltype(left / right) {
            return left / right;
        }
    };

    template<typename L, typename R>
    struct rem {
        auto operator()(L left, R right) const -> decltype(left % right) {
            return left % right;
        }
    };

    template<typename L, typename R>
    struct bit_and {
        auto operator()(L left, R right) const -> decltype(left & right) {
            return left & right;
        }
    };

    template<typename L, typename R>
    struct bit_or {
        auto operator()(L left, R right) const -> decltype(left | right) {
            return left | right;
        }
    };

    template<typename L, typename R>
    struct cmp_lt {
        bool operator()(L left, R right) const {
            return cmp_less(left, right);
        }
    };

    template<typename L, typename R>
    struct cmp_gt {
        bool operator()(L left, R right) const {
            return cmp_greater(left, right);
        }
    };

    template<typename L, typename R>
    struct cmp_le {
        bool operator()(L left, R right) const {
            return cmp_less_equal(left, right);
        }
    };

    template<typename L, typename R>
    struct cmp_ge {
        bool operator()(L left, R right) const {
            return cmp_greater_equal(left, right);
        }
    };

    template<typename L, typename R>
    struct cmp_eq {
        bool operator()(L left, R right) const {
            return cmp_equal(left, right);
        }
    };

    template<typename L, typename R>
    struct cmp_ne {
        bool operator()(L left, R right) const {
            return cmp_not_equal(left, right);
        }
    };
}  // namespace binary_functors

template<typename L, typename R>
auto operator+(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::add<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator-(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::sub<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator*(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::mul<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator/(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::div<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator%(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::rem<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator&(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<
        binary_functors::bit_and<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator|(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::bit_or<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator==(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::cmp_eq<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator!=(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::cmp_ne<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator<(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::cmp_lt<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator>(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::cmp_gt<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator<=(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::cmp_le<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator>=(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::cmp_ge<ExprValue<L>, ExprValue<R>>>(
        lhs,
        rhs);
}

namespace binary_functors {
    template<typename T>
    struct min {
        T operator()(T left, T right) const {
            return std::min(left, right);
        }
    };

    template<typename T>
    struct max {
        T operator()(T left, T right) const {
            return std::min(left, right);
        }
    };

    template<typename T>
    struct minmax {
        std::pair<T, T> operator()(T left, T right) const {
            return std::minmax(left, right);
        }
    };
}  // namespace binary_functors

template<
    typename L,
    typename R,
    typename T =
        typename std::common_type<ExprValue<L>, ExprValue<R>>::type>
auto min(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::min<T>>(lhs, rhs);
}

template<
    typename L,
    typename R,
    typename T =
        typename std::common_type<ExprValue<L>, ExprValue<R>>::type>
auto max(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::max<T>>(lhs, rhs);
}

template<
    typename L,
    typename R,
    typename T =
        typename std::common_type<ExprValue<L>, ExprValue<R>>::type>
auto minmax(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::minmax<T>>(lhs, rhs);
}

}  // namespace capibara