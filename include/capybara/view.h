#pragma once

#include <type_traits>

#include "const_int.h"
#include "forwards.h"

namespace capybara {
namespace view {
    template<size_t N, typename Axis>
    struct insert_axis {
        insert_axis(Axis axis, index_t length = 1) :
            axis_(axis),
            length_(length) {
            assert_index<N>(axis);
        }

        template<typename A, typename F>
        index_t dimension(A axis, F delegate) {
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
        void advance(A axis, F delegate) {
            using namespace literals;

            if (axis < axis_) {
                return delegate(axis, 1_c);
            } else if (axis > axis_) {
                return delegate(axis - 1_c, 1_stride);
            }
        }

        template<typename F, typename D>
        void initialize_cursor(F delegate, D dim) {}

      private:
        Axis axis_;
        index_t length_;
    };

    template<size_t N, typename Axis, typename Index>
    struct remove_axis {
        remove_axis(Axis axis, Index index = 0) : axis_(axis), index_(index) {
            assert_index<N>(axis);
        }

        template<typename A, typename F>
        index_t dimension(A axis, F delegate) {
            using namespace literals;

            if (axis < axis_) {
                return delegate(axis);
            } else if (axis >= axis_) {
                return delegate(axis + 1_c);
            }
        }

        template<typename A, typename F>
        void advance(A axis, F delegate) {
            using namespace literals;

            if (axis < axis_) {
                return delegate(axis, 1_c);
            } else if (axis >= axis_) {
                return delegate(axis + 1_c, 1_stride);
            }
        }

        template<typename F, typename D>
        void initialize_cursor(F delegate, D dim) {
            delegate(axis_, index_);
        }

      private:
        Axis axis_;
        Index index_;
    };

    template<size_t N, typename Axis>
    struct flip_axis {
        flip_axis(Axis axis) : axis_(axis) {
            assert_index<N>(axis);
        }

        template<typename A, typename F>
        index_t dimension(A axis, F delegate) {
            return delegate(axis);
        }

        template<typename A, typename F>
        void advance(A axis, F delegate) {
            using namespace literals;

            if (axis == axis_) {
                delegate(axis, -1_stride);
            } else {
                delegate(axis, 1_stride);
            };
        }

        template<typename F, typename D>
        void initialize_cursor(F delegate, D dim) {
            using namespace literals;
            delegate(axis_, dim(axis_));
            delegate(axis_, -1_c);
        }

      private:
        Axis axis_;
    };

    template<size_t N, typename Axis>
    struct slice_axis {
        slice_axis(Axis axis, index_t start, index_t length) :
            axis_(axis),
            start_(start),
            length_(length) {
            assert_index<N>(axis);
        }

        template<typename A, typename F>
        index_t dimension(A axis, F delegate) const {
            if (axis == axis_) {
                return length_;
            } else {
                return delegate(axis);
            }
        }

        template<typename A, typename F>
        void advance(A axis, F delegate) {
            delegate(axis, 1_stride);
        }

        template<typename F, typename D>
        void initialize_cursor(F delegate, D dim) {
            delegate(axis_, start_);
        }

      private:
        Axis axis_;
        index_t start_;
        index_t length_;
    };

    template<size_t N, typename Axis, typename Stride>
    struct strided_axis {
        strided_axis(Axis axis, Stride stride) : axis_(axis), stride_(stride) {
            assert_index<N>(axis);
        }

        template<typename A, typename F>
        index_t dimension(A axis, F delegate) const {
            if (axis == axis_) {
                return delegate(axis) / stride_;
            } else {
                return delegate(axis);
            }
        }

        template<typename A, typename F>
        void advance(A axis, F delegate) {
            using namespace literals;
            if (axis == axis_) {
                delegate(axis, stride_);
            } else {
                delegate(axis, 1_stride);
            }
        }

        template<typename F, typename D>
        void initialize_cursor(F delegate, D dim) {}

      private:
        Axis axis_;
        Stride stride_;
    };

    template<size_t N>
    struct diagonal {
        template<typename F>
        index_t dimension(index_t axis, F dim) const {
            index_t m = dim(0);

            seq::for_each_n<N>([&delegate](auto i) {
                m = std::min(m, dim(const_index<i> {}));
            });

            return m;
        }

        template<typename F>
        void advance(index_t axis, F delegate) {
            using namespace literals;
            seq::for_each_n<N>(
                [&delegate](auto i) { delegate(const_index<i> {}, 1_stride); });
        }

        template<typename F, typename D>
        void initialize_cursor(F delegate, D dim) {}
    };
}  // namespace view

template<typename V, typename C>
struct view_cursor;

template<typename V, typename E, typename D>
struct expr_cursor<view_expr<V, E>, D> {
    using type = view_cursor<V, expr_cursor_type<E>>;
};

template<typename V, typename E, typename D>
struct expr_cursor<const view_expr<V, E>, D> {
    using type = view_cursor<V, expr_cursor_type<const E>>;
};

template<typename V, typename E>
struct view_expr: expr<view_expr<V, E>> {
    view_expr(V view, E expr) :
        view_(std::move(view)),
        expr_(std::move(expr)) {}

    index_t dimension_impl(index_t axis) const {
        return view_.dimension(axis, [](index_t new_axis) {
            return expr_.dimension(axis);
        });
    }

    stride_t stride_impl(index_t axis) const {
        stride_t result = 0;

        view_.dimension(
            axis,
            [&result, &expr_](index_t new_axis, stride_t new_steps) {
                result += expr_.stride(new_axis) * new_steps;
            });

        return result;
    }

  private:
    V view_;
    E expr_;
};

template<typename V, typename C>
struct view_cursor {
    using value_type = decltype(std::declval<C>().load());
    static constexpr size_t rank = 1337;

    template<typename E, typename D>
    view_cursor(view_expr<V, E>& p, D device) :
        view_(p.view_),
        cursor_(p.expr_.cursor(device)) {}

    template<typename E, typename D>
    view_cursor(const view_expr<V, E>& p, D device) :
        view_(p.view_),
        cursor_(p.expr_.cursor(device)) {
        view_.initialize_cursor([&cursor_](auto axis, auto steps) {
            cursor.advance(axis, steps);
        });
    }

    template <typename Axis, typename Steps>
    void advance(Axis axis, Steps steps) {
        C& cursor = cursor_;

        view_.advance(
            axis,
            [steps, &cursor](auto new_axis, auto new_steps) {
                cursor.advance(into_index<rank>(new_axis), new_steps * steps);
            });
    }

    value_type load() {
        return cursor_.load();
    };

    void store(value_type v) {
        cursor_.store(v);
    }

  private:
    V view_;
    C cursor_;
};

}  // namespace capybara