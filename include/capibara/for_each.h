#pragma once

#include "eval.h"

namespace capibara {

template<typename E, typename F>
struct EachCursor {
    using expr_type = E;
    using cursor_type = typename expr_type::cursor_type;

    EachCursor(const expr_type& e, F fun) :
        functor_(std::move(fun)),
        cursor_(e.cursor()) {}

    template<typename Axis, typename Diff>
    CAPIBARA_INLINE void advance(Axis axis, Diff diff) {
        cursor_.advance(axis, diff);
    }

    CAPIBARA_INLINE
    ControlFlow eval() const {
        functor_(cursor_.eval());
        return ControlFlow::Continue;
    }

  private:
    F functor_;
    cursor_type cursor_;
};

template<typename E, typename F>
void for_each(const Expr<E>& expr, F fun) {
    EachCursor<E, F> cursor(expr.self(), std::move(fun));
    evaluate(cursor, expr.dims());
}

}  // namespace capibara