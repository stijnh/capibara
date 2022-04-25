#pragma once

#include "types.h"

namespace capibara {

template<typename T, T Value>
struct ConstInt {
    operator T() const {
        return Value;
    }

    T operator()() const {
        return Value;
    }
};

template<typename T, T Left, T Right>
ConstInt<T, Left + Right> operator+(ConstInt<T, Left>, ConstInt<T, Right>) {
    return {};
}

template<typename T, T Left, T Right>
ConstInt<T, Left * Right> operator*(ConstInt<T, Left>, ConstInt<T, Right>) {
    return {};
}

template<typename T, T Left, T Right>
ConstInt<T, Left / Right> operator/(ConstInt<T, Left>, ConstInt<T, Right>) {
    return {};
}

template<typename T, T Left, T Right>
ConstInt<T, Left % Right> operator%(ConstInt<T, Left>, ConstInt<T, Right>) {
    return {};
}

template<typename T>
CAPIBARA_INLINE constexpr T convert_integer(T value) {
    return value;
}

template<typename R, typename T, T Value>
CAPIBARA_INLINE constexpr ConstInt<R, (T)Value>
convert_integer(ConstInt<T, Value>) {
    return {};
}

template<typename T, T N>
static constexpr ConstInt<T, N> const_int = {};

CAPIBARA_INLINE constexpr size_t convert_size(size_t value) {
    return value;
}

template<typename T, T Value>
CAPIBARA_INLINE constexpr ConstInt<size_t, (size_t)Value>
convert_size(ConstInt<T, Value>) {
    return {};
}

template<size_t N>
using ConstSize = ConstInt<size_t, N>;

template<size_t N>
static constexpr ConstSize<N> const_size = {};
static constexpr ConstSize<0> Zero = {};
static constexpr ConstSize<1> One = {};
static constexpr ConstSize<2> Two = {};

struct DynInt {};

template<typename T, typename... Items>
struct Tuple;

namespace tuple_helpers {
    template<typename T, size_t I, typename Axis, typename... Items>
    struct Get {};

    template<typename T, typename... Items>
    struct Get<T, 0, DynAxis, Items...> {
        using return_type = T;

        template<size_t N>
        static return_type call(DynAxis axis, const std::array<T, N>& storage) {
            return storage[(size_t)axis];
        }
    };

    template<typename T, size_t I, typename... Items>
    struct Get<T, I, Axis<I>, DynInt, Items...> {
        using return_type = T;

        template<size_t N>
        static return_type call(Axis<I>, const std::array<T, N>& storage) {
            static_assert(I < N, "internal error");
            return storage[I];
        }
    };

    template<typename T, size_t I, T StaticValue, typename... Items>
    struct Get<T, I, Axis<I>, ConstInt<T, StaticValue>, Items...> {
        using return_type = ConstInt<T, StaticValue>;

        template<size_t N>
        static return_type call(Axis<I>, const std::array<T, N>&) {
            return {};
        }
    };

    template<
        typename T,
        size_t I,
        size_t NotI,
        typename FirstItem,
        typename... Items>
    struct Get<T, I, Axis<NotI>, FirstItem, Items...> {
        using return_type =
            typename Get<T, I + 1, Axis<NotI>, Items...>::return_type;

        template<size_t N>
        static return_type
        call(Axis<NotI> axis, const std::array<T, N>& storage) {
            return Get<T, I + 1, Axis<NotI>, Items...>::call(axis, storage);
        }
    };

    template<typename T, size_t I, typename Axis, typename... Items>
    struct Check {};

    template<typename T, size_t I, typename... Items>
    struct Check<T, I, Axis<I>, DynInt, Items...> {
        static void call(Axis<I>, const T&) {
            // ok!
        }
    };

    template<typename T, size_t I, T StaticValue, typename... Items>
    struct Check<T, I, Axis<I>, ConstInt<T, StaticValue>, Items...> {
        static void call(Axis<I>, const T& arg) {
            if (arg != StaticValue) {
                throw std::runtime_error(
                    "dynamic value does not match constant value");
            }
            printf("B %d %d == %d\n", (int)I, (int)arg, (int)StaticValue);
        }

        template<T ArgValue>
        static void call(Axis<I>, ConstInt<T, ArgValue>) {
            static_assert(
                ArgValue == StaticValue,
                "cannot assign invalid value to constant");
            printf("A %d %d\n", (int)I, (int)ArgValue);
        }
    };

    template<
        typename T,
        size_t I,
        size_t NotI,
        typename FirstItem,
        typename... Items>
    struct Check<T, I, Axis<NotI>, FirstItem, Items...> {
        template<typename Arg>
        static void call(Axis<NotI> axis, Arg arg) {
            Check<T, I + 1, Axis<NotI>, Items...>::call(axis, arg);
            printf("recur %d %d\n", (int)I, (int)NotI);
        }
    };

