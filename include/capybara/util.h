#pragma once
#include <limits>
#include <type_traits>

#include "defines.h"

namespace capybara {
namespace detail {
    template<typename L, typename R, typename = void>
    struct Compare {
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
    struct Compare<
        L,
        R,
        typename std::enable_if<
            std::is_signed<typename std::decay<L>::type>::value
            && std::is_unsigned<typename std::decay<R>::type>::value>::type> {
        using Self = Compare<L, R>;
        using UL =
            typename std::make_unsigned<typename std::decay<L>::type>::type;

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
    struct Compare<
        L,
        R,
        typename std::enable_if<
            std::is_unsigned<typename std::decay<L>::type>::value
            && std::is_signed<typename std::decay<R>::type>::value>::type> {
        using Reverse = Compare<R, L>;

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
    return detail::Compare<const L&, const R&>::lt(left, right);
}

template<typename L, typename R>
CAPYBARA_INLINE constexpr bool cmp_greater(const L& left, const R& right) {
    return detail::Compare<const L&, const R&>::gt(left, right);
}

template<typename L, typename R>
CAPYBARA_INLINE constexpr bool cmp_less_equal(const L& left, const R& right) {
    return detail::Compare<const L&, const R&>::le(left, right);
}

template<typename L, typename R>
CAPYBARA_INLINE constexpr bool
cmp_greater_equal(const L& left, const R& right) {
    return detail::Compare<const L&, const R&>::ge(left, right);
}

template<typename L, typename R>
CAPYBARA_INLINE constexpr bool cmp_equal(const L& left, const R& right) {
    return detail::Compare<const L&, const R&>::eq(left, right);
}

template<typename L, typename R>
CAPYBARA_INLINE constexpr bool cmp_not_equal(const L& left, const R& right) {
    return detail::Compare<const L&, const R&>::ne(left, right);
}

template<typename L, typename A, typename B>
CAPYBARA_INLINE constexpr bool
cmp_bounds(const L& value, const A& lo, const B& hi) {
    return cmp_greater_equal(value, lo) && cmp_less_equal(value, hi);
}

template<typename R, typename L>
CAPYBARA_INLINE constexpr bool cmp_bounds(const L& value) {
    return cmp_bounds(
        value,
        std::numeric_limits<R>::min(),
        std::numeric_limits<R>::max());
}

namespace detail {
    template<typename L, typename R, typename O, typename = void>
    struct Arithmetic {
        static constexpr O add(const L& left, const R& right) {
            return left + right;
        }

        static constexpr O sub(const L& left, const R& right) {
            return left - right;
        }

        static constexpr O mul(const L& left, const R& right) {
            return left * right;
        }

        static constexpr O div(const L& left, const R& right) {
            return left / right;
        }
    };

}  // namespace detail

template<
    typename L,
    typename R,
    typename O = decltype(std::declval<L>() + std::declval<R>())>
CAPYBARA_INLINE constexpr O safe_add(const L& left, const R& right) {
    return detail::Arithmetic<L, R, O>::add(left, right);
}

template<
    typename L,
    typename R,
    typename O = decltype(std::declval<L>() - std::declval<R>())>
CAPYBARA_INLINE constexpr O safe_sub(const L& left, const R& right) {
    return detail::Arithmetic<L, R, O>::sub(left, right);
}

template<
    typename L,
    typename R,
    typename O = decltype(std::declval<L>() * std::declval<R>())>
CAPYBARA_INLINE constexpr O safe_mul(const L& left, const R& right) {
    return detail::Arithmetic<L, R, O>::mul(left, right);
}

template<
    typename L,
    typename R,
    typename O = decltype(std::declval<L>() / std::declval<R>())>
CAPYBARA_INLINE constexpr O safe_div(const L& left, const R& right) {
    return detail::Arithmetic<L, R, O>::div(left, right);
}

}  // namespace capybara