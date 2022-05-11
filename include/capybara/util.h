#pragma once
#include <limits>
#include <type_traits>

#include "defines.h"

namespace capybara {

template<bool cond, typename Type = void>
using enable_t = typename std::enable_if<cond, Type>::type;

template<typename F, typename... Args>
using apply_t = typename std::result_of<F(Args...)>::type;

template<typename... Ts>
using common_t = typename std::common_type<Ts...>::type;

template<typename T>
using decay_t = typename std::decay<T>::type;

template<typename... Ts>
using void_t = void;

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
        enable_t<
            std::is_signed<decay_t<L>>::value
            && std::is_unsigned<decay_t<R>>::value>> {
        using Self = CompareHelper<L, R>;
        using UL = typename std::make_unsigned<decay_t<L>>::type;

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
        enable_t<
            std::is_unsigned<decay_t<L>>::value
            && std::is_signed<decay_t<R>>::value>> {
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