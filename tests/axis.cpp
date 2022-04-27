#include "capibara/dimensions.h"
#include "catch.hpp"

template<size_t Rank, typename T, typename R>
auto test_axis_type(T input, R output) {
    auto axis = capibara::into_axis<Rank>(input);

    CHECK(std::is_same<decltype(axis), R>::value);
    CHECK(axis == output);
}

template<size_t Rank, typename T, typename R>
auto test_axis_types(T input, R output) {
    test_axis_type<Rank, T, R>(input, output);
    test_axis_type<Rank, const T, R>(input, output);
    test_axis_type<Rank, T&, R>(input, output);
    test_axis_type<Rank, const T&, R>(input, output);

    T input_copy = input;
    test_axis_type<Rank, T&&, R>(std::move(input_copy), output);

    T input_copy2 = input;
    test_axis_type<Rank, const T&&, R>(std::move(input_copy2), output);
}

TEST_CASE("test Axis") {
    using namespace capibara;

    constexpr int rank = 3;
    test_axis_types<rank>((int)1, DynAxis<rank>(1));
    test_axis_types<rank>((size_t)1, DynAxis<rank>(1));
    test_axis_types<rank>((DynAxis<rank>)1, DynAxis<rank>(1));
    test_axis_types<rank>((DynAxis<rank - 1>)1, DynAxis<rank>(1));
    test_axis_types<rank>((DynAxis<rank + 1>)1, DynAxis<rank>(1));

    test_axis_types<rank>(Axis1, Axis1);
    test_axis_types<rank>(std::integral_constant<int, 1> {}, Axis1);
    test_axis_types<rank>(std::integral_constant<size_t, 1> {}, Axis1);
    test_axis_types<rank>(std::integral_constant<bool, true> {}, Axis1);

    CHECK_THROWS(into_axis<3>(-1));
    CHECK(into_axis<3>(0) == DynAxis<3>(0));
    CHECK(into_axis<3>(1) == DynAxis<3>(1));
    CHECK(into_axis<3>(2) == DynAxis<3>(2));
    CHECK_THROWS(into_axis<3>(3));
    CHECK_THROWS(into_axis<3>(10));
    CHECK_THROWS(into_axis<3>(-5));
}