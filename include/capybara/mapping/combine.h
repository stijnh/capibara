#pragma once

#include "identity.h"
#include "permutate.h"
#include "slice.h"

namespace capybara {

namespace mapping {
    template<typename A, typename B>
    struct CombineCursor {
        CombineCursor(A a, B b) : left_(std::move(a)), right_(std::move(b)) {}

        template<typename F, typename Axis, typename Steps>
        auto advance(F delegate, Axis axis, Steps steps) const {
            auto f = [=](auto naxis, auto nsteps) {
                return left_.advance(delegate, naxis, nsteps);
            };
            return right_.advance(f, axis, steps);
        }

      private:
        A left_;
        B right_;
    };

    template<typename A, typename B, typename = void>
    struct Combine {
        static_assert(A::new_rank == B::old_rank, "invalid rank");
        static constexpr size_t old_rank = A::old_rank;
        static constexpr size_t new_rank = B::new_rank;

        Combine(A a, B b) : left_(std::move(a)), right_(std::move(b)) {}

        const A& first() {
            return left_;
        }

        const B& second() {
            return right_;
        }

        template<typename F, typename Axis>
        auto dim(F delegate, Axis axis) const {
            auto f = [=](auto newaxis) { return left_.dim(delegate, newaxis); };
            return right_.dim(f, axis);
        }

        template<typename F, typename Axis>
        auto stride(F delegate, Axis axis) const {
            auto f = [=](auto newaxis) {
                return left_.stride(delegate, newaxis);
            };
            return right_.stride(f, axis);
        }

        template<typename F, typename D>
        auto cursor(F advance, D dim) const {
            auto lhs = left_.cursor(advance, dim);

            auto f = [=](auto newaxis, auto newsteps) {
                left_.advance(advance, newaxis, newsteps);
            };
            auto g = [=](auto newaxis) { left_.dim(newaxis); };
            auto rhs = right_.cursor(f, g);

            return CombineCursor<decltype(lhs), decltype(rhs)>(lhs, rhs);
        }

      private:
        A left_;
        B right_;
    };

    template<typename T, typename = void>
    struct IsIdentity: ConstFalse {};

    template<>
    struct IsIdentity<Transpose<0>>: ConstTrue {};
    template<>
    struct IsIdentity<Transpose<1>>: ConstTrue {};
    template<size_t N, size_t I>
    struct IsIdentity<Swap<N, Axis<I>, Axis<I>>>: ConstTrue {};
    template<typename A, typename B>
    struct IsIdentity<Combine<A, B>>:
        ConstBool<IsIdentity<A>::value && IsIdentity<B>::value> {};

    template<size_t N, typename Axis>
    struct IsIdentity<SliceAxis<N, Axis, First, Last, ConstDiff<1>>>:
        ConstTrue {};

    namespace detail {
        template<
            typename L,
            typename R,
            bool = IsIdentity<L>::value,
            bool = IsIdentity<R>::value>
        struct CombineHelper {
            static Combine<L, R> call(L left, R right) {
                return {std::move(left), std::move(right)};
            }
        };

        template<typename L, typename R>
        struct CombineHelper<L, R, false, true> {
            static L call(L left, R right) {
                return left;
            }
        };

        template<typename L, typename R>
        struct CombineHelper<L, R, true, false> {
            static R call(L left, R right) {
                return right;
            }
        };

        template<typename L, typename R>
        struct CombineHelper<L, R, true, true> {
            constexpr static size_t N = L::new_rank;
            static_assert(N == L::new_rank, "internal error");
            static_assert(N == L::old_rank, "internal error");
            static_assert(N == R::new_rank, "internal error");
            static_assert(N == R::old_rank, "internal error");

            static Identity<N> call(L left, R right) {
                return {};
            }
        };

        template<typename LL, typename LR, typename R>
        struct CombineHelper<Combine<LL, LR>, R, false, false> {
            static auto call(Combine<LL, LR> left, R right) {
                return CombineHelper<LL, Combine<LR, R>>::call(
                    left.first(),
                    {left.second(), right});
            }
        };
    }  // namespace detail

    template<typename L, typename R>
    auto combine(L left, R right) {
        using LD = typename std::decay<L>::type;
        using RD = typename std::decay<R>::type;

        return mapping::Combine<LD, RD>::call(
            std::forward<L>(left),
            std::forward<R>(right));
    }
}  // namespace mapping

}  // namespace capybara