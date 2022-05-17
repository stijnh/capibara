#pragma once

#include "expr.h"
/*
namespace capybara {

    template <typename T, size_t N>
    struct expr_traits<indexed_expr<L, S>> {
        static constexpr size_t rank = N;
        using value_type = std::array<T, rank>;
        static constexpr bool is_writable = false;
        static constexpr bool is_view = false;
    }

    template <typename T, size_t N, typename D>
    struct expr_cursor<indexed_expr<T, N>, D> {
        using type = indexed_cursor<T, N>;
    };

    template <typename T, size_t N>
    struct indexed_expr: expr<indexed_expr<T, N>> {
        using shape_type = std::array<index_t, N>;

        indexed_expr(shape_type shape = {}): shape_(shape) {

        }

        index_t dimension_impl(index_t axis) {
            return shape_[axis];
        }

    private:
        shape_type shape_;
    };

    template <typename T, size_t N>
    struct indexed_cursor {
        using value_type = std::array<T, N>;

        indexed_cursor(const indexed_expr<T, N>&) {}

        void advance(index_t axis, stride_t steps) {
            indices_[axis] += T{steps};
        }

        value_type load() {
            return indices_;
        }

    private:
        std::array<T, N> indices_ = {};
    };
}
 */