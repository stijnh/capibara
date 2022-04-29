#pragma once

#include "dimensions.h"
#include "forwards.h"

namespace capibara {
namespace mapping {

    template<size_t Rank>
    struct Identity {
        static constexpr size_t old_rank = Rank;
        static constexpr size_t new_rank = Rank;

        template<typename Index>
        std::array<Index, old_rank>
        map(std::array<Index, new_rank> input) const {
            return input;
        }

        template<typename E, typename Axis>
        CAPIBARA_INLINE auto dim(const E& expr, Axis axis) const {
            return expr.dim(axis);
        }

        template<typename E, typename Axis>
        CAPIBARA_INLINE auto stride(const E& expr, Axis axis) const {
            return expr.stride(axis);
        }
    };

    template<typename Axis, size_t Rank, typename Size>
    struct InsertAxis {
        static constexpr size_t old_rank = Rank;
        static constexpr size_t new_rank = Rank + 1;

        InsertAxis(Axis axis, Size size) : axis_(axis), size_(size) {
            // must be axis <= rank, so axis < rank + 1
            assert_rank<Rank + 1>(axis_);
        }

        template<typename Index>
        std::array<Index, old_rank>
            CAPIBARA_INLINE map(std::array<Index, new_rank> input) const {
            std::array<Index, old_rank> output;

            for (size_t i = 0; i < axis_(); i++) {
                output[i] = input[i];
            }

            for (size_t i = axis_(); i < old_rank; i++) {
                output[i] = input[i + 1];
            }

            return output;
        }

        template<typename E>
        size_t dim(const E& expr, size_t axis) const {
            if (axis < axis_()) {
                return expr.dim(axis);
            } else if (axis == axis_()) {
                return size_;
            } else {
                return expr.dim(axis - 1);
            }
        }

        template<typename E>
        size_t stride(const E& expr, size_t axis) const {
            if (axis < axis_()) {
                return expr.stride(axis);
            } else if (axis == axis_()) {
                return 0;
            } else {
                return expr.stride(axis - 1);
            }
        }

      private:
        Axis axis_;
        Size size_;
    };

    template<typename Axis, size_t Rank, typename Index>
    struct RemoveAxis {
        static constexpr size_t old_rank = Rank;
        static constexpr size_t new_rank = Rank - 1;

        RemoveAxis(Axis axis, Index index) : axis_(axis), index_(index) {
            assert_rank<Rank>(axis_);
        }

        std::array<Index, old_rank>
            CAPIBARA_INLINE map(std::array<Index, new_rank> input) const {
            std::array<Index, old_rank> output;

            for (size_t i = 0; i < axis_(); i++) {
                output[i] = input[i];
            }

            output[axis_()] = index_();

            for (size_t i = axis_(); i < new_rank; i++) {
                output[i + 1] = input[i];
            }

            return output;
        }

        template<typename E>
        size_t dim(const E& expr, size_t axis) const {
            if (axis < axis_()) {
                return expr.dim(axis);
            } else {
                return expr.dim(axis + 1);
            }
        }

        template<typename E>
        size_t stride(const E& expr, size_t axis) const {
            if (axis < axis_()) {
                return expr.stride(axis);
            } else {
                return expr.stride(axis + 1);
            }
        }

      private:
        Axis axis_;
        Index index_;
    };

    template<typename AxisA, typename AxisB, size_t Rank>
    struct SwapAxes {
        static constexpr size_t old_rank = Rank;
        static constexpr size_t new_rank = Rank;

        SwapAxes(AxisA a, AxisB b) : a_(a), b_(b) {
            assert_rank<Rank>(a);
            assert_rank<Rank>(b);
        }

        template<typename Index>
        std::array<Index, old_rank>
            CAPIBARA_INLINE map(std::array<Index, new_rank> input) const {
            std::swap(input[a_()], input[b_()]);
            return input;
        }

        template<typename E>
        size_t dim(const E& expr, size_t axis) const {
            if (axis == a_()) {
                return expr.dim(b_());
            } else if (axis == b_()) {
                return expr.dim(a_());
            } else {
                return expr.dim(axis);
            }
        }

        template<typename E>
        size_t stride(const E& expr, size_t axis) const {
            if (axis == a_()) {
                return expr.stride(b_());
            } else if (axis == b_()) {
                return expr.stride(a_());
            } else {
                return expr.stride(axis);
            }
        }

      private:
        AxisA a_;
        AxisB b_;
    };

    template<typename Axis, typename Length, size_t Rank>
    struct ReverseAxis {
        static constexpr size_t old_rank = Rank;
        static constexpr size_t new_rank = Rank;

        ReverseAxis(Axis axis, Length length) : axis_(axis), length_(length) {
            assert_rank<Rank>(axis_);
        }

        template<typename Index>
        std::array<Index, old_rank>
            CAPIBARA_INLINE map(std::array<Index, new_rank> input) const {
            input[axis_()] = length_() - input[axis_()] - 1;
            return input;
        }