    template<typename T, typename Indices, typename... Items>
    struct Assign {};

    template<typename T, typename... Items>
    struct Assign<T, std::index_sequence<>, Items...> {
        template<size_t N, typename F>
        static void call(std::array<T, N>&, F) {}
    };

    template<typename T, size_t Index, size_t... Rest, typename... Items>
    struct Assign<T, std::index_sequence<Index, Rest...>, Items...> {
        template<size_t N, typename F>
        static void call(std::array<T, N>& storage, F fun) {
            static_assert(Index < N, "internal error");
            auto value = fun(Axis<Index> {});

            printf("Check axis %d\n", (int)Index);
            Check<T, 0, Axis<Index>, Items...>::call(Axis<Index> {}, value);
            storage[Index] = value;

            Assign<T, std::index_sequence<Rest...>, Items...>::call(storage, fun);
        }
    };

    template<typename Tuple, typename Indices>
    struct Apply {};

    template<typename Tuple, size_t Index, size_t... Rest>
    struct Apply<Tuple, std::index_sequence<Index, Rest...>> {
        template<typename F>
        static void call(F fun, const Tuple& tuple) {
            fun(Axis<Index> {}, tuple[Axis<Index> {}]);
            Apply<Tuple, std::index_sequence<Rest...>>::call(fun, tuple);
        }
    };

    template<typename Tuple>
    struct Apply<Tuple, std::index_sequence<>> {
        template<typename F>
        static void call(F fun, const Tuple& tuple) {
            //
        }
    };

    template<typename Tuple, typename Indices>
    struct Fold {};

    template<typename Tuple, size_t Index, size_t... Rest>
    struct Fold<Tuple, std::index_sequence<Index, Rest...>> {
        template<typename F, typename R>
        static auto call(F fun, R initial, const Tuple& tuple) {
            auto result = fun(Axis<Index> {}, initial, tuple[Axis<Index> {}]);
            return Fold<Tuple, std::index_sequence<Rest...>>::call(
                std::move(fun),
                std::move(result),
                tuple);
        }
    };

    template<typename Tuple>
    struct Fold<Tuple, std::index_sequence<>> {
        template<typename F, typename R>
        static R call(F fun, R initial, const Tuple& tuple) {
            return initial;
        }
    };

    template<typename Tuple, typename Indices>
    struct Map {};

    template<typename Tuple, size_t Index, size_t... Rest>
    struct Map<Tuple, std::index_sequence<Index, Rest...>> {
        template<typename F, typename... Results>
        static auto call(F fun, const Tuple& tuple, Results... results) {
            auto last = fun(Axis<Index> {}, tuple[Axis<Index> {}]);
            return Map<Tuple, std::index_sequence<Rest...>>::call(
                fun,
                tuple,
                results...,
                last);
        }
    };

    template<typename Tuple>
    struct Map<Tuple, std::index_sequence<>> {
        template<typename F, typename... Results>
        static auto call(F fun, const Tuple& tuple, Results... results) {
            return std::make_tuple(std::forward<Results>(results)...);
        }
    };

    template<typename T, size_t N, typename... Items>
    struct RepeatDyn {
        using type = typename RepeatDyn<T, N - 1, DynInt, Items...>::type;
    };

    template<typename T, typename... Items>
    struct RepeatDyn<T, 0, Items...> {
        using type = Tuple<T, Items...>;
    };

    template<typename Tuple, typename... Values>
    struct ConvertHelper {};

    template<typename Tuple>
    struct ConvertHelper<Tuple> {
        using type = Tuple;
    };

    template<typename T, typename... Items, T StaticValue, typename... Values>
    struct ConvertHelper<
        Tuple<T, Items...>,
        ConstInt<T, StaticValue>,
        Values...> {
        using type = typename ConvertHelper<
            Tuple<T, Items..., ConstInt<T, StaticValue>>,
            Values...>::type;
    };

    template<typename T, typename... Items, typename  FirstValue, typename... Values>
    struct ConvertHelper<Tuple<T, Items...>, FirstValue, Values...> {
        using type =
            typename ConvertHelper<Tuple<T, Items..., DynInt>, Values...>::type;
    };

    template<typename T, typename... Values>
    struct Convert {
        using type = typename ConvertHelper<
            Tuple<T>,
            typename std::decay<Values>::type...>::type;
    };

    template<typename T, typename... Values>
    struct Convert<T, std::tuple<Values...>> {
        using type = typename ConvertHelper<
            Tuple<T>,
            typename std::decay<Values>::type...>::type;
    };

