#pragma once

#include <sstream>
#include <stdexcept>

#include "const_int.h"
#include "defines.h"
#include "types.h"
#include "util.h"

namespace capybara {

namespace detail {
    inline void throw_invalid_axis(index_t axis, index_t rank) {
        std::stringstream ss;
        ss << "invalid axis: 0 <= " << axis << " < " << rank;
        throw std::runtime_error(ss.str());
    }
}  // namespace detail

template<index_t I>
using Axis = ConstIndex<I>;

namespace detail {
    template<index_t rank, typename = void>
    struct DynAxisImpl {
        DynAxisImpl(index_t axis) : axis_(axis) {}

        CAPYBARA_INLINE
        constexpr index_t get_impl() const {
            if (axis_ >= rank) {
                CAPYBARA_UNREACHABLE;
            }

            return (index_t)axis_;
        }

      private:
        index_t axis_;
    };

    template<index_t rank>
    struct DynAxisImpl<
        rank,
        enabled<(rank < std::numeric_limits<uint8_t>::max())>> {
        DynAxisImpl(index_t axis) : axis_(axis) {}

        CAPYBARA_INLINE
        constexpr index_t get_impl() const {
            if (axis_ >= rank) {
                CAPYBARA_UNREACHABLE;
            }

            return (index_t)axis_;
        }

      private:
        uint8_t axis_;
    };

    template<>
    struct DynAxisImpl<1> {
        DynAxisImpl(index_t axis) {}

        CAPYBARA_INLINE
        constexpr index_t get_impl() const {
            return 0;
        }
    };
}  // namespace detail

template<index_t N = std::numeric_limits<index_t>::max()>
struct DynAxis: private detail::DynAxisImpl<N> {
    using Base = detail::DynAxisImpl<N>;
    static constexpr index_t rank = N;
    static_assert(rank > 0, "invalid rank given");

  private:
    struct Trusted {};

    CAPYBARA_INLINE
    constexpr DynAxis(Trusted, index_t axis) : Base(axis) {
        // Trusted is private
    }

  public:
    CAPYBARA_INLINE constexpr DynAxis(index_t axis) : Base((index_t)axis) {
        if (axis < 0 || axis >= rank) {
            detail::throw_invalid_axis(axis, rank);
        }
    }

    template<index_t I>
    CAPYBARA_INLINE constexpr DynAxis(Axis<I>) : DynAxis(I) {}

    template<index_t other_rank>
    CAPYBARA_INLINE constexpr DynAxis(DynAxis<other_rank> other_axis) :
        DynAxis(other_axis.get()) {}

    template<index_t other_rank>
    CAPYBARA_INLINE DynAxis& operator=(const DynAxis<other_rank>& other_axis) {
        *this = DynAxis(other_axis);
    }

    CAPYBARA_INLINE
    constexpr index_t get() const {
        return this->get_impl();
    }

    CAPYBARA_INLINE
    constexpr void set(index_t value) {
        *this = DynAxis(value);
    }

    CAPYBARA_INLINE
    constexpr operator index_t() const {
        return get();
    }

    CAPYBARA_INLINE
    constexpr index_t operator()() const {
        return get();
    }
};

template<index_t rank, typename T, T value>
CAPYBARA_INLINE constexpr Axis<(index_t)value>
into_axis(std::integral_constant<T, value>) {
    if (cmp_less(value, 0) || cmp_greater_equal(value, rank)) {
        detail::throw_invalid_axis(value, rank);
    }

    return {};
}

template<index_t rank>
CAPYBARA_INLINE constexpr DynAxis<rank> into_axis(index_t index) {
    return DynAxis<rank> {index};
}

template<index_t rank, index_t other_rank>
CAPYBARA_INLINE constexpr DynAxis<rank>
into_axis(const DynAxis<other_rank>& index) {
    return DynAxis<rank> {index};
}

template<index_t rank, typename T>
CAPYBARA_INLINE constexpr void assert_axis(T&& axis) {
    into_axis<rank>(std::forward<T>(axis));  // discard result
}

template<index_t I>
static constexpr Axis<I> const_axis = {};

#define CAPYBARA_DEFINE_CONSTANT_AXIS(N) \
    using Axis##N = Axis<N>;             \
    static constexpr Axis<N> axis##N = const_axis<N>;

CAPYBARA_DEFINE_CONSTANT_AXIS(0)
CAPYBARA_DEFINE_CONSTANT_AXIS(1)
CAPYBARA_DEFINE_CONSTANT_AXIS(2)
CAPYBARA_DEFINE_CONSTANT_AXIS(3)
CAPYBARA_DEFINE_CONSTANT_AXIS(4)
CAPYBARA_DEFINE_CONSTANT_AXIS(5)
CAPYBARA_DEFINE_CONSTANT_AXIS(6)
CAPYBARA_DEFINE_CONSTANT_AXIS(7)
CAPYBARA_DEFINE_CONSTANT_AXIS(8)
#undef CAPYBARA_DEFINE_CONSTANT_AXIS

}  // namespace capybara