        template<typename E>
        CAPIBARA_INLINE size_t dim(const E& expr, size_t axis) const {
            return expr.dim(axis);
        }

        template<typename E>
        size_t stride(const E& expr, size_t axis) const {
            if (axis == axis_()) {
                return -expr.stride(axis);
            } else {
                return expr.stride(axis);
            }
        }

      private:
        Axis axis_;
        Length length_;
    };

    template<size_t Rank>
    struct ReverseAxes {
        static constexpr size_t old_rank = Rank;
        static constexpr size_t new_rank = Rank;

        template<typename Index>
        std::array<Index, Rank>
            CAPIBARA_INLINE map(std::array<Index, new_rank> input) const {
            std::array<Index, old_rank> result;
            for (size_t i = 0; i < old_rank / 2; i++) {
                result[i] = input[old_rank - i - 1];
            }

            return result;
        }

        template<typename E>
        CAPIBARA_INLINE size_t dim(const E& expr, size_t axis) const {
            return expr.dim(old_rank - axis - 1);
        }

        template<typename E>
        CAPIBARA_INLINE size_t stride(const E& expr, size_t axis) const {
            return expr.stride(old_rank - axis - 1);
        }
    };

    template<
        typename Axis,
        typename Start,
        typename End,
        typename Stride,
        size_t Rank>
    struct SliceAxis {
        static constexpr size_t old_rank = Rank;
        static constexpr size_t new_rank = Rank;

        SliceAxis(Axis axis, Start start, End end, Stride stride) :
            axis_(axis),
            start_(start),
            end_(end),
            stride_(stride) {
            assert_rank<Rank>(axis_);

            if (stride == 0) {
                throw std::runtime_error("invalid stride");
            }

            if (start > end) {
                throw std::runtime_error("start exceeds end");
            }
        }

        template<typename Index>
        std::array<Index, old_rank>
            CAPIBARA_INLINE map(std::array<Index, new_rank> index) const {
            index[axis_()] = start_() + index[axis_()] * stride_();
            return index;
        }

        template<typename E>
        size_t dim(const E& expr, size_t axis) const {
            if (axis == axis_()) {
                return (end_ - start_) / stride_;
            } else {
                return expr.dim(axis);
            }
        }

        template<typename E>
        size_t stride(const E& expr, size_t axis) const {
            if (axis == axis_()) {
                return expr.stride(axis) * stride_;
            } else {
                return expr.stride(axis);
            }
        }

      private:
        Axis axis_;
        Start start_;
        End end_;
        Stride stride_;
    };

    template<size_t Rank>
    struct Diagonal {
        static constexpr size_t old_rank = Rank;
        static constexpr size_t new_rank = Rank >= 1 ? 1 : 0;

        template<typename Index>
        std::array<Index, old_rank>
            CAPIBARA_INLINE map(std::array<Index, new_rank> input) const {
            std::array<Index, old_rank> result;
            for (size_t i = 0; i < old_rank; i++) {
                result[i] = input[0];
            }

            return result;
        }

        template<typename E>
        size_t dim(const E& expr, size_t) const {
            size_t result = expr.dim(Axis<0> {});
            for (size_t i = 1; i < old_rank; i++) {
                result = std::min(result, expr.dim(i));
            }
            return result;
        }

        template<typename E>
        size_t stride(const E& expr, size_t axis) const {
            size_t stride = 0;

            for (size_t i = 0; i < old_rank; i++) {
                stride += expr.stride(i);
            }

            return stride;
        }
    };

    template<typename Axis, typename BlockSize, size_t Rank>
    struct SplitAxis {
        static constexpr size_t old_rank = Rank;
        static constexpr size_t new_rank = Rank + 1;

        SplitAxis(Axis axis, BlockSize block_size) :
            axis_(axis),
            block_size_(block_size) {
            assert_rank<old_rank>(axis);
        }

        template<typename Index>
        std::array<Index, old_rank>
            CAPIBARA_INLINE map(std::array<Index, new_rank> input) const {
            std::array<Index, old_rank> output;

            for (size_t i = 0; i < axis_; i++) {
                output[i] = input[i];
            }

            output[axis_] = input[axis_] * block_size_ + input[axis_ + 1];

            for (size_t i = axis_ + 1; i < old_rank; i++) {
                output[i] = input[i + 1];
            }

            return output;
        }

        template<typename E>
        size_t dim(const E& expr, size_t axis) const {
            if (axis == axis_()) {
                return expr.dim(axis) / block_size_;
            } else if (axis == axis_() + 1) {
                return block_size_;
            } else if (axis < axis_()) {
                return expr.dim(axis);
            } else {
                return expr.dim(axis + 1);
            }
        }

        template<typename E>
        size_t stride(const E& expr, size_t axis) const {
            if (axis == axis_()) {
                return expr.stride(axis) * block_size_;
            } else if (axis < axis_() || axis == axis_() + 1) {
                return expr.stride(axis);
            } else {
                return expr.stride(axis + 1);
            }
        }

