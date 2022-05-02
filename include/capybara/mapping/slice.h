#pragma once

#include <optional>

#include "../axis.h"
#include "identity.h"

namespace capybara {
namespace mapping {
    struct First {
        template<typename Dim>
        ConstSize<0> operator()(Dim d) const {
            return {};
        }
    };

    struct Last {
        template<typename Dim>
        Dim operator()(Dim d) const {
            return d;
        }
    };

    template<typename F>
    struct IndexFun {
        IndexFun() {}
        IndexFun(F fun) : fun_(std::move(fun)) {}

        template<typename Dim>
        auto eval(Dim d) const {
            return fun_(d);
        }

      private:
        F fun_;
    };

    template<typename F, typename = typename std::result_of<F(size_t)>::type>
    IndexFun<F> make_index_fun(F fun) {
        return IndexFun<F>(fun);
    }

    template<typename F>
    IndexFun<F> make_index_fun(const IndexFun<F>& fun) {
        return fun;
    }

    auto make_index_fun(size_t val) {
        return make_index_fun([=](auto in) { return val; });
    }

    auto make_index_fun(ConstSize<0> val) {
        return IndexFun<First> {};
    }

    template<size_t N>
    auto make_index_fun(ConstSize<N> val) {
        return make_index_fun([=](auto in) { return val; });
    }

#define MAKE_INDEX_FUN_OP(op)                                                \
    template<typename F>                                                     \
    auto operator op(IndexFun<F> lhs, IndexFun<F> rhs) {                     \
        return make_index_fun(                                               \
            [=](auto in) { return lhs.eval(in) op rhs.eval(in); });          \
    }                                                                        \
    template<typename F>                                                     \
    auto operator op(IndexFun<F> lhs, size_t rhs) {                          \
        return make_index_fun([=](auto in) { return lhs.eval(in) op rhs; }); \
    }                                                                        \
    template<typename F, size_t N>                                           \
    auto operator op(IndexFun<F> lhs, ConstSize<N> rhs) {                    \
        return make_index_fun([=](auto in) { return lhs.eval(in) op rhs; }); \
    }                                                                        \
    template<typename F>                                                     \
    auto operator op(size_t lhs, IndexFun<F> rhs) {                          \
        return make_index_fun([=](auto in) { return lhs op rhs.eval(in); }); \
    }                                                                        \
    template<typename F, size_t N>                                           \
    auto operator op(ConstSize<N> lhs, IndexFun<F> rhs) {                    \
        return make_index_fun([=](auto in) { return lhs op rhs.eval(in); }); \
    }

    MAKE_INDEX_FUN_OP(+)
    MAKE_INDEX_FUN_OP(-)
    MAKE_INDEX_FUN_OP(*)
    MAKE_INDEX_FUN_OP(/)

    namespace detail {
        template<typename Needle, typename Axis>
        struct SliceHelper {
            template<typename F, typename Length, typename Stride>
            auto
            dim(F delegate,
                Needle needle,
                Axis axis,
                const IndexFun<Length>& length,
                Stride stride) {
                auto d = delegate(needle);
                return needle == axis ? length.eval(d) / stride : d;
            }

            template<typename F, typename Steps, typename Stride>
            void advance(
                F delegate,
                Needle needle,
                Steps steps,
                Axis axis,
                Stride stride) {
                if (needle == axis) {
                    delegate(needle, steps * stride);
                } else {
                    delegate(needle, steps);
                }
            }
        };

        template<size_t N, size_t I>
        struct SliceHelper<DynAxis<N>, Axis<I>> {
            template<typename F, typename Length, typename Stride>
            auto
            dim(F delegate,
                DynAxis<N> needle,
                Axis<I>,
                const IndexFun<Length>& length,
                Stride stride) {
                return needle == I ? length.eval(delegate(Axis<I> {})) / stride
                                   : delegate(needle);
            }

            template<typename F, typename Steps, typename Stride>
            void advance(
                F delegate,
                DynAxis<N> needle,
                Steps steps,
                Axis<I>,
                Stride stride) {
                if (needle == I) {
                    delegate(Axis<I> {}, steps * stride);
                } else {
                    delegate(needle, steps);
                }
            }
        };

        template<size_t I>
        struct SliceHelper<Axis<I>, Axis<I>> {
            template<typename F, typename Length, typename Stride>
            auto
            dim(F delegate,
                Axis<I>,
                Axis<I>,
                const IndexFun<Length>& length,
                Stride stride) {
                return length.eval(delegate(Axis<I> {})) / stride;
            }

            template<typename F, typename Steps, typename Stride>
            void
            advance(F delegate, Axis<I>, Steps steps, Axis<I>, Stride stride) {
                delegate(Axis<I> {}, steps * stride);
            }
        };

        template<size_t I, size_t NotI>
        struct SliceHelper<Axis<I>, Axis<NotI>> {
            template<typename F, typename Length, Stride stride>
            auto
            dim(F delegate,
                Axis<I>,
                Axis<NotI>,
                const IndexFun<Length>& length,
                Stride stride) {
                return delegate(Axis<I> {});
            }

            template<typename F, typename Steps, typename Stride>
            void advance(
                F delegate,
                Axis<I>,
                Steps steps,
                Axis<NotI>,
                Stride stride) {
                delegate(Axis<I> {}, steps);
            }
        };

    }  // namespace detail

    template<size_t N, typename Axis, typename Stride>
    struct SliceCursor {
        SliceCursor(Axis axis, Stride stride) : axis_(axis), stride_(stride) {}

        template<typename F, typename Needle, typename Steps>
        void advance(F delegate, Needle needle, Steps steps) const {
            return detail::SliceHelper<Needle, Axis>::advance(
                delegate,
                steps,
                needle,
                axis_,
                stride_);
        }

      private:
        Axis axis_;
        Stride stride_;
    };

    template<
        size_t N,
        typename Axis,
        typename Start,
        typename Length,
        typename Stride>
    struct SliceAxis {
        static constexpr size_t old_rank = N;
        static constexpr size_t new_rank = N;

        SliceAxis(
            Axis axis,
            IndexFun<Start> start,
            IndexFun<Length> length,
            Stride stride) :
            axis_(axis),
            start_(start),
            length_(length),
            stride_(stride) {}

        template<typename F, typename Needle>
        auto dim(F delegate, Needle needle) const {
            return detail::SliceHelper<Needle, Axis>::dim(
                delegate,
                needle,
                axis_,
                length_,
                stride_);
        }

        template<typename F, typename D>
        SliceCursor<N, Axis, Stride> cursor(F advance, D dim) const {
            auto d = dim(axis_);
            auto start = start_.eval(d);
            auto length = length_.eval(d);

            if (start > d || length > d || start + length > d) {
                throw std::runtime_error("index out of bounds");
            }

            advance(axis_, start);
            return SliceCursor<N, Axis, Stride>(axis_, stride_);
        }

      private:
        Axis axis_;
        IndexFun<Start> start_;
        IndexFun<Length> length_;
        Stride stride_;
    };

    template<size_t N, typename Axis>
    struct ReverseAxis {
        ReverseAxis(Axis axis) : axis_(axis) {}

        template<typename F, typename Needle>
        auto dim(F delegate, Needle needle) const {
            return delegate(needle);
        }

        template<typename F, typename D>
        SliceCursor<N, Axis, ConstDiff<-1>> cursor(F advance, D dim) const {
            auto d = dim(axis_);

            if (d > 0) {
                advance(axis_, convert_diff(d) - ConstDiff<1> {});
            }

            return {axis_, {}};
        }

      private:
        Axis axis_;
    };

}  // namespace mapping
}  // namespace capybara