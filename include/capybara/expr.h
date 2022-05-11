#pragma once

#include <array>
#include <type_traits>
#include <utility>

#include "const_int.h"
#include "dimensions.h"
#include "forwards.h"
#include "types.h"

namespace capybara {

namespace detail {
    template<typename E, typename Axis, typename = void>
    struct DimHelper {
        using type = index_t;

        CAPYBARA_INLINE
        static type call(const E& expr, Axis axis) {
            return expr.dim_impl(axis);
        }
    };

    template<typename E, index_t axis>
    struct DimHelper<
        E,
        ConstIndex<axis>,
        enable_t<ExprConstDim<E, axis>::value >= 0>> {
        using type = ConstIndex<ExprConstDim<E, axis>::value>;

        CAPYBARA_INLINE
        static type call(const E& expr, ConstIndex<axis>) {
            return {};
        }
    };

    template<
        typename E,
        typename = std::make_index_sequence<ExprTraits<E>::rank>>
    struct IterateHelper;

    template<typename E, size_t... axes>
    struct IterateHelper<E, std::index_sequence<axes...>> {
        static constexpr size_t rank = sizeof...(axes);
        using Shape = std::array<index_t, rank>;

        CAPYBARA_INLINE
        static Shape shape(const E& expr) {
            return {expr.dim(ConstIndex<axes> {})...};
        }

        using Dims = Dimensions<FromDimensionType<ExprDim<E, axes>>::value...>;

        CAPYBARA_INLINE
        static Dims dims(const E& expr) {
            return {expr.dim(const_index<axes>)...};
        }

        CAPYBARA_INLINE
        static bool empty(const E& expr) {
            std::array<bool, rank> flags {
                (expr.dim(const_index<axes>) <= 0)...};
            bool result = false;

            for (size_t i = 0; i < rank; i++) {
                result |= flags[i];
            }

            return result;
        }

        CAPYBARA_INLINE
        static index_t size(const E& expr) {
            Shape shape = {expr.dim(const_index<axes>)...};
            index_t result = 1;

            for (size_t i = 0; i < rank; i++) {
                result *= shape[i];
            }

            return result;
        }
    };
}  // namespace detail

template<typename E>
using ExprDims = typename detail::IterateHelper<E>::Dims;

template<typename E>
using ExprValue = typename ExprTraits<E>::Value;

template<typename Derived>
struct Expr {
    using Self = Derived;
    static constexpr size_t rank = ExprTraits<Self>::rank;
    static constexpr bool is_view = ExprTraits<Self>::is_view;
    static constexpr bool is_readable = ExprTraits<Self>::is_readable;
    static constexpr bool is_writeable = ExprTraits<Self>::is_writeable;
    using NestedMut = typename ExprNested<Self>::type;
    using Nested = typename ExprNested<const Self>::type;
    using CursorMut = typename ExprTraits<NestedMut>::Cursor;
    using Cursor = typename ExprTraits<Nested>::Cursor;
    using Value = ExprValue<Self>;
    using Dims = typename detail::IterateHelper<Self>::Dims;
    using Shape = std::array<index_t, rank>;

    Derived& self() {
        return *(Derived*)this;
    }

    const Derived& self() const {
        return *(const Derived*)this;
    }

    NestedMut nested() & {
        return NestedMut(self());
    }

    NestedMut nested() && {
        return NestedMut(self());
    }

    Nested nested() const& {
        return Nested(self());
    }

    CursorMut cursor() {
        return CursorMut(nested());
    }

    Cursor cursor() const {
        return Cursor(nested());
    }

    template<typename Axis>
    index_t dim(Axis axis) const {
        auto axis_ = into_index<rank>(axis);
        return detail::DimHelper<Derived, decltype(axis_)>::call(self(), axis_);
    }

    Dims dims() const {
        return detail::IterateHelper<Self>::dims(self());
    }

    bool empty() const {
        return detail::IterateHelper<Self>::empty(self());
    }

    index_t size() const {
        return detail::IterateHelper<Self>::size(self());
    }

    Shape shape() const {
        return detail::IterateHelper<Self>::shape(self());
    }
};
}  // namespace capybara