#pragma once

#include <array>
#include <iostream>
#include <tuple>
#include <utility>

#include "const_int.h"
#include "types.h"

namespace capybara {

template<typename... Ts>
struct Dimensions;

using DynSize = size_t;
static constexpr size_t Dyn = (size_t)-1;

namespace dimension_helpers {

    template<typename T, size_t I, typename Axis, typename... Values>
    struct Getter {};

    template<typename T, size_t N>
    struct Getter<T, 0, DynAxis<N>> {
        using return_type = T;

        static return_type call(DynAxis<N>, const std::array<T, N>&) {
            throw std::runtime_error("invalid axis");
        }
    };

    template<typename T, T StaticValue, size_t N>
    struct Getter<T, 0, DynAxis<N>, ConstInt<T, StaticValue>> {
        using return_type = ConstInt<T, StaticValue>;

        static return_type call(DynAxis<N>, const std::array<T, N>&) {
            static_assert(N != 1, "internal error");
            return {};
        }
    };

    template<typename T, typename... Values, size_t N>
    struct Getter<T, 0, DynAxis<N>, Values...> {
        using return_type = T;

        static return_type
        call(DynAxis<N> axis, const std::array<T, N>& storage) {
            static_assert(N > 0, "internal error");
            return storage[axis];
        }
    };

    template<typename T, size_t I, typename... Values>
    struct Getter<T, I, Axis<I>, T, Values...> {
        using return_type = T;

        template<size_t N>
        static T call(Axis<I>, const std::array<T, N>& storage) {
            static_assert(I < N, "internal error");
            return storage[I];
        }
    };

    template<typename T, size_t I, T StaticValue, typename... Values>
    struct Getter<T, I, Axis<I>, ConstInt<T, StaticValue>, Values...> {
        using return_type = ConstInt<T, StaticValue>;

        template<size_t N>
        static return_type call(Axis<I>, const std::array<T, N>&) {
            static_assert(I < N, "internal error");
            return {};
        }
    };

    template<
        typename T,
        size_t I,
        size_t NotI,
        typename FirstValue,
        typename... Values>
    struct Getter<T, I, Axis<NotI>, FirstValue, Values...> {
        using return_type =
            typename Getter<T, I + 1, Axis<NotI>, Values...>::return_type;

        template<size_t N>
        static auto call(Axis<NotI> axis, const std::array<T, N>& storage) {
            return Getter<T, I + 1, Axis<NotI>, Values...>::call(axis, storage);
        }
    };

    template<typename T, size_t I, typename Axis, typename... Items>
    struct Check {};

    template<typename T, size_t I, typename... Items>
    struct Check<T, I, Axis<I>, T, Items...> {
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
        }

        template<T ArgValue>
        static void call(Axis<I>, ConstInt<T, ArgValue>) {
            if (ArgValue != StaticValue) {
                // Maybe static_assert?
                throw std::runtime_error(
                    "cannot assign invalid value to constant");
            }
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
        static void call(Axis<NotI> axis, Arg&& arg) {
            Check<T, I + 1, Axis<NotI>, Items...>::call(
                axis,
                std::forward<Arg>(arg));
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

            Check<T, 0, Axis<Index>, Items...>::call(Axis<Index> {}, value);
            storage[Index] = value;

            Assign<T, std::index_sequence<Rest...>, Items...>::call(
                storage,
                fun);
        }
    };

    template<typename T, size_t I, typename... Items>
    struct Init;

    template<typename T, size_t I, typename... Rest>
    struct Init<T, I, T, Rest...> {
        template<size_t N>
        static void call(std::array<T, N>& storage) {
            static_assert(I < N, "internal error");
            storage[I] = T {};
        }
    };

    template<typename T, size_t I, T StaticValue, typename... Rest>
    struct Init<T, I, ConstInt<T, StaticValue>, Rest...> {
        template<size_t N>
        static void call(std::array<T, N>& storage) {
            static_assert(I < N, "internal error");
            storage[I] = StaticValue;
        }
    };

