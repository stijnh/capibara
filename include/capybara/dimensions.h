#pragma once

#include <iostream>

#include "forwards.h"
#include "functional.h"
#include "tuple.h"

namespace capybara {
static constexpr index_t Dyn = -1;

template<index_t... sizes>
struct Dimensions;

namespace detail {
    template<typename T, typename = void>
    struct FromDimensionType {};

    template<typename T>
    struct FromDimensionType<const T>: FromDimensionType<T> {};

    template<typename T>
    struct FromDimensionType<T&>: FromDimensionType<T> {};

    template<typename T>
    struct FromDimensionType<T&&>: FromDimensionType<T> {};

    template<typename T, index_t i>
    struct FromDimensionType<ConstInt<T, i>, enable_t<((index_t)i >= 0)>> {
        static constexpr index_t value = i;
    };

    template<typename T, index_t i>
    struct FromDimensionType<
        std::integral_constant<T, i>,
        enable_t<((index_t)i >= 0)>> {
        static constexpr index_t value = i;
    };

    template<typename T>
    struct FromDimensionType<T, enable_t<std::is_integral<T>::value>> {
        static constexpr index_t value = Dyn;
    };

    template<size_t N, typename Indices = std::make_index_sequence<N>>
    struct RepeatDyn;

    template<size_t n, size_t... indices>
    struct RepeatDyn<n, std::index_sequence<indices...>> {
        using type = Dimensions<(Dyn + 0 * (index_t)indices)...>;
    };
}  // namespace detail

template<index_t size>
using DimensionType =
    typename std::conditional<(size < 0), index_t, ConstIndex<size>>::type;

template<typename T>
static constexpr index_t FromDimensionType =
    detail::FromDimensionType<T>::value;

template<size_t N>
using DimensionsN = typename detail::RepeatDyn<N>::type;
using Dimensions0 = DimensionsN<0>;
using Dimensions1 = DimensionsN<1>;
using Dimensions2 = DimensionsN<2>;
using Dimensions3 = DimensionsN<3>;
using Dimensions4 = DimensionsN<4>;
using Dimensions5 = DimensionsN<5>;
using Dimensions6 = DimensionsN<6>;

template<index_t... sizes>
struct Dimensions {
    template<index_t... other_sizes>
    friend struct Dimensions;

    using Self = Dimensions<sizes...>;
    using Tuple = ::capybara::Tuple<DimensionType<sizes>...>;

    static constexpr index_t rank = sizeof...(sizes);

    Dimensions() = default;
    Dimensions(const Dimensions&) = default;

    template<typename... Ts>
    Dimensions(const ::capybara::Tuple<Ts...>& other) : inner_(other) {}

    template<index_t... other_sizes>
    Dimensions(const Dimensions<other_sizes...>& other) :
        inner_(other.inner_) {}

    Dimensions(const index_t* begin, const index_t* end) : inner_(begin, end) {}

    template<size_t length>
    Dimensions(const std::array<index_t, length>& other) :
        Dimensions(other.begin(), other.end()) {}

    Dimensions(std::initializer_list<index_t> items) :
        Dimensions(items.begin(), items.end()) {}

    index_t get(DynIndex<rank> index) const {
        return inner_.template get<index_t>(index);
    }

    void set(DynIndex<rank> index, index_t value) {
        inner_.template set<index_t>(index, value);
    }

    template<index_t i>
    TupleElementAt<i, DimensionType<sizes>...>
    get(ConstIndex<i> index = {}) const {
        return inner_.template get_ref<i>();
    }

    template<index_t i>
    void set(ConstIndex<i> index, index_t value) {
        inner_.template get_ref<i>() = value;
    }

    index_t operator[](DynIndex<rank> index) const {
        return get(index);
    }

    template<index_t i>
    TupleElementAt<i, DimensionType<sizes>...>
    operator[](ConstIndex<i> index) const {
        return get(index);
    }

    template<index_t... other_sizes>
    bool operator==(const Dimensions<other_sizes...>& rhs) const {
        return inner_ == rhs.inner_;
    }

    template<index_t... other_sizes>
    bool operator!=(const Dimensions<other_sizes...>& rhs) const {
        return !(*this == rhs);
    }

    static constexpr size_t size() {
        return sizeof...(sizes);
    }

    constexpr std::array<index_t, rank> to_array() const {
        return inner_.to_array();
    }

    constexpr Tuple to_tuple() const {
        return inner_;
    }

    friend std::ostream& operator<<(std::ostream& out, const Self& self) {
        return out << self.inner_;
    }

  private:
    Tuple inner_ {};
};

template<size_t n>
DimensionsN<n> into_dims(const std::array<index_t, n>& dims) {
    return dims;
}

template<index_t... sizes>
Dimensions<sizes...> into_dims(const Dimensions<sizes...>& dims) {
    return dims;
}

template<typename... Ts>
Dimensions<detail::FromDimensionType<Ts>::value...> into_dims(Ts&&... dims) {
    return {std::forward<Ts>(dims)...};
}

template<typename... Ts>
Dimensions<FromDimensionType<Ts>...> into_dims(const Tuple<Ts...>& dims) {
    return dims;
}

template<typename... Ts>
using IntoDims = decltype(into_dims(std::declval<Ts>()...));

template<
    typename E,
    typename indices = std::make_index_sequence<ExprTraits<E>::rank>>
struct ExprToDims;

template<typename E, size_t... indices>
struct ExprToDims<E, std::index_sequence<indices...>> {
    using type = Dimensions<FromDimensionType<ExprDim<E, indices>>...>;

    static type call(const E& expr) {
        return {expr.dim(ConstIndex<indices> {})...};
    }
};

template<typename E>
typename ExprToDims<E>::type into_dims(const Expr<E>& dims) {
    return ExprToDims<E>::call(dims.self());
}

}  // namespace capybara

namespace std {
// These make structured binding possible
template<capybara::index_t... sizes>
struct tuple_size<capybara::Dimensions<sizes...>>:
    tuple_size<typename capybara::Dimensions<sizes...>::Tuple> {};

template<size_t i, capybara::index_t... sizes>
struct tuple_element<i, capybara::Dimensions<sizes...>>:
    tuple_element<i, typename capybara::Dimensions<sizes...>::Tuple> {};

template<size_t i, capybara::index_t... sizes>
tuple_element<i, capybara::Dimensions<sizes...>>&
get(capybara::Dimensions<sizes...>& t) {
    return t.template get<i>();
}

template<size_t i, capybara::index_t... sizes>
const tuple_element<i, capybara::Dimensions<sizes...>>&
get(const capybara::Dimensions<sizes...>& t) {
    return t.template get<i>();
}
}  // namespace std