      private:
        Axis axis_;
        BlockSize block_size_;
    };

    template<typename A, typename B>
    struct Combine {
        static_assert(
            A::new_rank == B::old_rank,
            "invalid combination of axes mappers");
        static constexpr size_t old_rank = A::old_rank;
        static constexpr size_t new_rank = B::new_rank;

        Combine(A a, B b) : a_(a), b_(b) {}

        template<typename Index>
        std::array<Index, old_rank>
            CAPIBARA_INLINE map(std::array<Index, new_rank> index) const {
            return a_.map(b_.map(index));
        }

        template<typename E>
        struct dummy {
            dummy(const A& a, const E& expr) : a_(a), expr_(expr) {}

            CAPIBARA_INLINE
            size_t dim(size_t axis) const {
                return a_.dim(expr_, axis);
            }

            CAPIBARA_INLINE
            size_t stride(size_t axis) const {
                return a_.stride(expr_, axis);
            }

          private:
            const A a_;
            const E& expr_;
        };

        template<typename E>
        CAPIBARA_INLINE size_t dim(const E& expr, size_t axis) const {
            return b_.dim(dummy<E> {a_, expr}, axis);
        }

        template<typename E>
        CAPIBARA_INLINE size_t stride(const E& expr, size_t axis) const {
            return b_.stride(dummy<E> {a_, expr}, axis);
        }

      private:
        A a_;
        B b_;
    };

    template<typename T>
    struct IsIdentityMapper: std::false_type {};

    template<size_t Rank>
    struct IsIdentityMapper<Identity<Rank>>: std::true_type {};

    template<size_t A, size_t Rank>
    struct IsIdentityMapper<SwapAxes<Axis<A>, Axis<A>, Rank>>:
        std::true_type {};

    template<>
    struct IsIdentityMapper<ReverseAxes<0>>: std::true_type {};

    template<>
    struct IsIdentityMapper<ReverseAxes<1>>: std::true_type {};

    template<typename A, size_t Rank>
    struct IsIdentityMapper<ReverseAxis<A, ConstInt<size_t, 1>, Rank>>:
        std::true_type {};

    template<
        typename A,
        typename B,
        bool identityA = IsIdentityMapper<A>::value,
        bool identityB = IsIdentityMapper<B>::value>
    struct CombineHelper {
        Combine<A, B> operator()(A a, B b) {
            return Combine<A, B> {a, b};
        }
    };

    template<typename A, typename B>
    struct CombineHelper<A, B, true, false> {
        B operator()(A, B b) {
            return b;
        }
    };

    template<typename A, typename B>
    struct CombineHelper<A, B, false, true> {
        A operator()(A a, B) {
            return a;
        }
    };

    template<typename A, typename B>
    struct CombineHelper<A, B, true, true> {
        static constexpr size_t rank = A::new_rank;
        static_assert(
            rank == A::new_rank && rank == A::old_rank,
            "invalid Rank");
        static_assert(
            rank == B::new_rank && rank == B::old_rank,
            "invalid Rank");

        Identity<rank> operator()(A, B) {
            return {};
        }
    };

    template<typename A, typename B>
    auto combine(A a, B b) {
        return CombineHelper<A, B> {}(a, b);
    }

}  // namespace mapping

template<typename F, typename Op>
struct MappingCursor {};

template<typename F, typename Op>
struct ExprTraits<MappingExpr<F, Op>> {
    static_assert(F::old_rank == ExprTraits<Op>::rank, "internal error");

    static constexpr size_t rank = F::new_rank;
    using Value = typename ExprTraits<Op>::Value;
    using Index = typename ExprTraits<Op>::Index;
    using Cursor = MappingCursor<F, Op>;
    using Nested = MappingExpr<F, Op>;
};

template<typename F, typename Op>
struct MappingExpr: Expr<MappingExpr<F, Op>, Op::access_mode> {
    using Base = Expr<MappingExpr<F, Op>, Op::access_mode>;
    using typename Base::Index;
    using typename Base::NdIndex;
    using typename Base::Value;

    MappingExpr(F mapper, const Op& inner) : mapper_(mapper), inner_(inner) {}

    CAPIBARA_INLINE
    Value eval(NdIndex idx) const {
        return inner_.eval(mapper_.map(idx));
    }

    template<typename Axis>
    CAPIBARA_INLINE auto dim(Axis i) const {
        return mapper_.dim(inner_, i);
    }

    template<typename Axis>
    CAPIBARA_INLINE auto stride(Axis i) const {
        return mapper_.stride(inner_, i);
    }

  private:
    F mapper_;
    const Op& inner_;
};

template<typename F, typename E>
auto make_mapping_expr(F op, const E& inner) {
    return MappingExpr<F, E>(op, inner);
}

}  // namespace capibara