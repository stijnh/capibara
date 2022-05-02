#pragma once

#include "../axis.h"

namespace capybara {

namespace mapping {
    template<typename Needle, typename Axis>
    struct InsertHelper;

    template<size_t I>
    struct InsertHelper<Axis<I>, Axis<I>> {
        template<typename F, typename Size>
        static auto dim(F delegate, Axis<I>, Axis<I>, Size size) {
            return size;
        }

        template<typename F>
        static auto stride(F delegate, Axis<I>, Axis<I>) {
            return ConstDiff<0> {};
        }

        template<typename F, typename Steps>
        static void advance(F delegate, Axis<I>, Steps steps, Axis<I>) {}
    };

    template<size_t I, size_t J>
    struct InsertHelper<Axis<I>, Axis<J>> {
        static constexpr Axis<(I < J) ? I : I - 1> newaxis = {};

        template<typename F, typename Size>
        static auto dim(F delegate, Axis<I>, Axis<J>, Size size) {
            return delegate(newaxis);
        }

        template<typename F>
        static auto stride(F delegate, Axis<I>, Axis<J>) {
            return delegate(newaxis);
        }

        template<typename F, typename Steps>
        static void advance(F delegate, Axis<I>, Steps steps, Axis<J>) {
            return delegate(newaxis, steps);
        }
    };

    template<size_t N, typename Axis>
    struct InsertHelper<DynAxis<N>, Axis> {
        template<typename F, typename Size>
        static auto dim(F delegate, DynAxis<N> needle, Axis axis, Size size) {
            return needle == axis ? size : delegate(needle.drop_axis(axis));
        }

        template<typename F>
        static auto stride(F delegate, DynAxis<N> needle, Axis axis) {
            return needle == axis ? ConstDiff<0> {}
                                  : delegate(needle.drop_axis(axis));
        }

        template<typename F, typename Steps>
        static void
        advance(F delegate, DynAxis<N> needle, Steps steps, Axis axis) {
            if (needle != axis) {
                delegate(needle.drop_axis(axis), steps);
            }
        }
    };

    template<size_t N, typename Axis>
    struct InsertCursor {
        InsertCursor(Axis axis) : axis_(axis) {
            assert_rank<N>(axis);
        }

        template<typename F, typename Needle, typename Steps>
        void advance(F delegate, Needle needle, Steps steps) const {
            return InsertHelper<Needle, Axis>::advance(
                delegate,
                needle,
                steps,
                axis_);
        }

      private:
        Axis axis_;
    };

    template<size_t N, typename Axis, typename Size>
    struct Insert {
        static constexpr size_t old_rank = N;
        static constexpr size_t new_rank = N + 1;

        Insert(Axis axis, Size size) : axis_(axis), size_(size) {
            assert_rank<N>(axis);
        }

        template<typename F, typename Needle>
        auto dim(F delegate, Needle needle) const {
            return InsertHelper<Needle, Axis>::dim(
                delegate,
                needle,
                axis_,
                size_);
        }

        template<typename F, typename Needle>
        auto stride(F delegate, Needle needle) const {
            return InsertHelper<Needle, Axis>::stride(delegate, needle, axis_);
        }

        template<typename F, typename D>
        InsertCursor<N, Axis> cursor(F advance, D dim) {
            return InsertCursor<N, Axis>(axis_);
        }

      private:
        Axis axis_;
        Size size_;
    };

    template<size_t N, typename Axis>
    struct RemoveCursor {
        RemoveCursor(Axis axis) : axis_(std::move(axis)) {
            assert_rank<N>(axis);
        }

        template<typename F, typename Needle, typename Steps>
        void advance(F delegate, Needle needle, Steps steps) const {
            return delegate(needle.insert_axis(axis_), steps);
        }

      private:
        Axis axis_;
    };

    template<size_t N, typename Axis, typename Index>
    struct Remove {
        static constexpr size_t old_rank = N;
        static constexpr size_t new_rank = N - 1;

        Remove(Axis axis, Index index) : axis_(axis), index_(index) {
            assert_rank<N>(axis);
        }

        template<typename F, typename Needle>
        auto dim(F delegate, Needle needle) const {
            return delegate(needle.insert_axis(axis_));
        }

        template<typename F, typename Needle>
        auto stride(F delegate, Needle needle) const {
            return delegate(needle.insert_axis(axis_));
        }

        template<typename F, typename D>
        RemoveCursor<N, Axis> cursor(F advance, D dim) {
            advance(axis_, index_);
            return RemoveCursor<N, Axis>(axis_);
        }

      private:
        Axis axis_;
        Index index_;
    };
}  // namespace mapping

}  // namespace capybara