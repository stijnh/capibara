#include "forwards.h"
#include "literals.h"

namespace capybara {
static constexpr size_t max_index_value = std::numeric_limits<index_t>::max();
template<index_t U = max_index_value>
struct dyn_index;

template<typename T, T I>
struct const_integer: std::integral_constant<T, I> {
    const_integer() = default;
    const_integer(const const_integer&) = default;

    CAPYBARA_INLINE
    const_integer(T v) {
        if (v != I) {
            throw std::runtime_error("invalid index");
        }
    }

    CAPYBARA_INLINE
    T get() const {
        return I;
    }

    CAPYBARA_INLINE
    operator T() const {
        return I;
    }

    CAPYBARA_INLINE
    T operator()() const {
        return I;
    }
};

template <typename T, T I>
CAPYBARA_INLINE
const_integer<T, -I> operator-(const_integer<T, I>) {
    return {};
}

#define CAPYBARA_CONST_INTEGER_OP(op)                         \
    template<typename T, T I, T J>              \
    CAPYBARA_INLINE const_integer<T, (I op J)> operator op( \
        const_integer<T, I>,                                \
        const_integer<T, J>) {                              \
        return {};                                          \
    }

CAPYBARA_CONST_INTEGER_OP(+)
CAPYBARA_CONST_INTEGER_OP(-)
CAPYBARA_CONST_INTEGER_OP(*)
CAPYBARA_CONST_INTEGER_OP(/)
CAPYBARA_CONST_INTEGER_OP(%)
#undef CAPYBARA_CONST_INTEGER_OP

template<size_t I>
using const_size = const_integer<size_t, I>;

template<index_t I>
using const_index = const_integer<index_t, I>;

template<stride_t I>
using const_stride = const_integer<stride_t, I>;

template<index_t U>
struct dyn_index {
    CAPYBARA_INLINE
    dyn_index(index_t v) {
        if (v < 0 || v >= U) {
            throw std::runtime_error("invalid index");
        }

        value_ = v;
    }

    template<index_t I>
    CAPYBARA_INLINE dyn_index(const_index<I>) : dyn_index(I) {}

    template<index_t U2>
    CAPYBARA_INLINE dyn_index(dyn_index<U2> v) {
        if (U2 > U && v.get() >= U) {
            throw std::runtime_error("invalid index");
        }

        value_ = v;
    }

    CAPYBARA_INLINE
    index_t get() const {
        if (value_ < 0) {
            CAPYBARA_UNREACHABLE;
        }

        if (value_ >= U) {
            CAPYBARA_UNREACHABLE;
        }

        return value_;
    }

    CAPYBARA_INLINE
    operator index_t() const {
        return get();
    }

    CAPYBARA_INLINE
    index_t operator()() const {
        return get();
    }

  private:
    index_t value_;
};

template<index_t U = max_index_value>
CAPYBARA_INLINE dyn_index<U> into_index(index_t v) {
    return v;
}

template<index_t U = max_index_value, index_t U2>
CAPYBARA_INLINE dyn_index<U> into_index(dyn_index<U2> v) {
    return v;
}

template<index_t U = max_index_value, typename T, T I>
CAPYBARA_INLINE const_index<index_t(I)>
into_index(std::integral_constant<T, I> v) {
    static_assert(I >= 0 && I < U, "invalid index");
    return v;
}

template<index_t U, typename T>
CAPYBARA_INLINE void assert_index(T&& v) {
    into_index(std::forward<T>(v));  // ignore result
}

template<index_t U, index_t J>
CAPYBARA_INLINE dyn_index<saturating_add(U, J)>
operator+(dyn_index<U> v, const_index<J>) {
    return v + J;
}

template<index_t U, index_t J>
CAPYBARA_INLINE dyn_index<saturating_add(U, J)>
operator+(const_index<J>, dyn_index<U> v) {
    return v + J;
}

template<index_t U, index_t U2>
CAPYBARA_INLINE dyn_index<saturating_add(U, U2)>
operator+(dyn_index<U> lhs, dyn_index<U2> rhs) {
    return (index_t)lhs + (index_t)rhs;
}
}  // namespace capybara