#pragma once

#include "mapping.h"

namespace capibara {
namespace slice {
    struct All {
        All operator()() const {
            return {};
        }
    };

    template<typename S>
    struct Reverse {
        Reverse operator()() const {
            return {};
        }

        template<typename S2>
        Reverse<S2> operator()(S2 slice) const {
            return {slice};
        }

        S slice_;
    };

    template<
        typename Start,
        typename End,
        typename Stride = ConstInt<size_t, 1>>
    struct Range {
        Start start_;
        End end_;
        Stride stride_;
    };

    template<typename Size>
    struct Insert {
        Insert<Size> operator()() const {
            return {size_};
        }

        template<typename Size2>
        CAPIBARA_INLINE auto operator()(Size2 s) const {
            auto size = convert_integer<size_t>(s);
            return Insert<decltype(size)> {size};
        }

        Size size_;
    };

    template<typename BlockSize>
    struct Split {
        BlockSize block_size_;
    };

    template<typename T>
    struct IntoMapper {};

    template<>
    struct IntoMapper<All> {
        template<typename Axis, size_t rank>
        CAPIBARA_INLINE static auto into(Axis, size_t, All) {
            return mapping::Identity<rank> {};
        }
    };

    template<typename Slice>
    struct IntoMapper<Reverse<Slice>> {
        template<typename Axis, size_t Rank, typename Dim>
        CAPIBARA_INLINE static auto
        into(Axis axis, Dim dim, Reverse<Slice> slice) {
            auto lhs = IntoMapper<Slice>::template into<Axis, Rank>(
                axis,
                dim,
                slice.slice_);
            auto rhs = mapping::ReverseAxis<Axis, Dim, Rank> {axis, dim};

            static_assert(
                decltype(lhs)::new_rank == decltype(lhs)::old_rank,
                "Cannot reverse slice that adds or removes dimensions");
            return mapping::combine(lhs, rhs);
        }
    };

    template<>
    struct IntoMapper<Reverse<All>> {
        template<typename Axis, size_t Rank, typename Dim>
        CAPIBARA_INLINE static auto into(Axis axis, Dim dim, Reverse<All>) {
            return mapping::ReverseAxis<Axis, Dim, Rank> {axis, dim};
        }
    };

    template<typename Size>
    struct IntoMapper<Insert<Size>> {
        template<typename Axis, size_t Rank>
        CAPIBARA_INLINE static auto into(Axis axis, size_t, Insert<Size> r) {
            auto size = r.size_;
            return mapping::InsertAxis<decltype(axis), Rank, decltype(size)> {
                axis,
                r.size_};
        }
    };

    template<typename Start, typename End, typename Stride>
    struct IntoMapper<Range<Start, End, Stride>> {
        template<typename Axis, size_t Rank>
        CAPIBARA_INLINE static auto
        into(Axis axis, size_t, Range<Start, End, Stride> r) {
            return mapping::SliceAxis<Axis, Start, End, Stride, Rank> {
                axis,
                r.start_,
                r.end_,
                r.stride_};
        }
    };

    template<typename BlockSize>
    struct IntoMapper<Split<BlockSize>> {
        template<typename Axis, size_t Rank>
        CAPIBARA_INLINE static auto
        into(Axis axis, size_t, Split<BlockSize> slice) {
            return mapping::SplitAxis<Axis, BlockSize, Rank> {
                axis,
                slice.block_size_};
        }
    };

    template<>
    struct IntoMapper<size_t> {
        template<typename Axis, size_t Rank>
        CAPIBARA_INLINE static auto into(Axis axis, size_t, size_t index) {
            return mapping::RemoveAxis<Axis, Rank, size_t> {axis, index};
        }
    };

    template<>
    struct IntoMapper<int> {
        template<typename Axis, size_t Rank>
        CAPIBARA_INLINE static auto into(Axis axis, size_t dim, int index) {
            return IntoMapper<size_t>::template into<Axis, Rank>(
                axis,
                dim,
                (size_t)index);
        }
    };

    template<>
    struct IntoMapper<unsigned> {
        template<typename Axis, size_t Rank>
        CAPIBARA_INLINE static auto
        into(Axis axis, size_t dim, unsigned index) {
            return IntoMapper<size_t>::template into<Axis, Rank>(
                axis,
                dim,
                (size_t)index);
        }
    };

    template<size_t I>
    struct IntoMapper<ConstInt<size_t, I>> {
        template<typename Axis, size_t Rank>
        CAPIBARA_INLINE static auto
        into(Axis axis, size_t, ConstInt<size_t, I> index) {
            return mapping::RemoveAxis<Axis, Rank, ConstInt<size_t, I>> {
                axis,
                index};
        }
    };

    template<size_t I, size_t Rank, typename... Slices>
    struct MultiMapperHelper {};

    template<size_t I, size_t rank>
    struct MultiMapperHelper<I, rank> {
        static_assert(
            I >= rank,
            "invalid slice operation, did you pass too few arguments to slice?");

        CAPIBARA_INLINE
        auto operator()() const {
            return mapping::Identity<rank> {};
        }
    };

    template<size_t I, size_t rank, typename Slice, typename... Slices>
    struct MultiMapperHelper<I, rank, Slice, Slices...> {
        static_assert(
            I <= rank,
            "invalid slice operation, did you pass too many arguments to slice?");

        CAPIBARA_INLINE
        auto operator()(Slice slice, Slices... slices) const {
            throw std::runtime_error("no dimension to get");

            auto lhs = IntoMapper<Slice>::template into<Axis<I>, rank>(
                {},
                12345678,
                slice);

            static constexpr size_t new_rank = decltype(lhs)::new_rank;
            auto rhs = MultiMapperHelper<
                I + 1 + new_rank - rank,
                new_rank,
                Slices...> {}(slices...);

            return mapping::combine(lhs, rhs);
        }
    };
}  // namespace slice

static constexpr slice::All all = {};
constexpr slice::Insert<ConstInt<size_t, 0>> newaxis = {};
constexpr slice::Reverse<slice::All> reverse = {};

template<typename Start, typename End, typename Stride>
CAPIBARA_INLINE constexpr auto range(Start start, End end, Stride stride) {
    auto start_ = convert_integer<size_t>(start);
    auto end_ = convert_integer<size_t>(end);
    auto stride_ = convert_integer<size_t>(stride);
    return slice::Range<decltype(start_), decltype(end_), decltype(stride_)> {
        start_,
        end_,
        stride_};
}

template<typename Start, typename End>
CAPIBARA_INLINE constexpr auto range(Start start, End end) {
    return range(start, end, S1);
}

template<typename Count>
CAPIBARA_INLINE constexpr auto first(Count count) {
    return range(S0, count);
}

template<typename Size>
CAPIBARA_INLINE constexpr auto split(Size size) {
    auto size_ = convert_integer<size_t>(size);
    return slice::Split<decltype(size_)> {size_};
}

template<typename Expr, typename Axis, typename Slice>
CAPIBARA_INLINE auto make_slice_expr(const Expr& expr, Axis axis, Slice slice) {
    auto axis_ = into_axis(axis);
    auto op =
        slice::IntoMapper<Slice>::template into<decltype(axis_), Expr::rank>(
            axis_,
            expr.dim(axis_),
            slice);
    return make_mapping_expr(op, expr);
}

template<typename Expr, typename... Slices>
CAPIBARA_INLINE auto make_slices_expr(const Expr& expr, Slices... slices) {
    auto op = slice::MultiMapperHelper<0, Expr::rank, Slices...> {}(slices...);
    return make_mapping_expr(op, expr);
}

}  // namespace capibara