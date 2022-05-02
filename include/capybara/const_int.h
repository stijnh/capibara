#pragma once

#include <limits>

#include "util.h"

namespace capybara {

template<typename T, T Value>
struct ConstInt: std::integral_constant<T, Value> {
    operator T() const {
        return Value;
    }
};

template<typename T, T Value>
ConstInt<T, -Value> operator-(ConstInt<T, Value>) {
    return {};
}

template<typename T, T Value>
ConstInt<T, +Value> operator+(ConstInt<T, Value>) {
    return {};
}

#define IMPL_BINARY_OPERATOR(op, return_type)           \
    template<typename T, T Left, T Right>               \
    ConstInt<return_type, (Left op Right)> operator op( \
        ConstInt<T, Left>,                              \
        ConstInt<T, Right>) {                           \
        return {};                                      \
    }

IMPL_BINARY_OPERATOR(+, T)
IMPL_BINARY_OPERATOR(-, T)
IMPL_BINARY_OPERATOR(*, T)
IMPL_BINARY_OPERATOR(/, T)
IMPL_BINARY_OPERATOR(%, T)
IMPL_BINARY_OPERATOR(==, bool)
IMPL_BINARY_OPERATOR(!=, bool)
IMPL_BINARY_OPERATOR(<, bool)
IMPL_BINARY_OPERATOR(>, bool)
IMPL_BINARY_OPERATOR(<=, bool)
IMPL_BINARY_OPERATOR(>=, bool)
#undef IMPL_BINARY_OPERATOR

template<typename T>
CAPYBARA_INLINE constexpr T convert_integer(T value) {
    return value;
}

template<typename R, typename T, T Value>
CAPYBARA_INLINE constexpr ConstInt<R, (T)Value>
convert_integer(ConstInt<T, Value>) {
    static_assert(cmp_bounds<R>(Value), "value is out of bounds");
    return {};
}

template<typename R, typename T, T Value>
CAPYBARA_INLINE constexpr ConstInt<R, (T)Value>
convert_integer(std::integral_constant<T, Value>) {
    return convert_integer<R>(ConstInt<T, Value> {});
}

template<typename T, T N>
static constexpr ConstInt<T, N> const_int = {};

template<size_t N>
using ConstSize = ConstInt<size_t, N>;

template<size_t N>
static constexpr ConstSize<N> S = {};
static constexpr ConstSize<0> S0 = {};
static constexpr ConstSize<1> S1 = {};
static constexpr ConstSize<2> S2 = {};

CAPYBARA_INLINE constexpr size_t convert_size(size_t value) {
    return value;
}

template<typename T, T Value>
CAPYBARA_INLINE constexpr ConstSize<Value> convert_size(ConstInt<T, Value> v) {
    return convert_integer<size_t>(v);
}

template<ptrdiff_t N>
using ConstDiff = ConstInt<ptrdiff_t, N>;
using DynDiff = ptrdiff_t;

template<ptrdiff_t N>
static constexpr ConstDiff<N> D = {};

CAPYBARA_INLINE constexpr ptrdiff_t convert_diff(ptrdiff_t value) {
    return value;
}

template<typename T, T Value>
CAPYBARA_INLINE constexpr ConstDiff<(ptrdiff_t)Value>
convert_diff(ConstInt<T, Value> v) {
    return convert_integer<ptrdiff_t>(v);
}

template<typename T, T Value>
CAPYBARA_INLINE constexpr ConstDiff<(ptrdiff_t)Value>
convert_diff(std::integral_constant<T, Value> v) {
    return convert_integer<ptrdiff_t>(v);
}

template<bool b>
using ConstBool = ConstInt<bool, b>;
using ConstTrue = ConstBool<true>;
using ConstFalse = ConstBool<false>;

static constexpr ConstBool<true> const_true {};
static constexpr ConstBool<false> const_false {};

CAPYBARA_INLINE constexpr bool convert_bool(bool value) {
    return value;
}

template<typename T, T Value>
CAPYBARA_INLINE constexpr ConstBool<(bool)Value>
convert_bool(ConstInt<T, Value>) {
    return {};
}

template<typename T, T Value>
CAPYBARA_INLINE constexpr ConstBool<(bool)Value>
convert_bool(std::integral_constant<T, Value>) {
    return {};
}

}  // namespace capybara