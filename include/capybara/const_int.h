#pragma once

#include <type_traits>

#include "defines.h"
#include "types.h"

namespace capybara {

template<typename T, T value>
struct ConstInt: std::integral_constant<T, value> {
    constexpr operator T() const {
        return value;
    }

    static T get() {
        return value;
    }

    static void set(T input) {
        if (value != input) {
            throw std::runtime_error("cannot set constant value");
        }
    }

    ConstInt& operator=(T input) {
        ConstInt::set(input);
        return *this;
    }
};

#define CAPYBARA_CONST_INT_UNARY_OP(op, return_type)                  \
    template<typename T, T value>                                     \
    ConstInt<return_type, op value> operator op(ConstInt<T, value>) { \
        return {};                                                    \
    }

CAPYBARA_CONST_INT_UNARY_OP(+, T)
CAPYBARA_CONST_INT_UNARY_OP(-, T)
CAPYBARA_CONST_INT_UNARY_OP(!, bool)
#undef CAPYBARA_CONST_INT_UNARY_OP

#define CAPYBARA_CONST_INT_BINARY_OP(op, return_type)   \
    template<typename T, T left, T right>               \
    ConstInt<return_type, (left op right)> operator op( \
        ConstInt<T, left>,                              \
        ConstInt<T, right>) {                           \
        return {};                                      \
    }

CAPYBARA_CONST_INT_BINARY_OP(+, T)
CAPYBARA_CONST_INT_BINARY_OP(-, T)
CAPYBARA_CONST_INT_BINARY_OP(*, T)
CAPYBARA_CONST_INT_BINARY_OP(/, T)
CAPYBARA_CONST_INT_BINARY_OP(%, T)
CAPYBARA_CONST_INT_BINARY_OP(==, bool)
CAPYBARA_CONST_INT_BINARY_OP(!=, bool)
CAPYBARA_CONST_INT_BINARY_OP(<, bool)
CAPYBARA_CONST_INT_BINARY_OP(>, bool)
CAPYBARA_CONST_INT_BINARY_OP(<=, bool)
CAPYBARA_CONST_INT_BINARY_OP(>=, bool)
#undef CAPYBARA_CONST_INT_BINARY_OP

template<stride_t I>
using ConstStride = ConstInt<stride_t, I>;

template<stride_t I>
static constexpr ConstStride<I> const_stride {};

CAPYBARA_INLINE constexpr stride_t into_stride(stride_t i) {
    return i;
}

template<stride_t I>
CAPYBARA_INLINE constexpr ConstStride<I>
into_stride(const std::integral_constant<stride_t, I>&) {
    return {};
}

template<index_t I>
using ConstIndex = ConstInt<index_t, I>;

template<index_t I>
static constexpr ConstIndex<I> const_index {};

CAPYBARA_INLINE constexpr index_t into_index(index_t i) {
    return i;
}

template<index_t I>
CAPYBARA_INLINE constexpr ConstIndex<I>
into_index(const std::integral_constant<index_t, I>&) {
    return {};
}

template<length_t I>
using ConstLength = ConstInt<length_t, I>;

template<length_t I>
static constexpr ConstLength<I> const_length {};

CAPYBARA_INLINE constexpr length_t into_length(length_t i) {
    return i;
}

template<length_t I>
CAPYBARA_INLINE constexpr ConstLength<I>
into_length(const std::integral_constant<length_t, I>&) {
    return {};
}

template<bool b>
using ConstBool = ConstInt<bool, b>;
using ConstTrue = ConstBool<true>;
using ConstFalse = ConstBool<false>;

static constexpr ConstBool<true> const_true {};
static constexpr ConstBool<false> const_false {};

CAPYBARA_INLINE constexpr bool into_bool(bool value) {
    return value;
}

template<typename T, T value>
CAPYBARA_INLINE constexpr ConstBool<(bool)value>
into_bool(const std::integral_constant<T, value>&) {
    return value;
}

}  // namespace capybara