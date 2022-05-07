#pragma once

#include <type_traits>

#include "axis.h"

namespace capybara {
template<index_t... axes>
using AxesOrder = std::integer_sequence<index_t, axes...>;

namespace axes {
    namespace detail {

        template<index_t I, typename Axes>
        struct Contains;

        template<index_t I, index_t first, index_t... rest>
        struct Contains<I, AxesOrder<first, rest...>>:
            ConstBool<I == first || Contains<I, AxesOrder<rest...>>::value> {};

        template<index_t I>
        struct Contains<I, AxesOrder<>>: ConstFalse {};

        template<index_t N, typename Axes>
        struct HasRank;

        template<index_t N, index_t First, index_t... Rest>
                struct HasRank<N, AxesOrder<First, Rest...>>:
            ConstBool
            < First
            >= 0
            && First<N && HasRank<N, AxesOrder<Rest...>>::value> {};

        template<index_t N>
        struct HasRank<N, AxesOrder<>>: ConstTrue {};

        template<typename Axes>
        struct IsDistinct;

        template<index_t First, index_t... Axes>
        struct IsDistinct<AxesOrder<First, Axes...>>:
            ConstBool<
                !Contains<First, AxesOrder<Axes...>>::value
                && IsDistinct<AxesOrder<Axes...>>::value> {};

        template<>
        struct IsDistinct<AxesOrder<>>: ConstTrue {};

        template<typename Axes>
        struct IsPermutation: ConstFalse {};

        template<index_t... Axes>
        struct IsPermutation<AxesOrder<Axes...>>:
            ConstBool<
                HasRank<sizeof...(Axes), AxesOrder<Axes...>>::value
                && IsDistinct<AxesOrder<Axes...>>::value> {};

        template<size_t N, typename = std::make_index_sequence<N>>
        struct Sequence;

        template<size_t N, size_t... indices>
        struct Sequence<N, std::index_sequence<indices...>> {
            using type = AxesOrder<indices...>;
        };

    }  // namespace detail

    template<typename Axes>
    constexpr static bool is_permutation = detail::IsPermutation<Axes>::value;

    template<size_t N>
    using seq = typename detail::Sequence<N>::type;

}  // namespace axes

}  // namespace capybara