    template<size_t N, typename... Ts>
    struct Repeat {
        using type = typename Repeat<N - 1, DynSize, Ts...>::type;
    };

    template<typename... Ts>
    struct Repeat<0, Ts...> {
        using type = Dimensions<Ts...>;
    };

    template<typename Ns, typename... Results>
    struct Sizes;

    template<typename... Results>
    struct Sizes<std::index_sequence<>, Results...> {
        using type = Dimensions<Results...>;
    };

    template<size_t First, size_t... Rest, typename... Results>
    struct Sizes<std::index_sequence<First, Rest...>, Results...> {
        using type = typename Sizes<
            std::index_sequence<Rest...>,
            Results...,
            ConstSize<First>>::type;
    };

    template<size_t... Rest, typename... Results>
    struct Sizes<std::index_sequence<Dyn, Rest...>, Results...> {
        using type =
            typename Sizes<std::index_sequence<Rest...>, Results..., DynSize>::
                type;
    };

    template<typename Result, typename... Ts>
    struct ConvertInner;

    template<typename Result>
    struct ConvertInner<Result> {
        using type = Result;
    };

    template<typename... Results, size_t N, typename... Ts>
    struct ConvertInner<Dimensions<Results...>, ConstSize<N>, Ts...> {
        using type =
            typename ConvertInner<Dimensions<Results..., ConstSize<N>>, Ts...>::
                type;
    };

    template<typename... Results, typename First, typename... Ts>
    struct ConvertInner<Dimensions<Results...>, First, Ts...> {
        using type =
            typename ConvertInner<Dimensions<Results..., DynSize>, Ts...>::type;
    };

    template<typename... Ts>
    struct Convert {
        using type = typename ConvertInner<
            Dimensions<>,
            typename std::decay<Ts>::type...>::type;
    };

    template<typename... Ts>
    struct Convert<std::tuple<Ts...>> {
        using type = typename Convert<Ts...>::type;
    };

    template<typename... Ts>
    struct Convert<Dimensions<Ts...>> {
        using type = typename Convert<Ts...>::type;
    };

    template<typename T, size_t N>
    struct Convert<std::array<T, N>> {
        using type = typename Repeat<N>::type;
    };

}  // namespace dimension_helpers

template<typename... Ts>
struct Dimensions {
    static constexpr size_t rank = sizeof...(Ts);
    using Value = size_t;

    Dimensions(const Dimensions&) = default;
    Dimensions(Dimensions&&) noexcept = default;
    Dimensions(Dimensions& that) {
        *this = that;
    }

    Dimensions() {
        dimension_helpers::Init<Value, 0, Ts...>::call(storage_);
    }

    Dimensions(const Value* begin, const Value* end) {
        if (std::distance(begin, end) != rank) {
            throw std::runtime_error("invalid number of values given");
        }

        assign([&begin](auto axis) { return begin[(size_t)axis]; });
    }

    template<typename... OtherValues>
    Dimensions(const std::tuple<OtherValues...>& values) {
        *this = values;
    }

    template<typename... OtherValues>
    Dimensions(std::tuple<OtherValues...>&& values) {
        *this = values;
    }

    Dimensions(const std::initializer_list<Value>& values) :
        Dimensions(values.begin(), values.end()) {}

    Dimensions(std::array<Value, rank> values) {
        *this = values;
    }

    template<typename... OtherValues>
    Dimensions(OtherValues&&... values) {
        *this = std::make_tuple<OtherValues...>(
            std::forward<OtherValues>(values)...);
    }

    template<typename F, typename Indices = std::index_sequence_for<Ts...>>
    void assign(F fun, Indices = {}) {
        dimension_helpers::Assign<Value, Indices, Ts...>::call(
            storage_,
            fun);
    }

    Dimensions& operator=(const Dimensions&) = default;
    Dimensions& operator=(Dimensions&&) noexcept = default;

