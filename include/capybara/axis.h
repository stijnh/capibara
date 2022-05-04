#pragma once

#include <cstddef>
#include <limits>
#include <stdexcept>

#include "types.h"
#include "const_int.h"

namespace capybara {

constexpr axis_t MaxRank = (axis_t)std::numeric_limits<uint8_t>::max();

template<axis_t Rank = MaxRank>
struct DynAxis;

template<axis_t I>
struct Axis: ConstInt<axis_t, I> {
    static_assert(I <= MaxRank, "axis exceeds max rank");
    static_assert(I >= 0, "axis is negative");

    constexpr Axis() noexcept = default;
    constexpr Axis(axis_t axis) noexcept {
        if (axis != I) {
            throw std::runtime_error("invalid value given");
        }
    }

    template<axis_t N>
    constexpr void assert_rank() const {
        static_assert(I < N, "Invalid axis given");
    }

    CAPYBARA_INLINE
    constexpr operator axis_t() const {
        return I;
    }

    CAPYBARA_INLINE
    constexpr axis_t get() const {
        return I;
    }

    template<axis_t D>
    CAPYBARA_INLINE constexpr Axis<I + D> increment() const {
        static_assert(D >= 0, "increment must be positive");
        return {};
    }

    template<axis_t J>
        CAPYBARA_INLINE constexpr Axis
        < (I<J) ? I : I - 1> drop_axis(Axis<J>) const {
        return {};
    }

    template<axis_t J>
    CAPYBARA_INLINE constexpr Axis<I <= J ? I : I + 1>
    insert_axis(Axis<J>) const {
        return {};
    }

    template<axis_t N>
    CAPYBARA_INLINE constexpr DynAxis<N - 1> drop_axis(DynAxis<N> axis) const {
        return DynAxis<N>(*this).drop_axis(axis);
    }

    template<axis_t N>
    CAPYBARA_INLINE constexpr DynAxis<N + 1>
    insert_axis(DynAxis<N> axis) const {
        return DynAxis<N>(*this).insert_axis(axis);
    }
};

template<axis_t Rank>
struct DynAxisStorage {
    DynAxisStorage(axis_t axis) : axis_(axis) {}

  public:
    CAPYBARA_INLINE
    constexpr axis_t get() const {
        if (axis_ >= Rank) {
            CAPYBARA_UNREACHABLE;
        }

        return axis_;
    }

  private:
    uint8_t axis_;
};

template<>
struct DynAxisStorage<1> {
    DynAxisStorage(axis_t axis) {}

  public:
    CAPYBARA_INLINE
    constexpr axis_t get() const {
        return 0;
    }
};

template<axis_t Rank>
struct DynAxis: private DynAxisStorage<Rank> {
    static_assert(Rank > 0 && Rank <= MaxRank, "invalid rank given");

  private:
    struct Trusted {};

    constexpr DynAxis(Trusted, axis_t axis) : DynAxisStorage<Rank>(axis) {
        // Trused is private
    }

  public:
    template<axis_t I>
    constexpr DynAxis(Axis<I>) : DynAxisStorage<Rank>(I) {
        static_assert(I < Rank, "invalid axis");
    }

    template<axis_t OtherRank>
    constexpr DynAxis(DynAxis<OtherRank> other_axis) :
        DynAxisStorage<Rank>((size_t)other_axis) {
        other_axis.template assert_rank<Rank>();
    }

    constexpr DynAxis(axis_t axis) : DynAxisStorage<Rank>(axis) {
        if (axis >= Rank) {
            throw std::runtime_error("invalid axis given");
        }
    }

    constexpr DynAxis(int axis) : DynAxis((axis_t)axis) {}

    template<axis_t N>
    constexpr void assert_rank() const {
        if (N > Rank && this->get() >= N) {
            throw std::runtime_error("invalid axis given");
        }
    }

    CAPYBARA_INLINE
    constexpr operator axis_t() const {
        return this->get();
    }

    CAPYBARA_INLINE
    constexpr axis_t operator()() const {
        return this->get();
    }

    template<axis_t D = 1>
    CAPYBARA_INLINE constexpr DynAxis<Rank + D> increment() const {
        static_assert(D >= 0, "increment must be non-negative");
        return {Trusted {}, this->get() + D};
    }

