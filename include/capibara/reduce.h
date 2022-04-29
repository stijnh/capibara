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
    using Value = typename R::aggregate_type;
    using Index = typename ExprTraits<E>::Index;
    using Cursor = ReduceCursor<R, E, N>;
    using Nested = ReduceExpr<R, E, N>;
};

template<typename R, typename E, size_t N>
struct ReduceExpr: Expr<ReduceExpr<R, E, N>> {
    using Base = Expr<ReduceExpr<R, E, N>>;
    using Nested = E::Nested;
    using Base::Cursor;

    friend Cursor;

    ReduceExpr(R reducer, const E& inner) :
        reducer_(std::move(reducer)),
        inner_(inner) {}

    template<typename Axis>
    CAPIBARA_INLINE auto dim(Axis i) const {
        return inner_.dim(i);
    }

  private:
    R reducer_;
    Nested inner_;
};

template<typename R, typename E, size_t N>
struct InnerReduceCursor {
    using Expr = ReduceExpr<F, L, R>;
    using Expr::rank;

    InnerReduceCursor(const Expr& e) :
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
    typename E::Cursor cursor_;
};

template<typename R, typename E, size_t N>
struct ReduceCursor {
    using Expr = ReduceExpr<F, L, R>;
    using Value = Expr::Value;
    static constexpr size_t inner_rank = E::rank;

    ReduceCursor(const Expr& e) : inner_(e) {}

    template<typename Axis, typename Diff>
    void advance(Axis axis, Diff diff) {
        inner_.cursor_.advance(into_axis<inner_rank>(axis), diff);
    }

    Value eval() const {
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