    template<typename... OtherValues>
    Dimensions& operator=(const std::tuple<OtherValues...>& values) {
        static_assert(
            sizeof...(Ts) == sizeof...(OtherValues),
            "invalid number of values");

        assign([&values](auto axis) { return std::get<(size_t)axis>(values); });
        return *this;
    }

    template<typename... OtherValues>
    Dimensions& operator=(std::tuple<OtherValues...>&& values) {
        return (*this = values);
    }

    template<typename... OtherValues>
    Dimensions& operator=(const Dimensions<OtherValues...>& values) {
        static_assert(
            sizeof...(Ts) == sizeof...(OtherValues),
            "invalid number of values");

        assign([&values](auto axis) { return values[axis]; });
        return *this;
    }

    Dimensions& operator=(const std::array<Value, rank>& values) {
        assign([&values](auto axis) { return values[axis]; });
        return *this;
    }

    template<typename Axis>
    auto operator[](Axis axis) const {
        auto axis_ = into_axis<rank>(axis);
        return dimension_helpers::
            Getter<Value, 0, decltype(axis_), Ts...>::call(
                axis_,
                storage_);
    }

    static constexpr size_t size() noexcept {
        return rank;
    }

    template<typename... OtherTypes>
    bool operator==(const Dimensions<OtherTypes...>& dims) const {
        if (size() != dims.size()) {
            return false;
        }

        // TODO: use constants
        for (size_t i = 0; i < size(); i++) {
            if ((*this)[i] != dims[i]) {
                return false;
            }
        }

        return true;
    }

    template<typename... OtherTypes>
    bool operator!=(const Dimensions<OtherTypes...>& dims) const {
        return !(*this == dims);
    }

    std::array<Value, rank> to_array() noexcept {
        return storage_;
    }

  private:
    std::array<Value, rank> storage_;
};

template<>
struct Dimensions<> {
    static constexpr size_t rank = 0;
    using Value = size_t;

    Dimensions() {}
    Dimensions(const Dimensions&) = default;
    Dimensions(Dimensions&&) = default;
    Dimensions(const std::tuple<>&) {}
    Dimensions(std::tuple<>&&) {}
    Dimensions(std::array<Value, 0>) {}

    Dimensions(const Value* begin, const Value* end) {
        if (std::distance(begin, end) != 0) {
            throw std::runtime_error("invalid number of values given");
        }
    }

    Dimensions(const std::initializer_list<Value>& values) :
        Dimensions(values.begin(), values.end()) {}

    Dimensions& operator=(const Dimensions&) = default;
    Dimensions& operator=(Dimensions&&) noexcept = default;

    Dimensions& operator=(const std::tuple<>&) {
        return *this;
    }

    Dimensions& operator=(const std::array<Value, 0>&) {
        return *this;
    }

    template<typename Axis>
    auto operator[](Axis axis) const {
        throw std::runtime_error("index out of bounds");
    }

    static constexpr size_t size() noexcept {
        return 0;
    }

    template<typename... OtherTypes>
    bool operator==(const Dimensions<OtherTypes...>& dims) const {
        return dims.size() == 0;
    }

    template<typename... OtherTypes>
    bool operator!=(const Dimensions<OtherTypes...>& dims) const {
        return !(*this == dims);
    }

    std::array<Value, rank> to_array() noexcept {
        return {};
    }
};

template<size_t N>
using DimensionsN = typename dimension_helpers::Repeat<N>::type;

template<size_t... Sizes>
using DimensionsDyn =
    typename dimension_helpers::Sizes<std::index_sequence<Sizes...>>::type;

template<typename... Dim>
using dims_type = typename dimension_helpers::Convert<
    typename std::decay<Dim>::type...>::type;

template<typename... Dim>
dims_type<Dim...> dims(Dim... values) {
    return dims_type<Dim...>(values...);
}

template<typename... Ts>
std::ostream& operator<<(std::ostream& os, const Dimensions<Ts...>& dims) {
    os << "(";

    for (size_t i = 0; i < dims.size(); i++) {
        if (i != 0)
            os << ", ";
        os << dims[i];
    }

    os << ")";
    return os;
}

}  // namespace capybara
