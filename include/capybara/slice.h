#pragma once

#include "mapping.h"

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
        mapping::IndexFun<Start> start_;
        mapping::IndexFun<Length> length_;
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
    return sliceops::Range<Start, Length, Stride> {start, length, stride};
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
    return range(mapping::IndexFun<mapping::Last> {} - length, length);
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
            return mapping::Identity<N>();
        }
    };

    template<size_t N, typename Slice>
    struct IntoMapper<N, sliceops::Reverse<Slice>> {
        template<typename Axis>
        CAPYBARA_INLINE static auto
        into(Axis axis, sliceops::Reverse<Slice> slice) {
            auto lhs = mapping::ReverseAxis<N, Axis>(axis);
            auto rhs = IntoMapper<N, Slice>::into(slice.slice_);
            return mapping::combine(lhs, rhs);
        }
    };

    template<size_t N, typename Start, typename Length, typename Stride>
    struct IntoMapper<N, sliceops::Range<Start, Length, Stride>> {
        template<typename Axis>
        CAPYBARA_INLINE static auto
        into(Axis axis, sliceops::Range<Start, Length, Stride> slice) {
            auto start = mapping::make_index_fun(slice.start);
            auto length = mapping::make_index_fun(slice.length);
            auto stride = convert_diff(slice.stride);

            return mapping::SliceAxis<
                N,
                Axis,
                decltype(start),
                decltype(length),
                decltype(stride)>(axis, start, length, stride);
        }
    };

    template<size_t N, typename Size>
    struct IntoMapper<N, sliceops::Insert<Size>> {
        template<typename Axis>
        CAPYBARA_INLINE static auto into(Axis axis, sliceops::Insert<Size> m) {
            return mapping::Insert<N, Axis, Size>(axis, m.size_);
        }
    };

    template<size_t N>
    struct IntoMapper<N, size_t> {
        template<typename Axis>
        CAPYBARA_INLINE static auto into(Axis axis, size_t idx) {
            return mapping::Remove<N, Axis, size_t>(axis, idx);
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
            return mapping::Remove<N, Axis, ConstInt<size_t, I>>(axis, idx);
        }
    };

    template<size_t I, size_t N, typename... Slices>
    struct IntoMultiMapper {};

    template<size_t I, size_t N>
    struct IntoMultiMapper<I, N> {
        CAPYBARA_INLINE
        static auto call() {
            return mapping::Identity<N> {};
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
            auto rhs = IntoMultiMapper<I + 1 + (M - N), M, Slices...> {}(
                std::forward<Slices>(slices)...);

            return mapping_combine(lhs, rhs);
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
    return make_mapping_expr(op, expr);
}

template<typename Expr, typename... Slices>
CAPYBARA_INLINE auto make_slices_expr(const Expr& expr, Slices... slices) {
    auto op = detail::IntoMultiMapper<
        0,
        Expr::rank,
        typename std::decay<Slices>::type...>::call(slices...);
    return make_mapping_expr(op, expr);
}

}  // namespace capybara