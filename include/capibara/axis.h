#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>

#include "const_int.h"

namespace capibara {

constexpr size_t MaxRank = std::numeric_limits<uint8_t>::max();

template<size_t I>
struct Axis: ConstInt<size_t, I> {
    static_assert(I <= MaxRank, "rank is too large");

    constexpr Axis() noexcept = default;
    constexpr Axis(size_t axis) noexcept {
        if (axis != I) {
            throw std::runtime_error("invalid value given");
        }
    }

    template<size_t N>
    constexpr void assert_rank() const {
        static_assert(I < N, "Invalid axis given");
    }

    CAPIBARA_INLINE
    constexpr operator size_t() const {
        return I;
    }

    CAPIBARA_INLINE
    constexpr size_t get() const {
        return I;
    }
};

template<size_t Rank = MaxRank>
struct DynAxis {
    static_assert(Rank > 0 && Rank <= MaxRank, "invalid rank given");

    template<size_t I>
    constexpr DynAxis(Axis<I>) : axis_(I) {
        static_assert(I < Rank, "invalid axis");
    }

    template<size_t OtherRank>
    constexpr DynAxis(DynAxis<OtherRank> axis) : axis_((size_t)axis) {
        if (OtherRank > Rank && axis >= Rank) {
            throw std::runtime_error("invalid axis given");
        }
    }

    constexpr DynAxis(size_t axis) : axis_(axis) {
        if (axis >= Rank) {
            throw std::runtime_error("invalid axis given");
        }
    }

    constexpr DynAxis(int axis) : DynAxis((size_t)axis) {}

    template<size_t N>
    constexpr void assert_rank() const {
        if (axis_ >= N) {
            throw std::runtime_error("invalid axis given");
        }
    }

    CAPIBARA_INLINE
    constexpr size_t get() const {
        if (axis_ >= Rank) {
            CAPIBARA_UNREACHABLE;
        }

        return axis_;
    }

    CAPIBARA_INLINE
    constexpr operator size_t() const {
        return get();
    }

    CAPIBARA_INLINE
    constexpr size_t operator()() const {
        return get();
    }

  private:
    uint8_t axis_;
};

template<size_t Rank, size_t I>
CAPIBARA_INLINE constexpr Axis<I> into_axis(Axis<I>) {
    static_assert(I < Rank, "invalid axis");
    return {};
}

template<size_t Rank, typename T, T Value>
CAPIBARA_INLINE constexpr Axis<(size_t)Value>
into_axis(std::integral_constant<T, Value>) {
    static_assert(cmp_bounds(Value, 0, Rank - 1), "invalid axis");
    return {};
}

template<size_t Rank>
CAPIBARA_INLINE constexpr DynAxis<Rank> into_axis(const DynAxis<Rank>& axis) {
    return axis;
}

template<size_t Rank, typename Axis>
CAPIBARA_INLINE constexpr void assert_rank(const Axis& axis) {
    into_axis<Rank>(axis).template assert_rank<Rank>();
}

#define DEFINE_FIXED_AXIS_CONSTANT(N) \
    static constexpr Axis<N> A##N {}; \
    static constexpr Axis<N> Axis##N {};
DEFINE_FIXED_AXIS_CONSTANT(0)
DEFINE_FIXED_AXIS_CONSTANT(1)
DEFINE_FIXED_AXIS_CONSTANT(2)
DEFINE_FIXED_AXIS_CONSTANT(3)
DEFINE_FIXED_AXIS_CONSTANT(4)
DEFINE_FIXED_AXIS_CONSTANT(5)
DEFINE_FIXED_AXIS_CONSTANT(6)
#undef DEFINE_FIXED_AXIS_CONSTANT

}  // namespace capibara