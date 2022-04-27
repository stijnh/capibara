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

    static constexpr size_t rank = ExprTraits<E>::rank - N;
    using value_type = typename R::aggregate_type;
    using index_type = typename ExprTraits<E>::index_type;
    using cursor_type = ReduceCursor<R, E, N>;
    using nested_type = ReduceExpr<R, E, N>;
};

template<typename R, typename E, size_t N>
struct ReduceExpr: Expr<ReduceExpr<R, E, N>> {
    friend ReduceCursor<R, E, N>;

    using base_type = ReduceExpr<R, E, N>;
    using base_type::rank;
    using typename base_type::cursor_type;
    using typename base_type::value_type;
    using typename

        ReduceExpr(R reducer, const E& inner) :
        reducer_(std::move(reducer)),
        inner_(inner) {}

    template<typename Axis>
    CAPIBARA_INLINE auto dim(Axis i) const {
        //return inner_.dim(i);
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

    template<size_t I, typename Diff>
    void advance(Axis<I> axis, Diff diff) {
        inner_.cursor_.advance(Axis<I + N>, diff);
    }

    template<typename Axis, typename Diff>
    void advance(DynAxis<rank - N> axis, Diff diff) {
        inner_.cursor_.advance((DynAxis<rank>)(axis + N), diff);
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
    using expr_type::rank;

    ReduceCursor(const expr_type& e) : inner_(e) {}

    template<size_t I, typename Diff>
    void advance(Axis<I> axis, Diff diff) {
        inner_.cursor_.advance(axis, diff);
    }

    template<typename Axis, typename Diff>
    void advance(DynAxis<N> axis, Diff diff) {
        inner_.cursor_.advance((DynAxis<rank>)axis, diff);
    }

    value_type eval() const {
        evaluate(inner_, dims_, axes::seq<N> {});
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

    template<
        typename Cond,
        typename Init,
        typename T = typename std::result_of<Init()>::type>
    struct Find {
        using value_type = T;

        Find(Cond cond, Init init) :
            cond_(std::move(cond)),
            init_(std::move(init)),
            state_(init_()) {}

        bool accumulate(value_type input) {
            if (cond_(input)) {
                state_ = std::move(input);
                return true;
            } else {
                return false;
            };
        }

        value_type reset() {
            value_type old_state = init_();
            std::swap(old_state, state_);
            return old_state;
        }

      private:
        Cond cond_;
        Init init_;
        value_type state_;
    };

}  // namespace reducers

}  // namespace capibara