#pragma once

#include "view.h"

namespace capybara {

namespace sliceops {
    struct All {
        constexpr All operator()() const noexcept {
            return *this;
        }
    };

    template<typename S = All>
    struct Reverse {
        constexpr Reverse<S> operator()() const noexcept {
            return *this;
        }

        template<typename S2>
        constexpr Reverse<S2> operator()(S2 slice) const noexcept {
            return {slice};
        }

        S slice_;
    };

    template<typename Start, typename Length, typename Stride = ConstDiff<1>>
    struct Range {
        Symbolic<Start> start_;
        Symbolic<Length> length_;
        Stride stride_;
    };

    template<typename Size>
    struct Insert {
        constexpr Insert<Size> operator()() const noexcept {
            return *this;
        }

        template<typename Size2>
        CAPYBARA_INLINE auto operator()(Size2 s) const {
            auto size = convert_integer<size_t>(s);
            return Insert<decltype(size)> {size};
        }

        Size size_;
    };

    template<typename BlockSize>
    struct Split {
        BlockSize block_size_;
    };
}  // namespace sliceops

static constexpr sliceops::All all = {};
static constexpr sliceops::Insert<ConstInt<size_t, 1>> newaxis = {};
static constexpr sliceops::Reverse<sliceops::All> reverse = {};

template<typename Start, typename Length, typename Stride>
auto range(Start start, Length length, Stride stride) {
    auto start_ = make_index_fun(start);
    auto length_ = make_index_fun(length);
    return sliceops::Range<typename decltype(start_)::function_type, typename decltype(length_)::function_type, Stride> {start_, length_, stride};
}

template<typename Start, typename Length>
auto range(Start start, Length length) {
    return range(start, length, ConstDiff<1> {});
}

template<typename Length>
auto first(Length length) {
    return range(ConstSize<0> {}, length);
}

template<typename Length>
auto last(Length length) {
    return range(Symbolic<Last> {} - length, length);
}

namespace detail {
    template<size_t N, typename T>
    struct IntoMapper;

    template<size_t N, typename T>
    struct IntoMapper<N, T&> {
        template<typename Axis>
        CAPYBARA_INLINE static auto into(Axis axis, T mapper) {
            return IntoMapper<N, T>::into(axis, mapper);
        }
    };

    template<size_t N, typename T>
    struct IntoMapper<N, T&&> {
        template<typename Axis>
        CAPYBARA_INLINE static auto into(Axis axis, T mapper) {
            return IntoMapper<N, T>::into(axis, mapper);
        }
    };

    template<size_t N, typename T>
    struct IntoMapper<N, const T> {
        template<typename Axis>
        CAPYBARA_INLINE static auto into(Axis axis, const T mapper) {
            return IntoMapper<N, T>::into(axis, mapper);
        }
    };

    template<size_t N>
    struct IntoMapper<N, sliceops::All> {
        template<typename Axis>
        CAPYBARA_INLINE static auto into(Axis, sliceops::All) {
            return view::Identity<N>();
        }
    };

    template<size_t N, typename Slice>
    struct IntoMapper<N, sliceops::Reverse<Slice>> {
        template<typename Axis>
        CAPYBARA_INLINE static auto
        into(Axis axis, sliceops::Reverse<Slice> slice) {
            auto lhs = view::ReverseAxis<N, Axis>(axis);
            auto rhs = IntoMapper<N, Slice>::into(axis, slice.slice_);
            return view::combine(lhs, rhs);
        }
    };

    template<size_t N, typename Start, typename Length, typename Stride>
    struct IntoMapper<N, sliceops::Range<Start, Length, Stride>> {
        template<typename Axis>
        CAPYBARA_INLINE static auto
        into(Axis axis, sliceops::Range<Start, Length, Stride> slice) {
            auto start = make_index_fun(slice.start_);
            auto length = make_index_fun(slice.length_);
            auto stride = convert_diff(slice.stride_);

            return view::SliceAxis<
                N,
                Axis,
                typename decltype(start)::function_type,
                typename decltype(length)::function_type,
                decltype(stride)>(axis, start, length, stride);
        }
    };

    template<size_t N, typename Size>
    struct IntoMapper<N, sliceops::Insert<Size>> {
        template<typename Axis>
        CAPYBARA_INLINE static auto into(Axis axis, sliceops::Insert<Size> m) {
            return view::Insert<N, Axis, Size>(axis, m.size_);
        }
    };

    template<size_t N, typename F>
    struct IntoMapper<N, Symbolic<F>> {
        template<typename Axis>
        CAPYBARA_INLINE static auto into(Axis axis, Symbolic<F> idx) {
            return view::Remove<N, Axis, F>(axis, make_index_fun(idx));
        }
    };

    template<size_t N>
    struct IntoMapper<N, size_t> {
        template<typename Axis>
        CAPYBARA_INLINE static auto into(Axis axis, size_t idx) {
            auto f = make_index_fun(idx);
            return view::Remove<N, Axis, typename decltype(f)::function_type>(axis, f);
        }
    };

    template<size_t N>
    struct IntoMapper<N, int> {
        template<typename Axis>
        CAPYBARA_INLINE static auto into(Axis axis, int idx) {
            return IntoMapper<N, size_t>::into(axis, idx);
        }
    };

    template<size_t N>
    struct IntoMapper<N, unsigned> {
        template<typename Axis>
        CAPYBARA_INLINE static auto into(Axis axis, unsigned idx) {
            return IntoMapper<N, size_t>::into(axis, idx);
        }
    };

    template<size_t N, size_t I>
    struct IntoMapper<N, ConstInt<size_t, I>> {
        template<typename Axis>
        CAPYBARA_INLINE static auto into(Axis axis, ConstInt<size_t, I> idx) {
            auto f = make_index_fun(ConstInt<size_t, I>{});
            return view::Remove<N, Axis, typename decltype(f)::function_type>(axis, f);
        }
    };

    template<size_t I, size_t N, typename... Slices>
    struct IntoMultiMapper {};

    template<size_t I, size_t N>
    struct IntoMultiMapper<I, N> {
        CAPYBARA_INLINE
        static auto call() {
            return view::Identity<N> {};
        }
    };

    template<size_t I, size_t N, typename Slice, typename... Slices>
    struct IntoMultiMapper<I, N, Slice, Slices...> {
        static_assert(
            I <= N,
            "invalid slice operation, did you pass too many arguments to slice?");

        CAPYBARA_INLINE
        static auto call(Slice slice, Slices... slices) {
            auto lhs = IntoMapper<N, Slice>::into(Axis<I> {}, slice);

            static constexpr size_t M = decltype(lhs)::new_rank;
            auto rhs = IntoMultiMapper<I + 1 + M - N, M, Slices...>::call(
                std::forward<Slices>(slices)...);

            return view::combine(lhs, rhs);
        }
    };
}  // namespace detail

template<typename Expr, typename Axis, typename Slice>
CAPYBARA_INLINE auto make_slice_expr(const Expr& expr, Axis axis, Slice slice) {
    auto axis_ = into_axis<Expr::rank>(axis);
    auto op = detail::IntoMapper<Expr::rank, Slice>::into(
        axis_,
        expr.dim(axis_),
        slice);
    return make_view_expr(expr, op);
}

template<typename Expr, typename... Slices>
CAPYBARA_INLINE auto make_slices_expr(const Expr& expr, Slices... slices) {
    auto op = detail::IntoMultiMapper<
        0,
        Expr::rank,
        typename std::decay<Slices>::type...>::call(slices...);
    return make_view_expr(expr, op);
}

}  // namespace capybara