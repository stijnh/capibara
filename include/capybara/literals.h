#pragma once

#include "const_int.h"

namespace capybara {
namespace literals {
    namespace detail {
        template<typename T>
        constexpr T parse_digit(T result) {
            return result;
        }

        template<typename T, typename... Rest>
        constexpr T parse_digit(T result, char first, Rest... rest) {
            if (first < '0' || first > '9') {
                throw "invalid character in constant expression";
            }

            T tmp = (result * 10 + (first - '0'));
            return parse_digit<T>(tmp, rest...);
        }
    }  // namespace detail

    template<char... chars>
    constexpr const_index<detail::parse_digit<stride_t>(0, chars...)>
    operator"" _stride() {
        return {};
    }

    template<char... chars>
    constexpr const_index<detail::parse_digit<index_t>(0, chars...)>
    operator"" _c() {
        return {};
    }
}  // namespace literals
}  // namespace capybara