    template<typename Other>
    CAPYBARA_INLINE constexpr DynAxis<Rank - 1> drop_axis(Other other) const {
        static_assert(Rank - 1 > 0, "internal error");
        return {
            Trusted {},
            this->get() < ((axis_t)other) ? this->get() : this->get() - 1};
    }

    template<typename Other>
    CAPYBARA_INLINE constexpr DynAxis<Rank + 1> insert_axis(Other other) const {
        return {
            Trusted {},
            this->get() <= ((axis_t)other) ? this->get() : this->get() + 1};
    }
};

template<axis_t Rank, axis_t I>
CAPYBARA_INLINE constexpr Axis<I> into_axis(Axis<I>) {
    static_assert(I < Rank, "invalid axis");
    return {};
}

template<axis_t Rank, typename T, T Value>
CAPYBARA_INLINE constexpr Axis<(axis_t)Value>
into_axis(std::integral_constant<T, Value>) {
    static_assert(cmp_bounds(Value, 0, Rank - 1), "invalid axis");
    return {};
}

template<axis_t Rank>
CAPYBARA_INLINE constexpr DynAxis<Rank> into_axis(const DynAxis<Rank>& axis) {
    return axis;
}

template<axis_t Rank, typename Axis>
CAPYBARA_INLINE constexpr void assert_rank(const Axis& axis) {
    into_axis<Rank>(axis).template assert_rank<Rank>();
}

namespace detail {
    template<typename A, typename B>
    struct AxesEqual;

    template<axis_t I>
    struct AxesEqual<Axis<I>, Axis<I>> {
        template<typename L, typename R>
        CAPYBARA_INLINE static auto call(Axis<I>, Axis<I>, L left, R) {
            return left(Axis<I> {});
        }
    };

    template<axis_t I, axis_t NotI>
    struct AxesEqual<Axis<I>, Axis<NotI>> {
        template<typename L, typename R>
        CAPYBARA_INLINE static auto call(Axis<I>, Axis<NotI>, L, R right) {
            return right(Axis<I> {});
        }
    };

    template<axis_t I, axis_t N>
    struct AxesEqual<Axis<I>, DynAxis<N>> {
        template<typename L, typename R>
        CAPYBARA_INLINE static auto call(Axis<I> a, DynAxis<N> b, L left, R) {
            return (a == b) ? left(Axis<I> {}) : right(a);
        }
    };

    template<axis_t I, axis_t N>
    struct AxesEqual<DynAxis<N>, Axis<I>> {
        template<typename L, typename R>
        CAPYBARA_INLINE static auto call(DynAxis<N> a, Axis<I> b, L left, R) {
            return (a == b) ? left(Axis<I> {}) : right(a);
        }
    };

    template<axis_t N>
    struct AxesEqual<DynAxis<N>, DynAxis<N>> {
        template<typename L, typename R>
        CAPYBARA_INLINE static auto
        call(DynAxis<N> a, DynAxis<N> b, L left, R) {
            return (a == b) ? left(a) : right(a);
        }
    };
}  // namespace detail

template<typename A, typename B, typename L, typename R>
CAPYBARA_INLINE auto axes_equal(A axis, B reference, L left, B right) {
    return detail::AxesEqual<A, B>::call(axis, reference, left, right);
}

template<typename A, typename B>
CAPYBARA_INLINE auto axes_equal(A axis, B reference) {
    return detail::AxesEqual<A, B>::call(
        axis,
        reference,
        [](auto) { return const_true; },
        [](auto) { return const_false; });
}

#define DEFINE_FIXED_AXIS_CONSTANT(N) \
    using Axis##N = Axis<N>;          \
    static constexpr Axis##N axis##N {};
DEFINE_FIXED_AXIS_CONSTANT(0)
DEFINE_FIXED_AXIS_CONSTANT(1)
DEFINE_FIXED_AXIS_CONSTANT(2)
DEFINE_FIXED_AXIS_CONSTANT(3)
DEFINE_FIXED_AXIS_CONSTANT(4)
DEFINE_FIXED_AXIS_CONSTANT(5)
DEFINE_FIXED_AXIS_CONSTANT(6)
#undef DEFINE_FIXED_AXIS_CONSTANT

}  // namespace capybara