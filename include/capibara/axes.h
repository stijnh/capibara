#include <stddef.h>

#include <type_traits>
#include <utility>

#include "const_int.h"

namespace capibara {

template<size_t... Axes>
using AxesOrder = std::index_sequence<Axes...>;

namespace axes_helper {
    template<typename... Axes>
    struct Join;

    template<typename A>
    struct Join<A> {
        using type = A;
    };

    template<size_t... Head, size_t... Tail, typename... Rest>
    struct Join<AxesOrder<Head...>, AxesOrder<Tail...>, Rest...> {
        using type = typename Join<AxesOrder<Head..., Tail...>, Rest...>::type;
    };

    template<size_t I, typename Axes>
    struct Contains;

    template<size_t I, size_t J, size_t... Axes>
    struct Contains<I, AxesOrder<J, Axes...>>:
        ConstBool<I == J || Contains<I, AxesOrder<Axes...>>::value> {};

    template<size_t I>
    struct Contains<I, AxesOrder<>>: ConstFalse {};

    template<size_t N, typename Axes>
    struct HasRank;

    template<size_t N, size_t I, size_t... Axes>
        struct HasRank<N, AxesOrder<I, Axes...>>:
        ConstBool
        < I<N && HasRank<N, AxesOrder<Axes...>>::value> {};

    template<size_t N>
    struct HasRank<N, AxesOrder<>>: ConstTrue {};

    template<typename Axes>
    struct IsDistinct;

    template<size_t I, size_t... Axes>
    struct IsDistinct<AxesOrder<I, Axes...>>:
        ConstBool<
            !Contains<I, AxesOrder<Axes...>>::value
            && IsDistinct<AxesOrder<Axes...>>::value> {};

    template<>
    struct IsDistinct<AxesOrder<>>: ConstTrue {};

    template<typename Axes>
    struct IsPermutation;

    template<size_t... Axes>
    struct IsPermutation<AxesOrder<Axes...>>:
        ConstBool<
            HasRank<sizeof...(Axes), AxesOrder<Axes...>>::value
            && IsDistinct<AxesOrder<Axes...>>::value> {};

    template<typename Axes, typename = AxesOrder<>>
    struct Reverse;

    template<size_t... Outputs>
    struct Reverse<AxesOrder<>, AxesOrder<Outputs...>> {
        using type = AxesOrder<Outputs...>;
    };

    template<size_t First, size_t... Inputs, size_t... Outputs>
    struct Reverse<AxesOrder<First, Inputs...>, AxesOrder<Outputs...>> {
        using type = typename Reverse<
            AxesOrder<Inputs...>,
            AxesOrder<First, Outputs...>>::type;
    };

    template<size_t Axis, typename Axes, typename = AxesOrder<>>
    struct Remove;

    template<size_t Axis, size_t... Tail, size_t... Head>
    struct Remove<Axis, AxesOrder<Axis, Tail...>, AxesOrder<Head...>> {
        using type = AxesOrder<Head..., Tail...>;
    };

    template<size_t Axis, size_t First, size_t... Tail, size_t... Head>
    struct Remove<Axis, AxesOrder<First, Tail...>, AxesOrder<Head...>> {
        using type = typename Remove<
            Axis,
            AxesOrder<Tail...>,
            AxesOrder<Head..., First>>::type;
    };

    template<size_t I, typename Axes>
    struct GetAt;

    template<size_t First, size_t... Tail>
    struct GetAt<0, AxesOrder<First, Tail...>>: ConstSize<First> {};

    template<size_t I, size_t First, size_t... Tail>
    struct GetAt<I, AxesOrder<First, Tail...>>:
        ConstSize<GetAt<I - 1, AxesOrder<Tail...>>::value> {};

    template<size_t I, typename Axes, typename = AxesOrder<>>
    struct RemoveAt;

    template<size_t First, size_t... Tail, size_t... Head>
    struct RemoveAt<0, AxesOrder<First, Tail...>, AxesOrder<Head...>> {
        using type = AxesOrder<Head..., Tail...>;
    };

    template<size_t I, size_t First, size_t... Tail, size_t... Head>
    struct RemoveAt<I, AxesOrder<First, Tail...>, AxesOrder<Head...>> {
        using type = typename RemoveAt<
            I - 1,
            AxesOrder<Tail...>,
            AxesOrder<Head..., First>>::type;
    };

    template<size_t I, size_t Axis, typename Axes, typename = AxesOrder<>>
    struct InsertAt;

    template<size_t Axis, size_t First, size_t... Tail, size_t... Head>
    struct InsertAt<0, Axis, AxesOrder<First, Tail...>, AxesOrder<Head...>> {
        using type = AxesOrder<Head..., Axis, First, Tail...>;
    };

    template<size_t Axis, size_t... Head>
    struct InsertAt<0, Axis, AxesOrder<>, AxesOrder<Head...>> {
        using type = AxesOrder<Head..., Axis>;
    };

    template<
        size_t I,
        size_t Axis,
        size_t First,
        size_t... Tail,
        size_t... Head>
    struct InsertAt<I, Axis, AxesOrder<First, Tail...>, AxesOrder<Head...>> {
        using type = typename InsertAt<
            I - 1,
            Axis,
            AxesOrder<Tail...>,
            AxesOrder<Head..., First>>::type;
    };

    template<typename Axes, typename Remove>
    struct Difference;

    template<typename Axes, size_t First, size_t... Rest>
    struct Difference<Axes, AxesOrder<First, Rest...>> {
        using type = typename Difference<
            typename Remove<First, Axes>::type,
            AxesOrder<Rest...>>::type;
    };

    template<typename Axes>
    struct Difference<Axes, AxesOrder<>> {
        using type = Axes;
    };

}  // namespace axes_helper

namespace axes {
    template<size_t N>
    using seq = std::make_index_sequence<N>;

    template<typename Left, typename Right>
    using join = typename axes_helper::Join<Left, Right>::type;

    template<typename Axes, size_t... AxesToAdd>
    using prepend = join<AxesOrder<AxesToAdd...>, Axes>;

    template<typename Axes, size_t... AxesToAdd>
    using append = join<Axes, AxesOrder<AxesToAdd...>>;

    template<typename Axes, typename Remove>
    using difference = typename axes_helper::Difference<Axes, Remove>::type;

    template<typename Axes, size_t... AxesToRemove>
    using remove = difference<Axes, AxesOrder<AxesToRemove...>>;

    template<typename Axes>
    using reverse = typename axes_helper::Reverse<Axes>::type;

    template<typename Axes, size_t N>
    constexpr static size_t has_rank = axes_helper::HasRank<N, Axes>::value;

    template<typename Axes, size_t I>
    constexpr static size_t at = axes_helper::GetAt<I, Axes>::value;

    template<typename Axes, size_t I, size_t Axis>
    using insert_at = typename axes_helper::InsertAt<I, Axis, Axes>::type;

    template<typename Axes, size_t I>
    using remove_at = typename axes_helper::RemoveAt<I, Axes>::type;

    template<typename Axes>
    constexpr static bool is_distinct = axes_helper::IsDistinct<Axes>::value;

    template<typename Axes>
    constexpr static bool is_permutation =
        axes_helper::IsPermutation<Axes>::value;
}  // namespace axes

template<size_t... Is>
AxesOrder<Is...> into_axes(const ConstInt<size_t, Is>&...) {
    return {};
}

template<size_t... Is>
AxesOrder<Is...> into_axes(const AxesOrder<Is...>&) {
    return {};
}

}  // namespace capibara