#pragma once

#include "eval.h"

namespace capibara {

template<typename R, typename E, size_t N>
struct ReduceCursor;

template<typename R, typename E, size_t N>
struct ReduceExpr;

template<typename R, typename E, size_t N>
struct ExprTraits<ReduceExpr<R, E, N>> {
    static_assert(N <= ExprTraits<E>::rank, "incompatible axes");

    static constexpr size_t rank = N;
    using value_type = typename R::aggregate_type;
    using index_type = typename ExprTraits<E>::index_type;
    using cursor_type = ReduceCursor<R, E, N>;
    using nested_type = ReduceExpr<R, E, N>;
};

template<typename R, typename E, size_t N>
struct ReduceExpr: Expr<ReduceExpr<R, E, N>> {
    friend ReduceCursor<R, E, N>;

    ReduceExpr(R reducer, const E& inner) :
        reducer_(std::move(reducer)),
        inner_(inner) {}

    template<typename Axis>
    CAPIBARA_INLINE auto dim(Axis i) const {
        return inner_.dim(i);
    }

  private:
    R reducer_;
    typename E::nested_type inner_;
};

template<typename R, typename E, size_t N>
struct InnerReduceCursor {
    using expr_type = ReduceExpr<F, L, R>;
    using expr_type::rank;

    InnerReduceCursor(const expr_type& e) :
        reducer_(e.reducer_),
        cursor_(e.inner_.cursor()) {}

    template<typename Axis, typename Diff>
    void advance(Axis axis, Diff diff) {
        inner_.cursor_.advance(axis.template increment<N>(), diff);
    }

    ControlFlow eval() const {
        if (reducer_.accumulate(cursor_.eval())) {
            return ControlFlow::Continue;
        } else {
            return ControlFlow::Break;
        }
    }

  private:
    R reducer_;
    typename E::cursor_type cursor_;
};

template<typename R, typename E, size_t N>
struct ReduceCursor {
    using expr_type = ReduceExpr<F, L, R>;
    using value_type = expr_type::value_type;
    static constexpr size_t inner_rank = E::rank;

    ReduceCursor(const expr_type& e) : inner_(e) {}

    template<typename Axis, typename Diff>
    void advance(Axis axis, Diff diff) {
        inner_.cursor_.advance(into_axis<inner_rank>(axis), diff);
    }

    value_type eval() const {
        evaluate(inner_, dims_, axes::seq<inner_rank - N> {});
        return inner_.reducer_.reset();
    }

  private:
    InnerReduceCursor<R, E, N> inner_;
};

namespace reducers {
    struct Any {
        bool accumulate(bool d) {
            state_ |= d;
            return !d;
        }

        bool reset() {
            bool old_state = state_;
            std::swap(old_state, state_);
            return old_state;
        }

      private:
        bool state_ = false;
    };

    struct All {
        bool accumulate(bool d) {
            state_ &= d;
            return d;
        }

        bool reset() {
            bool old_state = true;
            std::swap(old_state, state_);
            return old_state;
        }

      private:
        bool state_ = true;
    };

    template<typename T, typename R = T>
    struct Sum {
        ConstFalse accumulate(T d) {
            state_ += T;
            return {};
        }

        R reset() {
            R old_state = {};
            std::swap(old_state, state_);
            return old_state;
        }

      private:
        R state_ = {};
    };

}  // namespace reducers

}  // namespace capibara