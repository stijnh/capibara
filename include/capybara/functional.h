#pragma once

#include <tuple>

#include "tuple.h"
#include "util.h"

namespace capybara {
namespace seq {

    template<size_t n, typename I, template<typename X, X value> class R>
    struct range_t {};

}  // namespace seq
}  // namespace capybara

namespace std {

template<size_t n, typename T, template<typename X, X> class R>
struct tuple_size<capybara::seq::range_t<n, T, R>>:
    std::integral_constant<size_t, n> {};

template<size_t n, size_t i, typename T, template<typename X, X> class R>
struct tuple_element<i, capybara::seq::range_t<n, T, R>> {
    using type = R<T, (T)i>;
};

template<size_t i, size_t n, typename T, template<typename X, X> class R>
capybara::ConstInt<T, (T)i> get(capybara::seq::range_t<n, T, R>) {
    return {};
}
}  // namespace std

namespace capybara {
namespace seq {
    namespace detail {
        template<size_t start, size_t end>
        struct ForEach {
            template<typename T, typename F>
            constexpr static void call(T&& tuple, F fun) {
                fun(std::get<start>(std::forward<T>(tuple)));
                ForEach<start + 1, end>::call(
                    std::forward<T>(tuple),
                    std::move(fun));
            }
        };

        template<size_t end>
        struct ForEach<end, end> {
            template<typename T, typename F>
            constexpr static void call(T&& tuple, F fun) {}
        };

        template<size_t start, size_t end>
        struct Fold {
            template<typename T, typename R, typename F>
            constexpr static R call(T&& tuple, R initial, F fun) {
                return Fold<start + 1, end>::call(
                    std::forward<T>(tuple),
                    fun(initial, std::get<start>(std::forward<T>(tuple))),
                    std::move(fun));
            }
        };

        template<size_t end>
        struct Fold<end, end> {
            template<typename T, typename R, typename F>
            constexpr static R call(T&& tuple, R initial, F fun) {
                return initial;
            }
        };

        template<
            template<typename... Xs>
            class R,
            typename F,
            typename T,
            typename Indices>
        struct Map;

        template<
            template<typename... Xs>
            class R,
            typename F,
            typename T,
            size_t... indices>
        struct Map<R, F, T, std::index_sequence<indices...>> {
            using type = R<decltype(std::declval<F>()(
                std::get<indices>(std::forward<T>(std::declval<T>()))))...>;

            constexpr static type call(T&& tuple, F fun) {
                return {fun(std::get<indices>(std::forward<T>(tuple)))...};
            }
        };
    }  // namespace detail

    template<
        size_t n,
        typename I = index_t,
        template<typename X, X value> class R = ConstInt>
    static constexpr range_t<n, I, R> range {};

    template<typename T, typename F>
    constexpr void for_each(T&& tuple, F fun) {
        constexpr size_t n = std::tuple_size<decay_t<T>>::value;
        detail::ForEach<0, n>::call(std::forward<T>(tuple), fun);
    }

    template<size_t n, typename I = index_t, typename F>
    constexpr void for_each_n(F fun) {
        for_each(range<n, I>, fun);
    }

    template<typename T, typename R, typename F>
    constexpr auto fold(T&& tuple, R initial, F fun) {
        constexpr size_t n = std::tuple_size<decay_t<T>>::value;
        return detail::Fold<0, n>::call(
            std::forward<T>(tuple),
            std::move(initial),
            fun);
    }

    template<size_t n, typename I = index_t, typename R, typename F>
    constexpr auto fold_n(R initial, F fun) {
        return fold(range<n, I>, std::move(initial), std::move(fun));
    }

    template<
        template<typename... Xs> class R = capybara::Tuple,
        typename T,
        typename F>
    constexpr auto map(T&& tuple, F fun) {
        constexpr size_t n = std::tuple_size<decay_t<T>>::value;
        using Indices = std::make_index_sequence<n>;
        return detail::Map<R, F, T, Indices>::call(std::forward<T>(tuple), fun);
    }

    template<
        size_t n,
        template<typename... Xs> class R = capybara::Tuple,
        typename I = index_t,
        typename F>
    constexpr auto map_n(F fun) {
        return map<R>(range<n, I>, fun);
    }

    template<typename T, typename F>
    constexpr bool all(T&& tuple, F fun) {
        return fold(
            std::forward<T>(tuple),
            true,
            [=](bool lhs, const auto& rhs) { return lhs && fun(rhs); });
    }

    template<size_t n, typename I = index_t, typename F>
    constexpr bool all_n(F fun) {
        return all(range<n, I>, fun);
    }

    template<typename T, typename F>
    constexpr bool any(T&& tuple, F fun) {
        return fold(
            std::forward<T>(tuple),
            false,
            [=](bool lhs, const auto& rhs) { return lhs || fun(rhs); });
    }

    template<size_t n, typename I = index_t, typename F>
    constexpr bool any_n(F fun) {
        return any_n(range<n, I>, fun);
    }

}  // namespace seq

namespace functional {
    constexpr bool all() {
        return true;
    }

    template<typename... Ts>
    constexpr bool all(bool first, Ts... rest) {
        return first && all(rest...);
    }

    constexpr bool any() {
        return false;
    }

    template<typename... Ts>
    constexpr bool any(bool first, Ts... rest) {
        return first || any(rest...);
    }
}  // namespace functional
}  // namespace capybara
