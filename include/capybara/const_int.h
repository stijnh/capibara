#pragma once

#include <sstream>
#include <stdexcept>
#include <type_traits>

#include "defines.h"
#include "types.h"
#include "util.h"

namespace capybara {

template<typename T, T value>
struct ConstInt: std::integral_constant<T, value> {
    CAPYBARA_INLINE
    constexpr ConstInt() {}
    CAPYBARA_INLINE
    ConstInt(T input) {
        set(input);
    }

    CAPYBARA_INLINE
    constexpr operator T() const {
        return value;
    }

    CAPYBARA_INLINE
    constexpr static T get() {
        return value;
    }

    CAPYBARA_INLINE
    static void set(T input) {
        if (value != input) {
            std::stringstream ss;
            ss << "cannot set constant value: expecting value " << value
               << ", given value" << input;
            throw std::runtime_error(ss.str());
        }
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
into_bool(std::integral_constant<T, value>) {
    return value;
}

template<index_t i>
using ConstIndex = ConstInt<index_t, i>;

template<index_t i>
static constexpr ConstIndex<i> const_index {};

namespace detail {
    template<index_t upper, typename = void>
    struct DynIndexStorage {
        DynIndexStorage(index_t v) : inner_(v) {}

      protected:
        CAPYBARA_INLINE
        index_t get_impl() const {
            index_t v = inner_;

            if (v < 0) {
                CAPYBARA_UNREACHABLE;
            }

            if (v >= upper) {
                CAPYBARA_UNREACHABLE;
            }

            return inner_;
        }

      private:
        index_t inner_;
    };

    template<index_t upper>
    struct DynIndexStorage<
        upper,
        enable_t<(upper > 0 && upper <= std::numeric_limits<uint8_t>::max())>> {
        DynIndexStorage(index_t v) : inner_(v) {}

      protected:
        CAPYBARA_INLINE
        index_t get_impl() const {
            index_t v = inner_;

            if (v < 0) {
                CAPYBARA_UNREACHABLE;
            }

            if (v >= upper) {
                CAPYBARA_UNREACHABLE;
            }

            return v;
        }

      private:
        uint8_t inner_;
    };

    template<>
    struct DynIndexStorage<1> {
        DynIndexStorage(index_t v) {}

      protected:
        CAPYBARA_INLINE
        index_t get_impl() const {
            return 0;
        }
    };
}  // namespace detail

template<index_t upper = std::numeric_limits<index_t>::max()>
struct DynIndex: private detail::DynIndexStorage<upper> {
    static_assert(upper >= 0, "upper must be non-negative");
    using Base = detail::DynIndexStorage<upper>;

    CAPYBARA_INLINE
    DynIndex(index_t v) : Base(v) {
        if (v < 0 || v >= upper) {
            throw std::runtime_error("index out of bounds");
        }
    }

    template<index_t i>
    CAPYBARA_INLINE DynIndex(ConstIndex<i>) : DynIndex(i) {}

    CAPYBARA_INLINE
    DynIndex() : DynIndex(0) {}

    CAPYBARA_INLINE
    index_t get() const {
        return this->get_impl();
    }

    CAPYBARA_INLINE
    operator index_t() const {
        return get();
    }

    CAPYBARA_INLINE
    void set(index_t v) const {
        *this = DynIndex(v);
    }
};

template<
    index_t n,
    index_t m,
    typename = enable_t<
        (n + m > 1) && (n - 1 <= std::numeric_limits<index_t>::max() - m)>>
DynIndex<n + m - 1> operator+(DynIndex<n> lhs, DynIndex<m> rhs) {
    return lhs.get() + rhs.get();
}

template<index_t n, index_t m>
DynIndex<n + m> operator+(DynIndex<n> lhs, ConstIndex<m> rhs) {
    return lhs.get() + rhs.get();
}

template<index_t n, index_t m>
DynIndex<n + m> operator+(ConstIndex<n> lhs, DynIndex<m> rhs) {
    return lhs.get() + rhs.get();
}

template<index_t upper>
CAPYBARA_INLINE constexpr DynIndex<upper> into_index(index_t value) {
    return value;
}

template<index_t upper, typename T, T value>
CAPYBARA_INLINE constexpr enable_t<
    std::is_integral<T>::value
        && (cmp_greater_equal(value, 0) && cmp_less(value, upper)),
    ConstIndex<(index_t)value>>
into_index(std::integral_constant<T, value>) {
    return {};
}

template<index_t upper, typename T>
CAPYBARA_INLINE constexpr void assert_index_bound(T value) {
    into_index<upper>(value);  // throws error if T is invalid
}

#define DEFINE_FIXED_AXIS_CONSTANT(N) static constexpr ConstIndex<N> axis##N {};
DEFINE_FIXED_AXIS_CONSTANT(0)
DEFINE_FIXED_AXIS_CONSTANT(1)
DEFINE_FIXED_AXIS_CONSTANT(2)
DEFINE_FIXED_AXIS_CONSTANT(3)
DEFINE_FIXED_AXIS_CONSTANT(4)
DEFINE_FIXED_AXIS_CONSTANT(5)
DEFINE_FIXED_AXIS_CONSTANT(6)
#undef DEFINE_FIXED_AXIS_CONSTANT

}  // namespace capybara