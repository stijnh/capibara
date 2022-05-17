#pragma once
#include <array>

#include "forwards.h"

namespace capybara {
template<typename E, bool V>
struct expr_base;

template<typename E>
struct expr: expr_base<E, expr_traits<E>::is_view> {};

template<typename E, bool V>
struct expr_base {
    using self_type = E;
    using traits_type = expr_traits<self_type>;
    static constexpr size_t rank = traits_type::rank;
    using value_type = typename traits_type::value_type;
    using shape_type = std::array<index_t, rank>;
    using nested_type = typename expr_nested<self_type>::type;
    using const_nested_type = typename expr_nested<const self_type>::type;

    template<typename D>
    using cursor_type = typename expr_cursor<nested_type, D>::type;
    template<typename D>
    using const_cursor_type = typename expr_cursor<const_nested_type, D>::type;

    static_assert(
        std::is_same<
            value_type,
            decltype(std::declval<cursor_type<device_seq>>().load())>::value,
        "invalid value type: value_type does not match cursor_type().load()");
    static_assert(
        std::is_same<
            value_type,
            decltype(
                std::declval<const_cursor_type<device_seq>>().load())>::value,
        "invalid value type: value_type does not match const_cursor_type().load()");

    self_type& self() & {
        return *static_cast<self_type*>(this);
    }

    const self_type& self() const& {
        return *static_cast<const self_type*>(this);
    }

    self_type&& self() && {
        return static_cast<self_type&&>(*static_cast<self_type*>(this));
    }

    nested_type nested() {
        return nested_type(self());
    }

    const_nested_type nested() const {
        return const_nested_type(self());
    }

    template<typename D>
    cursor_type<D> cursor(D device) {
        return cursor_type<D>(nested_type(self()), std::move(device));
    }

    template<typename D>
    const_cursor_type<D> cursor(D device) const {
        return const_cursor_type<D>(
            const_nested_type(self()),
            std::move(device));
    }

    index_t dimension(index_t axis) const {
        return self().dimension_impl(axis);
    }

    shape_type shape() const {
        shape_type shape;
        seq::for_each_n<rank>([&](auto i) { shape[i] = self().dimension(i); });

        return shape;
    }

    size_t size() const {
        index_t result = 0;
        seq::for_each_n<rank>([&](auto i) { result *= self().dimension(i); });
        return static_cast<size_t>(result);
    }

    bool empty() const {
        bool result = false;
        seq::for_each_n<rank>(
            [&](auto i) { result |= self().dimension(i) == 0; });

        return result;
    }
};

template<typename E>
struct expr_base<E, true>: expr_base<E, false> {
    using base_type = expr_base<E, false>;
    using base_type::rank;
    using base_type::self;

    using strides_type = std::array<stride_t, rank>;

    stride_t stride(index_t axis) const {
        return self().stride_impl(axis);
    }

    strides_type strides() const {
        strides_type result;

        for (size_t i = 0; i < rank; i++) {
            result[i] = self().stride(i);
        }

        return result;
    }
};
}  // namespace capybara