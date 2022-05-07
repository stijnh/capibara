#pragma once

namespace capybara {

enum AssignOp { Simple, Add, Sub, Mul, Div, Mod };

enum class AccessMode {
    ReadOnly,
    ReadWrite,
    WriteOnly,
};

template<typename T>
struct ExprTraits;

template<typename Derived>
struct Expr;

template<typename T, typename D>
struct ArrayBase;

template<typename Lhs, typename Rhs, AssignOp op>
struct AssignExpr;

}  // namespace capybara