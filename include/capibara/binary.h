#pragma once

#include <algorithm>

namespace capibara {

template<typename F, typename L, typename R>
struct BinaryCursor;

template<typename F, typename L, typename R>
struct ExprTraits<BinaryExpr<F, L, R>> {
    static constexpr size_t rank = ExprTraits<L>::rank;
    using cursor_type = BinaryCursor<F, L, R>;
    using nested_type = BinaryExpr<F, L, R>;
    using value_type = typename std::result_of<
        F(expr_value_type<L>, expr_value_type<R>)>::type;
    using index_type = typename std::common_type<
        typename ExprTraits<L>::index_type,
        typename ExprTraits<R>::index_type>::type;
};

template<typename F, typename L, typename R>
struct BinaryExpr: Expr<BinaryExpr<F, L, R>> {
    friend BinaryCursor<F, L, R>;

    using base_type = Expr<BinaryExpr<F, L, R>>;
    using base_type::rank;
    using typename base_type::cursor_type;
    using typename base_type::value_type;
    using left_type = typename L::nested_type;
    using right_type = typename R::nested_type;

    BinaryExpr(F op, left_type lhs, right_type rhs) :
        op_(std::move(op)),
        lhs_(std::move(lhs)),
        rhs_(std::move(rhs)) {}

    template<typename Axis>
    CAPIBARA_INLINE auto dim(Axis i) const {
        return lhs_.dim(i);
    }

  private:
    F op_;
    left_type lhs_;
    right_type rhs_;
};

template<typename F, typename L, typename R>
struct BinaryCursor {
    using expr_type = BinaryExpr<F, L, R>;
    using value_type = typename expr_type::value_type;

    BinaryCursor(const expr_type& e) :
        op_(e.op_),
        lhs_(e.lhs_.cursor()),
        rhs_(e.rhs_.cursor()) {}

    template<typename Axis, typename Diff>
    void advance(Axis axis, Diff diff) {
        lhs_.advance(axis, diff);
        rhs_.advance(axis, diff);
    }

    value_type eval() const {
        return op_(lhs_.eval(), rhs_.eval());
    }

  private:
    F op_;
    typename L::cursor_type lhs_;
    typename R::cursor_type rhs_;
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
    return zip<binary_functors::add<expr_value_type<L>, expr_value_type<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator-(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::sub<expr_value_type<L>, expr_value_type<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator*(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::mul<expr_value_type<L>, expr_value_type<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator/(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::div<expr_value_type<L>, expr_value_type<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator%(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::rem<expr_value_type<L>, expr_value_type<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator&(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<
        binary_functors::bit_and<expr_value_type<L>, expr_value_type<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator|(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::bit_or<expr_value_type<L>, expr_value_type<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator==(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::cmp_eq<expr_value_type<L>, expr_value_type<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator!=(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::cmp_ne<expr_value_type<L>, expr_value_type<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator<(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::cmp_lt<expr_value_type<L>, expr_value_type<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator>(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::cmp_gt<expr_value_type<L>, expr_value_type<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator<=(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::cmp_le<expr_value_type<L>, expr_value_type<R>>>(
        lhs,
        rhs);
}

template<typename L, typename R>
auto operator>=(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::cmp_ge<expr_value_type<L>, expr_value_type<R>>>(
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
        typename std::common_type<expr_value_type<L>, expr_value_type<R>>::type>
auto min(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::min<T>>(lhs, rhs);
}

template<
    typename L,
    typename R,
    typename T =
        typename std::common_type<expr_value_type<L>, expr_value_type<R>>::type>
auto max(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::max<T>>(lhs, rhs);
}

template<
    typename L,
    typename R,
    typename T =
        typename std::common_type<expr_value_type<L>, expr_value_type<R>>::type>
auto minmax(const Expr<L>& lhs, const Expr<R>& rhs) {
    return zip<binary_functors::minmax<T>>(lhs, rhs);
}

}  // namespace capibara