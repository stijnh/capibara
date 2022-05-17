#pragma once

#include <complex>
#include <vector>

#include "array.h"
#include "forwards.h"
#include "nullary.h"
#include "util.h"

namespace capybara {
namespace detail {
    template<typename T, typename = void>
    struct into_trivial_type {};

    template<typename T, typename = void>
    struct is_trivial_type: std::false_type {};

    template<typename T>
    struct is_trivial_type<T, void_t<typename into_trivial_type<T>::type>>:
        std::true_type {};

    template<typename T>
    struct into_trivial_type<const T>: into_trivial_type<T> {};

    template<typename T>
    struct into_trivial_type<const T&>: into_trivial_type<T> {};

    template<typename T>
    struct into_trivial_type<T&>: into_trivial_type<T> {};

    template<typename T>
    struct into_trivial_type<T&&>: into_trivial_type<T> {};

    template<typename T>
    struct into_trivial_type<T, enable_t<std::is_integral<T>::value>> {
        using type = T;
    };

    template<>
    struct into_trivial_type<std::string> {
        using type = std::string;
    };

    template<typename T>
    struct into_trivial_type<std::complex<T>> {
        using type = std::complex<T>;
    };

    template<typename A, typename B>
    struct into_trivial_type<
        std::pair<A, B>,
        enable_t<is_trivial_type<A>::value && is_trivial_type<B>::value>> {
        using type = std::pair<
            typename into_trivial_type<A>::type,
            typename into_trivial_type<B>::type>;
    };

    template<typename... Ts>
    struct into_trivial_type<
        std::tuple<Ts...>,
        enable_t<all(is_trivial_type<Ts>::value...)>> {
        using type = std::tuple<typename into_trivial_type<Ts>::type...>;
    };
}  // namespace detail

template<typename T>
struct expr_conversion<T, enable_t<detail::is_trivial_type<T>::value>> {
    using type = scalar_type<typename detail::into_trivial_type<T>::type>;

    static type call(T&& expr) {
        return scalar(std::forward<T>(expr));
    }
};

template<typename T>
struct expr_conversion<std::vector<T>&> {
    using type = array_base<layout::default_layout<1>, storage::span<T>>;

    static type call(std::vector<T>& expr) {
        return {
            layout::default_layout<1> {{expr.size()}},
            storage::span<T>(expr.data())};
    }
};

template<typename T>
struct expr_conversion<const std::vector<T>&> {
    using type = array_base<layout::default_layout<1>, storage::span<const T>>;

    static type call(const std::vector<T>& expr) {
        return {
            layout::default_layout<1> {{expr.size()}},
            storage::span<const T>(expr.data())};
    }
};

template<typename T, size_t N>
struct expr_conversion<std::array<T, N>&> {
    using type = array_base<layout::default_layout<1>, storage::span<T>>;

    static type call(std::array<T, N>& expr) {
        return {layout::default_layout<1> {{N}}, storage::span<T>(expr.data())};
    }
};

template<typename T, size_t N>
struct expr_conversion<const std::array<T, N>&> {
    using type = array_base<layout::default_layout<1>, storage::span<const T>>;

    static type call(const std::array<T, N>& expr) {
        return {
            layout::default_layout<1> {{N}},
            storage::span<const T>(expr.data())};
    }
};
}  // namespace capybara