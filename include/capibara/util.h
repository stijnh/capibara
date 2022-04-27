#pragma once
#include <limits>

#include "defines.h"

namespace capibara {
namespace detail {
    template<typename L, typename R, typename = void>
    struct Compare {
        CAPIBARA_INLINE
        constexpr static bool lt(L left, R right) {
            return left < right;
        }

        CAPIBARA_INLINE
        constexpr static bool gt(L left, R right) {
            return left > right;
        }

        CAPIBARA_INLINE
        constexpr static bool eq(L left, R right) {
            return left == right;
        }

        CAPIBARA_INLINE
        constexpr static bool ne(L left, R right) {
            return left != right;
        }

        CAPIBARA_INLINE
        constexpr static bool le(L left, R right) {
            return left <= right;
        }

        CAPIBARA_INLINE
        constexpr static bool ge(L left, R right) {
            return left >= right;
        }
    };

    template<typename L, typename R>
    struct Compare<
        L,
        R,
        typename std::enable_if<
            std::is_signed<typename std::decay<L>::type>::value
            && std::is_unsigned<typename std::decay<R>::type>::value>::type> {
        using Self = Compare<L, R>;
        using UL =
            typename std::make_unsigned<typename std::decay<L>::type>::type;

        CAPIBARA_INLINE
        constexpr static bool lt(L left, R right) noexcept {
            return left < 0 || (UL(left) < right);
        }

        CAPIBARA_INLINE
        constexpr static bool eq(L left, R right) noexcept {
            return (left >= 0) && (UL(left) == right);
        }

        CAPIBARA_INLINE
        constexpr static bool gt(L left, R right) noexcept {
            return Self::lt(right, left);
        }

        CAPIBARA_INLINE
        constexpr static bool le(L left, R right) noexcept {
            return !Self::gt(left, right);
        }

        CAPIBARA_INLINE
        constexpr static bool ne(L left, R right) noexcept {
            return !Self::eq(right, left);
        }

        CAPIBARA_INLINE
        constexpr static bool ge(L left, R right) noexcept {
            return !Self::lt(right, left);
        }
    };

    template<typename L, typename R>
    struct Compare<
        L,
        R,
        typename std::enable_if<
            std::is_unsigned<typename std::decay<L>::type>::value
            && std::is_signed<typename std::decay<R>::type>::value>::type> {
        using Reverse = Compare<R, L>;

        CAPIBARA_INLINE
        constexpr static bool eq(L left, R right) noexcept {
            return Reverse::eq(right, left);
        }

        CAPIBARA_INLINE
        constexpr static bool ne(L left, R right) noexcept {
            return Reverse::ne(right, left);
        }

        CAPIBARA_INLINE
        constexpr static bool lt(L left, R right) noexcept {
            return Reverse::gt(right, left);
        }

        CAPIBARA_INLINE
        constexpr static bool gt(L left, R right) noexcept {
            return Reverse::lt(right, left);
        }

        CAPIBARA_INLINE
        constexpr static bool le(L left, R right) noexcept {
            return Reverse::ge(right, left);
        }

        CAPIBARA_INLINE
        constexpr static bool ge(L left, R right) noexcept {
            return Reverse::le(right, left);
        }
    };
}  // namespace detail

template<typename L, typename R>
CAPIBARA_INLINE constexpr bool cmp_less(const L& left, const R& right) {
    return detail::Compare<const L&, const R&>::lt(left, right);
}

template<typename L, typename R>
CAPIBARA_INLINE constexpr bool cmp_greater(const L& left, const R& right) {
    return detail::Compare<const L&, const R&>::gt(left, right);
}

template<typename L, typename R>
CAPIBARA_INLINE constexpr bool cmp_less_equal(const L& left, const R& right) {
    return detail::Compare<const L&, const R&>::le(left, right);
}

template<typename L, typename R>
CAPIBARA_INLINE constexpr bool
cmp_greater_equal(const L& left, const R& right) {
    return detail::Compare<const L&, const R&>::ge(left, right);
}

template<typename L, typename R>
CAPIBARA_INLINE constexpr bool cmp_equal(const L& left, const R& right) {
    return detail::Compare<const L&, const R&>::eq(left, right);
}

template<typename L, typename R>
CAPIBARA_INLINE constexpr bool cmp_not_equal(const L& left, const R& right) {
    return detail::Compare<const L&, const R&>::ne(left, right);
}

template<typename L, typename A, typename B>
CAPIBARA_INLINE constexpr bool
cmp_bounds(const L& value, const A& lo, const B& hi) {
    return cmp_greater_equal(value, lo) && cmp_less_equal(value, hi);
}

template<typename R, typename L>
CAPIBARA_INLINE constexpr bool cmp_bounds(const L& value) {
    return cmp_bounds(
        value,
        std::numeric_limits<R>::min(),
        std::numeric_limits<R>::max());
}

}  // namespace capibara