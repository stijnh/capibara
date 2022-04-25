#pragma once
#include "types.h"

namespace capibara {
template<typename T>
struct ExprTraits {};

template<typename E>
using expr_value_type = typename ExprTraits<E>::value_type;

enum struct AccessMode {
    ReadWrite,
    ReadOnly,
};

template<typename Derived, AccessMode = AccessMode::ReadOnly>
struct Expr {};

template<typename Derived>
using View = Expr<Derived, AccessMode::ReadWrite>;

template<typename F, typename Dims>
struct NullaryExpr;

template<typename F, typename Op>
struct UnaryExpr;

template<typename F, typename L, typename R>
struct BinaryExpr;

template<typename F, typename Op>
struct MappingExpr;

template<typename T, size_t N>
struct Array;
}  // namespace capibara