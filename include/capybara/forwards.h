#pragma once
#include "types.h"

namespace capybara {
template<typename T>
struct ExprTraits {};

template<typename T>
struct ExprTraits<T&>: ExprTraits<T> {};

template<typename T>
struct ExprTraits<T&&>: ExprTraits<T> {};

template<typename T>
struct ExprTraits<T*>: ExprTraits<T> {};

template<typename T>
struct ExprTraits<const T>: ExprTraits<T> {};

template<typename E>
using ExprValue = typename ExprTraits<E>::Value;

enum struct AccessMode {
    ReadWrite,
    ReadOnly,
};

template<typename Derived, AccessMode = AccessMode::ReadOnly>
struct Expr;

template<typename Derived>
using View = Expr<Derived, AccessMode::ReadWrite>;

template<typename F, typename Dims>
struct NullaryExpr;

template<typename F, typename Op>
struct UnaryExpr;

template<typename F, typename L, typename R>
struct BinaryExpr;

template<typename M, typename E>
struct MappingExpr;

template<typename T, typename D>
struct ArrayBase;

template<typename D, typename I = size_t>
struct IndexedExpr;

}  // namespace capybara