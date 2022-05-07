#pragma once

#include "eval.h"
#include "expr.h"
#include "forwards.h"

namespace capybara {

template<typename Lhs, typename Rhs, AssignOp op>
struct AssignCursor;

template<typename Lhs, typename Rhs, AssignOp op>
struct ExprTraits<AssignExpr<Lhs, Rhs, op>> {
    static_assert(Lhs::rank == Rhs::rank, "invalid rank");

    static constexpr size_t rank = Lhs::rank;
    static constexpr AccessMode mode = AccessMode::ReadOnly;
    static constexpr bool is_view = false;
    using Value = ControlFlow;
    using Nested = AssignExpr<Lhs, Rhs, op>;
    using NestedConst = AssignExpr<Lhs, Rhs, op>;
};

template<typename Lhs, typename Rhs, AssignOp op>
struct AssignExpr: Expr<AssignExpr<Lhs, Rhs, op>> {
    AssignExpr(Lhs lhs, Rhs rhs) : lhs_(lhs), rhs_(rhs) {}

    template<typename Axis>
    auto dim_impl(Axis axis) const {
        return lhs_.dim(axis);  // TODO
    }

    template<typename Device>
    auto cursor_impl(Device device) {
        auto l = lhs_.cursor(device);
        auto r = rhs_.cursor(device);
        return AssignCursor<decltype(l), decltype(r), op>(
            std::move(l),
            std::move(r));
    }

  private:
    Lhs lhs_;
    Rhs rhs_;
};

namespace detail {
    template<AssignOp>
    struct AssignImpl;

    template<>
    struct AssignImpl<AssignOp::Simple> {
        template<typename Lhs, typename Rhs>
        static void call(Lhs& lhs, Rhs& rhs) {
            lhs.store(rhs.evaluate());
        }
    };

    template<>
    struct AssignImpl<AssignOp::Add> {
        template<typename Lhs, typename Rhs>
        static void call(Lhs& lhs, Rhs& rhs) {
            lhs.store(lhs.evaluate() + rhs.evaluate());
        }
    };

    template<>
    struct AssignImpl<AssignOp::Sub> {
        template<typename Lhs, typename Rhs>
        static void call(Lhs& lhs, Rhs& rhs) {
            lhs.store(lhs.evaluate() + rhs.evaluate());
        }
    };

    template<>
    struct AssignImpl<AssignOp::Mul> {
        template<typename Lhs, typename Rhs>
        static void call(Lhs& lhs, Rhs& rhs) {
            lhs.store(lhs.evaluate() * rhs.evaluate());
        }
    };

    template<>
    struct AssignImpl<AssignOp::Div> {
        template<typename Lhs, typename Rhs>
        static void call(Lhs& lhs, Rhs& rhs) {
            lhs.store(lhs.evaluate() / rhs.evaluate());
        }
    };

    template<>
    struct AssignImpl<AssignOp::Mod> {
        template<typename Lhs, typename Rhs>
        static void call(Lhs& lhs, Rhs& rhs) {
            lhs.store(lhs.evaluate() % rhs.evaluate());
        }
    };
}  // namespace detail

template<typename Lhs, typename Rhs, AssignOp op>
struct AssignCursor {
    AssignCursor(Lhs lhs, Rhs rhs) : lhs_(lhs), rhs_(rhs) {}

    template<typename Axis, typename Steps>
    void advance(Axis axis, Steps steps) {
        lhs_.advance(axis, steps);
        rhs_.advance(axis, steps);
    }

    ControlFlow evaluate() {
        detail::AssignImpl<op>::call(lhs_, rhs_);
        return ControlFlow::Continue;
    }

  private:
    Lhs lhs_;
    Rhs rhs_;
};

template<AssignOp op = AssignOp::Simple, typename Lhs, typename Rhs>
CAPYBARA_NOINLINE void execute_assign(Lhs lhs, Rhs rhs) {
    auto l = lhs.nested();
    auto r = rhs.nested();
    auto expr =
        AssignExpr<decltype(l), decltype(r), op>(std::move(l), std::move(r));

    auto cursor = expr.cursor();
    evaluate(cursor, expr.dims());
}

}  // namespace capybara