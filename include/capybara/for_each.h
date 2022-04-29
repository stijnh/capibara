#pragma once

#include "eval.h"

namespace capybara {

template<typename E, typename F>
struct EachCursor {
    using Expr = E;
    using Cursor = typename Expr::Cursor;

    EachCursor(const Expr& e, F fun) :
        functor_(std::move(fun)),
        cursor_(e.cursor()) {}

    template<typename Axis, typename Diff>
    CAPYBARA_INLINE void advance(Axis axis, Diff diff) {
        cursor_.advance(axis, diff);
    }

    CAPYBARA_INLINE
    ControlFlow eval() const {
        functor_(cursor_.eval());
        return ControlFlow::Continue;
    }

  private:
    F functor_;
    Cursor cursor_;
};

template<typename E, typename F>
void for_each(const Expr<E>& expr, F fun) {
    EachCursor<E, F> cursor(expr.self(), std::move(fun));
    evaluate(cursor, expr.dims());
}

}  // namespace capybara