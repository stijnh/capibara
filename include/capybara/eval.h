#pragma once

#include <type_traits>
#include <utility>

#include "axes.h"
#include "axis.h"
#include "const_int.h"
#include "forwards.h"
#include "util.h"

namespace capybara {

enum struct ControlFlow {
    Continue,
    Break,
};

template<typename Cursor, typename Dims, typename Axes>
struct Eval;

template<typename Cursor, typename Dims>
struct Eval<Cursor, Dims, AxesOrder<>> {
    CAPYBARA_INLINE
    static ControlFlow call(Cursor& cursor, const Dims&) {
        return cursor.evaluate();
    }
};

template<typename Cursor, typename Dims, index_t I, index_t... rest>
struct Eval<Cursor, Dims, AxesOrder<I, rest...>> {
    CAPYBARA_INLINE
    static ControlFlow call(Cursor& cursor, const Dims& dims) {
        Axis<I> axis;
        auto n = into_length(dims[axis]);

        for (index_t i = 0; i < n; i++) {
            ControlFlow result =
                Eval<Cursor, Dims, AxesOrder<rest...>>::call(cursor, dims);

            if (result == ControlFlow::Break) {
                return ControlFlow::Break;
            }

            cursor.advance(axis, const_stride<1>);
        }

        cursor.advance(axis, into_stride(n) * const_stride<-1>);
        return ControlFlow::Continue;
    }
};

template<typename Cursor, typename Dims, typename Axes = axes::seq<Dims::rank>>
void evaluate(Cursor& cursor, const Dims& dims, Axes = {}) {
    static_assert(axes::is_permutation<Axes>, "Axes must be a permutation");

    Eval<Cursor, Dims, Axes>::call(cursor, dims);
}

}  // namespace capybara