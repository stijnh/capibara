#pragma once

#include "../axis.h"

namespace capybara {
namespace view {
    template<typename Base, typename Axis>
    struct PermutationHelper;

    template<size_t N, typename Base>
    struct Permutation {
        static constexpr size_t old_rank = N;
        static constexpr size_t new_rank = N;

        template<typename Needle>
        auto apply_axis(Needle needle) {
            auto old_needle = into_axis<N>(needle);
            auto new_needle =
                PermutationHelper<Base, decltype(old_needle)>::apply(
                    *(const Base*)this,
                    old_needle);
            return into_axis<N>(new_needle);
        }

        template<typename F, typename Needle>
        auto dim(F delegate, Needle needle) const {
            return delegate(apply_axis(needle));
        }

        template<typename F, typename Needle>
        auto stride(F delegate, Needle needle) const {
            return delegate(apply_axis(needle));
        }

        template<typename F, typename Needle, typename Steps>
        auto advance(F delegate, Needle needle, Steps steps) const {
            return delegate(apply_axis(needle), steps);
        }

        template<typename F, typename D>
        Base cursor(F advance, D dim) {
            return *this;
        }
    };

    template<size_t N>
    struct Transpose: Permutation<N, Transpose<N>> {};

    template<size_t N, size_t K>
    struct PermutationHelper<Transpose<N>, Axis<K>> {
        static Axis<N - K - 1> apply(const Transpose<N>& self, Axis<K>) {
            return {};
        }
    };

    template<size_t N>
    struct PermutationHelper<Transpose<N>, DynAxis<N>> {
        static DynAxis<N> apply(const Transpose<N>& self, DynAxis<N> axis) {
            return DynAxis<N> {N - 1 - axis.get()};
        }
    };

    template<size_t N, typename A, typename B>
    struct Swap: Permutation<N, Swap<N, A, B>> {
        Swap(A first, B second) :
            first_(std::move(first)),
            second_(std::move(second)) {}

        A first() const {
            return first_;
        }

        B second() const {
            return second_;
        }

      private:
        A first_;
        B second_;
    };

    template<size_t N, size_t I, size_t J, size_t K>
    struct PermutationHelper<Swap<N, Axis<I>, Axis<J>>, Axis<K>> {
        using NewAxis = Axis<K == I ? J : (K == J ? I : K)>;

        static NewAxis apply(const Swap<N, Axis<I>, Axis<J>>& self, Axis<K>) {
            return {};
        }
    };

    template<size_t N, typename A, typename B, typename Needle>
    struct PermutationHelper<Swap<N, A, B>, Needle> {
        static DynAxis<N> apply(const Swap<N, A, B>& self, Needle axis) {
            if (axis == self.first()) {
                return self.second();
            } else if (axis == self.second()) {
                return self.first();
            } else {
                return axis;
            }
        }
    };

    template<size_t N, typename Axes>
    struct MoveBack: Permutation<N, MoveBack<N, Axes>> {
        MoveBack(Axes axes) : axes_(std::move(axes)) {}

      private:
        Axes axes_;
    };

    template<
        size_t I,
        size_t N,
        size_t K,
        size_t Residual,
        typename AxesRemaining>
    struct MoveBackStatic;

    template<size_t I, size_t N, size_t K, size_t Residual, size_t... RestAxes>
    struct MoveBackStatic<I, N, K, Residual, AxesOrder<K, RestAxes...>> {
        static constexpr size_t value = N - sizeof...(RestAxes);
    };

    template<
        size_t I,
        size_t N,
        size_t K,
        size_t Residual,
        size_t FirstAxis,
        size_t... RestAxes>
    struct MoveBackStatic<
        I,
        N,
        K,
        Residual,
        AxesOrder<FirstAxis, RestAxes...>> {
        static constexpr size_t residual = Residual - (K > FirstAxis ? 1 : 0);
        static constexpr size_t value =
            MoveBackStatic<I + 1, N, K, residual, AxesOrder<RestAxes...>>::
                newaxis;
    };

    template<size_t I, size_t N, size_t K, size_t Residual>
    struct MoveBackStatic<I, N, K, Residual, AxesOrder<>> {
        static constexpr size_t value = Residual;
    };

    template<size_t N, size_t K, typename Axes>
    struct PermutationHelper<MoveBack<N, Axes>, Axis<K>> {
        static constexpr size_t newaxis =
            MoveBackStatic<0, N, K, K, Axes>::value;

        static Axis<newaxis> apply(const MoveBack<N, Axes>& self, Axis<K>) {
            return {};
        }
    };

    template<size_t I, size_t N, typename AxesRemaining>
    struct MoveBackDyn;

    template<size_t I, size_t N, size_t First, size_t... Rest>
    struct MoveBackDyn<I, N, AxesOrder<First, Rest...>> {
        static DynAxis<N> call(DynAxis<N> axis, DynAxis<N - I> residual) {
            if (axis == First) {
                return Axis<N - 1 - sizeof...(Rest)> {};
            } else {
                return MoveBackDyn<I + 1, N, AxesOrder<Rest...>>::call(
                    axis,
                    residual.drop_axis(Axis<First> {}));
            }
        }
    };

    template<size_t I, size_t N>
    struct MoveBackDyn<I, N, AxesOrder<>> {
        static DynAxis<N> call(DynAxis<N> axis, DynAxis<N - I> residual) {
            return residual;
        }
    };

    template<size_t N, typename Axes>
    struct PermutationHelper<MoveBack<N, Axes>, DynAxis<N>> {
        static DynAxis<N>
        apply(const MoveBack<N, Axes>& self, DynAxis<N> axis) {
            return MoveBackDyn<0, N, Axes>::call(axis, axis);
        }
    };

}  // namespace view
}  // namespace capybara