#pragma once

#include <type_traits>

#include "const_int.h"
#include "forwards.h"
#include "literals.h"

namespace capybara {
namespace view {
    template<size_t N, typename Axis>
    struct insert_axis {
        static constexpr size_t rank_input = N;
        static constexpr size_t rank_output = N + 1;

        insert_axis(Axis axis, index_t length = 1) :
            axis_(axis),
            length_(length) {
            // +1 since we can insert axis at the end
            assert_index<N + 1>(axis);
        }

        template<typename A, typename F>
        CAPYBARA_INLINE index_t dimension(A axis, F delegate) const {
            using namespace literals;

            if (axis < axis_) {
                return delegate(axis);
            } else if (axis == axis_) {
                return length_;
            } else {
                return delegate(axis - 1_c);
            }
        }

        template<typename A, typename F>
        CAPYBARA_INLINE void advance(A axis, F delegate) const {
            using namespace literals;

            if (axis < axis_) {
                return delegate(axis, 1_c);
            } else if (axis > axis_) {
                return delegate(axis - 1_c, 1_stride);
            } else if (axis == axis_) {
                // noop
            }
        }

        template<typename F, typename D>
        void initialize_cursor(F delegate, D dim) const {}

        template<typename E>
        CAPYBARA_INLINE dshape<rank_input>
        convert_shape(E expr, dshape<rank_output> shape) const {
            if (shape[axis_] != length_ && length_ != 1) {
                throw std::runtime_error();
            }

            dshape<rank_input> new_shape;
            for (size_t i = 0; i < axis_; i++) {
                new_shape[i] = shape[i];
            }
            for (size_t i = axis_; i < rank_input; i++) {
                new_shape[i] = shape[i + 1];
            }

            return new_shape;
        }

      private:
        Axis axis_;
        index_t length_;
    };

    template<size_t N, typename Axis, typename Index>
    struct remove_axis {
        static_assert(N > 0, "rank cannot be zero");
        static constexpr size_t rank_input = N;
        static constexpr size_t rank_output = N - 1;

        remove_axis(Axis axis, Index index = 0) : axis_(axis), index_(index) {
            assert_index<N>(axis);
        }

        template<typename A, typename F>
        CAPYBARA_INLINE index_t dimension(A axis, F delegate) const {
            using namespace literals;

            if (axis < axis_) {
                return delegate(axis);
            } else if (axis >= axis_) {
                return delegate(axis + 1_c);
            }
        }

        template<typename A, typename F>
        CAPYBARA_INLINE void advance(A axis, F delegate) const {
            using namespace literals;

            if (axis < axis_) {
                return delegate(axis, 1_c);
            } else if (axis >= axis_) {
                return delegate(axis + 1_c, 1_stride);
            }
        }

        template<typename E>
        CAPYBARA_INLINE dshape<rank_input>
        convert_shape(E expr, dshape<rank_output> shape) const {
            index_t length = expr.dimension(axis_);

            if (length <= index_) {
                throw std::runtime_error("index out of bounds");
            }

            dshape<rank_input> new_shape;
            for (size_t i = 0; i < axis_; i++) {
                new_shape[i] = shape[i];
            }

            new_shape[axis_] = length;

            for (size_t i = axis_; i < rank_input; i++) {
                new_shape[i + 1] = shape[i];
            }

            return new_shape;
        }

        template<typename F, typename D>
        CAPYBARA_INLINE void initialize_cursor(F delegate, D dim) const {
            delegate(axis_, index_);
        }

      private:
        Axis axis_;
        Index index_;
    };

    template<size_t N, typename Axis>
    struct flip_axis {
        static constexpr size_t rank_input = N;
        static constexpr size_t rank_output = N;

        flip_axis(Axis axis) : axis_(axis) {
            assert_index<N>(axis);
        }

        template<typename A, typename F>
        CAPYBARA_INLINE index_t dimension(A axis, F delegate) const {
            return delegate(axis);
        }

        template<typename A, typename F>
        CAPYBARA_INLINE void advance(A axis, F delegate) const {
            using namespace literals;

            if (axis == axis_) {
                delegate(axis, -1_stride);
            } else {
                delegate(axis, 1_stride);
            };
        }

        template<typename E>
        CAPYBARA_INLINE dshape<rank_input>
        convert_shape(E expr, dshape<rank_output> shape) const {
            return shape;
        }

        template<typename F, typename D>
        CAPYBARA_INLINE void initialize_cursor(F delegate, D dim) const {
            using namespace literals;
            delegate(axis_, dim(axis_));
            delegate(axis_, -1_c);
        }