    template<typename T, typename R, typename... Values>
    struct Convert<T, Tuple<R, Values...>> {
        using type = typename ConvertHelper<
            Tuple<T>,
            typename std::decay<Values>::type...>::type;
    };

    template<typename T, size_t N>
    struct Convert<T, std::array<T, N>> {
        using type = typename RepeatDyn<T, N>::type;
    };

}  // namespace tuple_helpers

template<typename T, typename... Items>
struct Tuple {
    using self_type = Tuple<T, Items...>;
    static constexpr size_t rank = sizeof...(Items);

    Tuple(const Tuple& other) = default;
    Tuple(Tuple&& other)  noexcept = default;

    Tuple(const std::array<T, rank>& list) {
        assign([&list](auto axis) { return list[axis]; });
    }

    Tuple(T* begin, T* end) {
        if (std::distance(begin, end) != rank) {
            throw std::runtime_error("invalid number of items");
        }

        assign([&begin](auto axis) { return begin[axis]; });
    }

    Tuple(std::initializer_list<T> list) : Tuple(list.begin(), list.end()) {
        //
    }

    template<typename... Values>
    Tuple(const std::tuple<Values...>& tuple) {
        static_assert(sizeof...(Values) == rank, "invalid number of items");

        assign([&tuple](auto axis) { return std::get<(size_t)axis>(tuple); });
    }

    template<typename... Values>
    Tuple(Values... values) :
        Tuple(std::make_tuple<Values...>(std::forward<Values>(values)...)) {}

        template<typename... OtherItems>
        Tuple(const Tuple<T, OtherItems...>& other) {
        static_assert(sizeof...(OtherItems) == rank, "invalid number of items");

        assign([&other](auto axis) { return other[axis]; });
    }

    Tuple& operator=(const Tuple&) = default;
    Tuple& operator=(Tuple&&)  noexcept = default;

    Tuple& operator=(const std::array<T, rank>& rhs) {
        return (*this = Tuple(rhs));
    }

    template<typename... Values>
    Tuple& operator=(const std::tuple<Values...>& rhs) {
        return (*this = Tuple(rhs));
    }

    template<typename... OtherItems>
    Tuple& operator=(const Tuple<T, OtherItems...>& rhs) {
        return (*this = Tuple(rhs));
    }

    template<typename F, typename Indices = std::index_sequence_for<Items...>>
    void assign(F fun, Indices = {}) {
        tuple_helpers::Assign<T, Indices, Items...>::call(storage_, fun);
    }

    template<typename F, typename Indices = std::index_sequence_for<Items...>>
    void apply(F fun, Indices = {}) const {
        tuple_helpers::Apply<self_type, Indices>::call(fun, *this);
    }

    template<
        typename F,
        typename R,
        typename Indices = std::index_sequence_for<Items...>>
    auto fold(F fun, R initial, Indices = {}) const {
        return tuple_helpers::Fold<self_type, Indices>::call(
            fun,
            initial,
            *this);
    }

    template <typename Indices = std::index_sequence_for<Items...>>
    auto sum(Indices indices = {}) const {
        return fold([](auto axis, auto lhs, auto rhs) { return lhs + rhs; }, ConstInt<T, 0>{}, indices);
    }

    template <typename Indices = std::index_sequence_for<Items...>>
    auto product(Indices indices = {}) const {
        return fold([](auto axis, auto lhs, auto rhs) { return lhs * rhs; }, ConstInt<T, 1>{}, indices);
    }

    template<
        typename R = T,
        typename F,
        typename Indices = std::index_sequence_for<Items...>>
    auto map(F fun, Indices = {}) const {
        return tuple_helpers::Map<self_type, Indices>::call(fun, *this);
    }

    template<typename Axis>
    auto operator[](Axis axis) const {
        auto axis_ = into_axis(axis);
        return tuple_helpers::Get<T, 0, decltype(axis_), Items...>::call(
            axis_,
            storage_);
    }

    static constexpr size_t size() noexcept {
        return rank;
    }

    std::array<T, rank> to_array() const {
        return storage_;
    }

  private:
    std::array<T, rank> storage_;
};

template<typename T, size_t N>
using TupleN = typename tuple_helpers::RepeatDyn<T, N>::type;

template<typename T, typename... Values>
auto convert_tuple(Values... values) {
    using TupleType = typename tuple_helpers::Convert<T, Values...>::type;
    return TupleType(values...);
}

template<typename... Ts>
auto dims(Ts... values) {
    return convert_tuple<size_t, Ts...>(values...);
}

}  // namespace capibara
