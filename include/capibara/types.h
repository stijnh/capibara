#pragma once

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
    #define CAPIBARA_INLINE __attribute__((always_inline)) inline
#elif EIGEN_COMP_MSVC || EIGEN_COMP_ICC
    #define CAPIBARA_INLINE __forceinline
#else
    #define CAPIBARA_INLINE inline
#endif

namespace capibara {

template<size_t I>
struct Axis {
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
    constexpr size_t operator()() const {
        return I;
    }
};

struct DynAxis {
    template<size_t I>
    constexpr DynAxis(Axis<I>) : axis_(I) {
        static_assert(I < std::numeric_limits<uint8_t>::max(), "invalid axis");
    }

    constexpr DynAxis(size_t axis) : axis_(axis) {
        if (axis >= std::numeric_limits<uint8_t>::max()) {
            throw std::runtime_error("invalid axis given");
        }
    }

    constexpr DynAxis(int axis) : axis_(axis) {
        if (axis < 0 || axis >= std::numeric_limits<uint8_t>::max()) {
            throw std::runtime_error("invalid axis given");
        }
    }

    constexpr DynAxis(uint8_t axis) : axis_(axis) {}

    template<size_t N>
    constexpr void assert_rank() const {
        if (axis_ >= N) {
            throw std::runtime_error("invalid axis given");
        }
    }

    CAPIBARA_INLINE
    constexpr operator size_t() const {
        return axis_;
    }

    CAPIBARA_INLINE
    constexpr size_t operator()() const {
        return axis_;
    }

  private:
    uint8_t axis_;
};

template<size_t I>
CAPIBARA_INLINE constexpr Axis<I> into_axis(Axis<I>) {
    return {};
}

template<typename T, T Value>
CAPIBARA_INLINE constexpr Axis<(size_t)Value>
into_axis(std::integral_constant<T, Value>) {
    return {};
}

CAPIBARA_INLINE
constexpr DynAxis into_axis(const DynAxis& axis) {
    return axis;
}

template<size_t Rank, typename Axis>
CAPIBARA_INLINE constexpr void assert_rank(const Axis& axis) {
    into_axis(axis).template assert_rank<Rank>();
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