      private:
        Axis axis_;
    };

    template<size_t N, typename Axis>
    struct slice_axis {
        static constexpr size_t rank_input = N;
        static constexpr size_t rank_output = N;

        slice_axis(Axis axis, index_t start, index_t length) :
            axis_(axis),
            start_(start),
            length_(length) {
            assert_index<N>(axis);
        }

        template<typename A, typename F>
        CAPYBARA_INLINE index_t dimension(A axis, F delegate) const {
            if (axis == axis_) {
                return length_;
            } else {
                return delegate(axis);
            }
        }

        template<typename A, typename F>
        CAPYBARA_INLINE void advance(A axis, F delegate) const {
            using namespace literals;
            delegate(axis, 1_stride);
        }

        template<typename E>
        CAPYBARA_INLINE dshape<N> convert_shape(E expr, dshape<N> shape) const {
            if (shape[axis_] != length_) {
                throw std::runtime_error("invalid shape");
            }

            index_t expr_length = expr.dimension(axis_);

            if (start_ + length_ > expr_length) {
                throw std::runtime_error("index out of bounds");
            }

            shape[axis_] = expr_length;
            return shape;
        }

        template<typename F, typename D>
        CAPYBARA_INLINE void initialize_cursor(F delegate, D dim) const {
            delegate(axis_, start_);
        }

      private:
        Axis axis_;
        index_t start_;
        index_t length_;
    };

    template<size_t N, typename Axis, typename Stride>
    struct strided_axis {
        static constexpr size_t rank_input = N;
        static constexpr size_t rank_output = N;

        strided_axis(Axis axis, Stride stride) : axis_(axis), stride_(stride) {
            assert_index<N>(axis);
        }

        template<typename A, typename F>
        CAPYBARA_INLINE index_t dimension(A axis, F delegate) const {
            if (axis == axis_) {
                return delegate(axis) / stride_;
            } else {
                return delegate(axis);
            }
        }

        template<typename A, typename F>
        CAPYBARA_INLINE void advance(A axis, F delegate) const {
            using namespace literals;
            if (axis == axis_) {
                delegate(axis, stride_);
            } else {
                delegate(axis, 1_stride);
            }
        }

        template<typename E>
        CAPYBARA_INLINE dshape<N> convert_shape(E expr, dshape<N> shape) const {
            index_t expr_length = expr.dimension(axis_);

            if (expr_length / stride_ != shape[axis_]) {
                throw std::runtime_error("failed to broadcast shape");
            }

            shape[axis_] = expr_length;
            return shape;
        }

        template<typename F, typename D>
        CAPYBARA_INLINE void initialize_cursor(F delegate, D dim) const {}

      private:
        Axis axis_;
        Stride stride_;
    };

    template<size_t N>
    struct diagonal {
        static constexpr size_t rank_input = N;
        static constexpr size_t rank_output = 1;

        template<typename F>
        CAPYBARA_INLINE index_t dimension(index_t axis, F dim) const {
            index_t m = dim(0);

            seq::for_each_n<N>([&m, &dim](auto i) {
                m = std::min(m, dim(const_index<i> {}));
            });

            return m;
        }

        template<typename F>
        CAPYBARA_INLINE void advance(index_t axis, F delegate) const {
            using namespace literals;
            seq::for_each_n<N>(
                [&delegate](auto i) { delegate(const_index<i> {}, 1_stride); });
        }

        template<typename E>
        CAPYBARA_INLINE dshape<rank_input>
        convert_shape(E expr, dshape<1> shape) const {
            dshape<rank_input> new_shape;
            for (size_t i = 0; i < rank_input; i++) {
                new_shape[i] = expr.dimension(i);
            }

            return new_shape;
        }

        template<typename F, typename D>
        CAPYBARA_INLINE void initialize_cursor(F delegate, D dim) const {}
    };

    template<size_t N, size_t P>
    struct prepend_axes {
        static constexpr size_t rank_input = N;
        static constexpr size_t rank_output = P + N;

        template<typename A, typename F>
        CAPYBARA_INLINE index_t dimension(A axis, F delegate) const {
            using namespace literals;

            if (axis >= index_t(P)) {
                return delegate(
                    into_index<rank_input>(axis - const_index<P> {}));
            } else {
                return 1_c;
            }
        }

