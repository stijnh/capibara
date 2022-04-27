#pragma once

#include <stddef.h>

#include <type_traits>
#include <utility>

#include "axes.h"
#include "axis.h"
#include "const_int.h"
#include "util.h"

namespace capibara {

enum struct ControlFlow {
    Continue,
    Break,
};

template<typename Cursor, typename Dims, typename Axes>
struct Eval;

template<typename Cursor, typename Dims>
struct Eval<Cursor, Dims, AxesOrder<>> {
    CAPIBARA_INLINE
    static ControlFlow call(Cursor& cursor, const Dims&) {
        return cursor.eval();
    }
};

template<typename Cursor, typename Dims, size_t I, size_t... Rest>
struct Eval<Cursor, Dims, AxesOrder<I, Rest...>> {
    CAPIBARA_INLINE
    static ControlFlow call(Cursor& cursor, const Dims& dims) {
        Axis<I> axis;
        auto n = convert_size(dims[axis]);

        for (size_t i = 0; i < n; i++) {
            ControlFlow result =
                Eval<Cursor, Dims, std::index_sequence<Rest...>>::call(
                    cursor,
                    dims);

            if (result == ControlFlow::Break) {
                return ControlFlow::Break;
            }

            cursor.advance(axis, ConstDiff<1> {});
        }

        cursor.advance(axis, convert_diff(n) * ConstDiff<-1> {});
        return ControlFlow::Continue;
    }
};

template<typename Cursor, typename Dims, typename Axes = axes::seq<Dims::rank>>
void evaluate(Cursor& cursor, const Dims& dims, Axes = {}) {
    static_assert(axes::is_distinct<Axes>, "Axes must be distinct");
    static_assert(axes::has_rank<Axes, Dims::rank>, "Axes have incorrect rank");

    Eval<Cursor, Dims, Axes>::call(cursor, dims);
}

}  // namespace capibara