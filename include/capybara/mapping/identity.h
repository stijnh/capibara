#pragma once

namespace capybara {
namespace mapping {
    template<size_t N>
    struct Identity {
        static constexpr size_t old_rank = N;
        static constexpr size_t new_rank = N;

        template<typename F, typename Axis>
        auto dim(F delegate, Axis axis) const {
            return delegate(axis);
        }

        template<typename F, typename Axis>
        auto stride(F delegate, Axis axis) const {
            return delegate(axis);
        }

        template<typename F, typename D>
        Identity cursor(F advance, D dim) const {
            return *this;
        }

        template<typename F, typename Axis, typename Steps>
        auto advance(F delegate, Axis axis, Steps steps) const {
            return delegate(axis, steps);
        }
    };
}  // namespace mapping
}  // namespace capybara