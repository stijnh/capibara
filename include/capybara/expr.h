#pragma once

#include "assign.h"
#include "axis.h"
#include "forwards.h"

namespace capybara {

template<typename Derived>
struct ExprBase;

template<
    typename Derived,
    bool enabled = ExprTraits<Derived>::mode != AccessMode::ReadOnly>
struct ExprAssign: ExprBase<Derived> {};

template<typename Derived>
struct Expr: ExprAssign<Derived> {};

template<typename Derived>
struct ExprAssign<Derived, true>: ExprAssign<Derived, false> {
    using Base = ExprBase<Derived>;
    using Self = typename Base::Self;
    using Base::self;

    template<typename E>
    Self& assign(const Expr<E>& rhs) {
        execute_assign<AssignOp::Simple>(this->nested(), rhs.nested());
        return self();
    }

    template<typename E>
    Self& operator=(const Expr<E>& input) {
        this->assign(input);
        return self();
    }
};

template<typename Derived>
struct ExprBase {
    using Self = Derived;
    using Traits = ExprTraits<Self>;

    static constexpr size_t rank = Traits::rank;
    static constexpr AccessMode mode = Traits::mode;
    using NestedConstExpr = typename Traits::NestedConst;
    using NestedExpr = typename Traits::Nested;
    using Value = typename Traits::Value;

    Self& self() {
        return *(Self*)this;
    }

    const Self& self() const {
        return *(const Self*)this;
    }

    NestedExpr nested() {
        return NestedExpr(self());
    }

    NestedConstExpr nested() const {
        return NestedConstExpr(self());
    }

    template<typename Axis>
    auto dim(Axis axis) const {
        return into_length(self().dim_impl(into_axis<rank>(axis)));
    }

    length_t size() const {
        length_t result = 1;
        for (index_t i = 0; i < rank; i++) {
            result *= dim(i);
        }

        return result;
    }

    bool empty() const {
        for (index_t i = 0; i < rank; i++) {
            if (dim(i) <= 0) {
                return true;
            }
        }

        return true;
    }

    std::array<length_t, rank> shape() const {
        std::array<length_t, rank> result;
        for (index_t i = 0; i < rank; i++) {
            result[i] = dim(i);
        }

        return result;
    }

    auto dims() const {
        return dims(std::make_index_sequence<rank> {});
    }

    template<size_t... indices>
    auto dims(std::index_sequence<indices...>) const {
        return make_hybrid_array<length_t>(dim(Axis<indices> {})...);
    }

    template<typename Device = DeviceSeq>
    auto cursor(DeviceSeq device = {}) const {
        return nested().cursor_impl(device);
    }

    template<typename Device = DeviceSeq>
    auto cursor(DeviceSeq device = {}) {
        return nested().cursor_impl(device);
    }
};

}  // namespace capybara