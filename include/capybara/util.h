#pragma once
#include <limits>
#include <type_traits>

#include "defines.h"

namespace capybara {

template<bool cond>
using enabled = typename std::enable_if<cond>::type;

template<typename T>
using decayed = typename std::decay<T>::type;

namespace detail {
    template<typename L, typename R, typename = void>
    struct CompareHelper {
        CAPYBARA_INLINE
        constexpr static bool lt(const L& left, const R& right) {
            return left < right;
        }

        CAPYBARA_INLINE
        constexpr static bool gt(const L& left, const R& right) {
            return left > right;
        }

        CAPYBARA_INLINE
        constexpr static bool eq(const L& left, const R& right) {
            return left == right;
        }

        CAPYBARA_INLINE
        constexpr static bool ne(const L& left, const R& right) {
            return left != right;
        }

        CAPYBARA_INLINE
        constexpr static bool le(const L& left, const R& right) {
            return left <= right;
        }

        CAPYBARA_INLINE
        constexpr static bool ge(const L& left, const R& right) {
            return left >= right;
        }
    };

    template<typename L, typename R>
    struct CompareHelper<
        L,
        R,
        enabled<
            std::is_signed<decayed<L>>::value
            && std::is_unsigned<decayed<R>>::value>> {
        using Self = CompareHelper<L, R>;
        using UL = typename std::make_unsigned<decayed<L>>::type;

        CAPYBARA_INLINE
        constexpr static bool lt(const L& left, const R& right) noexcept {
            return left < 0 || (UL(left) < right);
        }

        CAPYBARA_INLINE
        constexpr static bool eq(const L& left, const R& right) noexcept {
            return (left >= 0) && (UL(left) == right);
        }

        CAPYBARA_INLINE
        constexpr static bool le(const L& left, const R& right) noexcept {
            return left < 0 || (UL(left) <= right);
        }

        CAPYBARA_INLINE
        constexpr static bool gt(const L& left, const R& right) noexcept {
            return !Self::le(left, right);
        }

        CAPYBARA_INLINE
        constexpr static bool ne(const L& left, const R& right) noexcept {
            return !Self::eq(left, right);
        }

        CAPYBARA_INLINE
        constexpr static bool ge(const L& left, const R& right) noexcept {
            return !Self::lt(left, right);
        }
    };

    template<typename L, typename R>
    struct CompareHelper<
        L,
        R,
        enabled<
            std::is_unsigned<decayed<L>>::value
            && std::is_signed<decayed<R>>::value>> {
        using Reverse = CompareHelper<R, L>;

        CAPYBARA_INLINE
        constexpr static bool eq(const L& left, const R& right) noexcept {
            return Reverse::eq(right, left);
        }

        CAPYBARA_INLINE
        constexpr static bool ne(const L& left, const R& right) noexcept {
            return Reverse::ne(right, left);
        }

        CAPYBARA_INLINE
        constexpr static bool lt(const L& left, const R& right) noexcept {
            return Reverse::gt(right, left);
        }

        CAPYBARA_INLINE
        constexpr static bool gt(const L& left, const R& right) noexcept {
            return Reverse::lt(right, left);
        }

        CAPYBARA_INLINE
        constexpr static bool le(const L& left, const R& right) noexcept {
            return Reverse::ge(right, left);
        }

        CAPYBARA_INLINE
        constexpr static bool ge(const L& left, const R& right) noexcept {
            return Reverse::le(right, left);
        }
    };
}  // namespace detail

template<typename L, typename R>
CAPYBARA_INLINE constexpr bool cmp_less(const L& left, const R& right) {
    return detail::CompareHelper<L, R>::lt(left, right);
}

template<typename L, typename R>
CAPYBARA_INLINE constexpr bool cmp_greater(const L& left, const R& right) {
    return detail::CompareHelper<L, R>::gt(left, right);
}

template<typename L, typename R>
CAPYBARA_INLINE constexpr bool cmp_less_equal(const L& left, const R& right) {
    return detail::CompareHelper<L, R>::le(left, right);
}

template<typename L, typename R>
CAPYBARA_INLINE constexpr bool
cmp_greater_equal(const L& left, const R& right) {
    return detail::CompareHelper<L, R>::ge(left, right);
}

template<typename L, typename R>
CAPYBARA_INLINE constexpr bool cmp_equal(const L& left, const R& right) {
    return detail::CompareHelper<L, R>::eq(left, right);
}

template<typename L, typename R>
CAPYBARA_INLINE constexpr bool cmp_not_equal(const L& left, const R& right) {
    return detail::CompareHelper<L, R>::ne(left, right);
}

template<typename L, typename A, typename B>
CAPYBARA_INLINE constexpr bool
in_range(const L& value, const A& lo, const B& hi) {
    return cmp_greater_equal(value, lo) && cmp_less_equal(value, hi);
}

template<typename R, typename L>
CAPYBARA_INLINE constexpr bool in_range(const L& value) {
    return in_range(
        value,
        std::numeric_limits<R>::min(),
        std::numeric_limits<R>::max());
}

}  // namespace capybara