        template<typename A, typename F>
        CAPYBARA_INLINE void advance(A axis, F delegate) const {
            if (axis >= index_t(P)) {
                return delegate(
                    into_index<rank_input>(axis - const_index<P> {}));
            }
        }

        template<typename E>
        CAPYBARA_INLINE dshape<rank_input>
        convert_shape(E expr, dshape<rank_output> shape) const {
            dshape<rank_input> new_shape;
            for (size_t i = 0; i < rank_input; i++) {
                new_shape[i] = shape[i + P];
            }

            return new_shape;
        }

        template<typename F, typename D>
        CAPYBARA_INLINE void initialize_cursor(F delegate, D dim) const {}
    };

}  // namespace view

template<typename V, typename C>
struct view_cursor;

template<typename V, typename C>
struct apply_cursor_view {
    using type = view_cursor<V, C>;

    CAPYBARA_INLINE
    static type call(V view, C cursor, dshape<V::rank_input> shape) {
        view.initialize_cursor(
            [&cursor](auto axis, auto steps) { cursor.advance(axis, steps); },
            [&shape](auto i) { return shape[i]; });

        return view_cursor<V, C>(std::move(view), std::move(cursor));
    }
};

template<typename V, typename E>
struct expr_traits<view_expr<V, E>> {
    static_assert(V::rank_input == expr_rank<E>, "rank mismatch");
    static constexpr size_t rank = V::rank_output;
    using value_type = expr_value_type<E>;
    static constexpr bool is_writable = expr_traits<E>::is_writable;
    static constexpr bool is_view = expr_traits<E>::is_view;
};

template<typename V, typename E, typename D>
struct expr_cursor<const view_expr<V, E>, D> {
    using apply_type = apply_cursor_view<V, expr_cursor_type<const E, D>>;
    using type = typename apply_type::type;

    CAPYBARA_INLINE
    static type
    call(const view_expr<V, E>& expr, dshape<V::rank_output> shape, D device) {
        const E& operand = expr.operand();
        dshape<V::rank_input> new_shape =
            expr.view().convert_shape(operand, shape);
        return apply_type::call(
            expr.view(),
            operand.cursor(new_shape, device),
            new_shape);
    }
};

template<typename V, typename E>
struct view_expr: expr<view_expr<V, E>> {
    template<typename, typename>
    friend struct view_cursor;

    static constexpr size_t expr_rank = expr_rank<E>;

    view_expr(V view, E expr) :
        view_(std::move(view)),
        expr_(std::move(expr)) {}

        CAPYBARA_INLINE
    index_t dimension_impl(index_t axis) const {
        const E& expr = expr_;

        return view_.dimension(axis, [&expr](auto new_axis) {
            return expr.dimension(into_index<expr_rank>(new_axis));
        });
    }

    CAPYBARA_INLINE
    stride_t stride_impl(index_t axis) const {
        const E& expr = expr_;
        stride_t result = 0;

        view_.advance(axis, [&result, &expr](auto new_axis, auto new_steps) {
            result += expr.stride(into_index<expr_rank>(new_axis)) * new_steps;
        });

        return result;
    }

    CAPYBARA_INLINE
    const V& view() const {
        return view_;
    }

    CAPYBARA_INLINE
    E& operand() {
        return expr_;
    }

    CAPYBARA_INLINE
    const E& operand() const {
        return expr_;
    }

  private:
    V view_;
    E expr_;
};

template<typename V, typename C>
struct view_cursor {
    using value_type = decltype(std::declval<C>().load());
    static constexpr size_t rank = V::input_rank;

    view_cursor(V view, C cursor) :
        view_(std::move(view)),
        cursor_(std::move(cursor)) {
        //
    }

    template<typename Axis, typename Steps>
    CAPYBARA_INLINE
    void advance(Axis axis, Steps steps) {
        C& cursor = cursor_;

        view_.advance(axis, [steps, &cursor](auto new_axis, auto new_steps) {
            cursor.advance(into_index<rank>(new_axis), new_steps * steps);
        });
    }

    CAPYBARA_INLINE
    value_type load() {
        return cursor_.load();
    };

    CAPYBARA_INLINE
    void store(value_type v) {
        cursor_.store(v);
    }

  private:
    V view_;
    C cursor_;
};

template<typename V, typename E>
using view_expr_type = view_expr<decay_t<V>, into_expr_type<E>>;

template<typename V, typename E>
view_expr_type<V, E> make_view(V&& view, E&& expr) {
    return view_expr_type<V, E>(view, into_expr(expr));
}

}  // namespace capybara