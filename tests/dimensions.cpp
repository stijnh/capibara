#include "capybara/dimensions.h"

#include "catch.hpp"

#define CHECK_SAME(...) CHECK(std::is_same<__VA_ARGS__>::value)

TEST_CASE("test Dimensions") {
    using namespace capybara;

    SECTION("check types") {
        CHECK_SAME(Dimensions<>, DimensionsN<0>);
        CHECK_SAME(Dimensions<DynSize>, DimensionsN<1>);
        CHECK_SAME(Dimensions<DynSize, DynSize>, DimensionsN<2>);
        CHECK_SAME(Dimensions<DynSize, DynSize, DynSize>, DimensionsN<3>);

        CHECK_SAME(Dimensions<>, DimensionsDyn<>);
        CHECK_SAME(Dimensions<DynSize>, DimensionsDyn<Dyn>);
        CHECK_SAME(Dimensions<ConstSize<123>>, DimensionsDyn<123>);
        CHECK_SAME(
            Dimensions<DynSize, ConstSize<123>>,
            DimensionsDyn<Dyn, 123>);
        CHECK_SAME(
            Dimensions<ConstSize<123>, DynSize>,
            DimensionsDyn<123, Dyn>);
        CHECK_SAME(Dimensions<DynSize, DynSize>, DimensionsDyn<Dyn, Dyn>);

        CHECK_SAME(Dimensions<>, decltype(dims()));
        CHECK_SAME(Dimensions<DynSize>, decltype(dims(1)));
        CHECK_SAME(Dimensions<DynSize, DynSize>, decltype(dims(1, 2)));

        CHECK_SAME(Dimensions<ConstSize<7>>, decltype(dims(S<7>)));
        CHECK_SAME(
            Dimensions<ConstSize<12>, DynSize>,
            decltype(dims(S<12>, 2)));
        CHECK_SAME(
            Dimensions<DynSize, ConstSize<20>>,
            decltype(dims(1, S<20>)));
    }

    SECTION("operator==") {
        auto d = dims(1, 2, S<3>);
        CHECK(d == dims(1, 2, 3));
        CHECK(d == dims(S<1>, 2, 3));
        CHECK(d == dims(S<1>, S<2>, 3));
        CHECK(d == dims(S<1>, 2, S<3>));

        CHECK(d != dims(1, 2));
        CHECK(d != dims(S<1>, 5, 3));
        CHECK(d != dims(S<8>, S<2>, 3));
        CHECK(d != dims(S<12>, 2, S<3>));
    }

    SECTION("check dims(1, 2, 3)") {
        auto d = dims(1, 2, 3);
        d = dims(1, 2, 3);
        d = dims(S<1>, 2, 3);
        d = dims(1, S<2>, 3);
        d = dims(1, 2, S<3>);

        CHECK_NOTHROW(d = dims(99, 2, 3));
        CHECK_NOTHROW(d = dims(1, 99, 3));
        CHECK_NOTHROW(d = dims(1, 2, 99));
        CHECK_NOTHROW(d = dims(S<99>, 2, 3));
        CHECK_NOTHROW(d = dims(1, S<99>, 3));
        CHECK_NOTHROW(d = dims(1, 2, S<99>));
        CHECK(d == dims(1, 2, 99));
    }

    SECTION("check dims(S<1>, 2, 3)") {
        auto d = dims(S<1>, 2, 3);
        d = dims(1, 2, 3);
        d = dims(S<1>, 2, 3);
        d = dims(1, S<2>, 3);
        d = dims(1, 2, S<3>);

        CHECK_THROWS(d = dims(99, 2, 3));
        CHECK_NOTHROW(d = dims(1, 99, 3));
        CHECK_NOTHROW(d = dims(1, 2, 99));
        CHECK_THROWS(d = dims(S<99>, 2, 3));
        CHECK_NOTHROW(d = dims(1, S<99>, 3));
        CHECK_NOTHROW(d = dims(1, 2, S<99>));
        CHECK(d == dims(1, 2, 99));
    }

    SECTION("check dims(S<1>, S<2>, 3)") {
        auto d = dims(S<1>, S<2>, 3);
        d = dims(1, 2, 3);
        d = dims(S<1>, 2, 3);
        d = dims(1, S<2>, 3);
        d = dims(1, 2, S<3>);

        CHECK_THROWS(d = dims(99, 2, 3));
        CHECK_THROWS(d = dims(1, 99, 3));
        CHECK_NOTHROW(d = dims(1, 2, 99));
        CHECK_THROWS(d = dims(S<99>, 2, 3));
        CHECK_THROWS(d = dims(1, S<99>, 3));
        CHECK_NOTHROW(d = dims(1, 2, S<99>));
        CHECK(d == dims(1, 2, 99));
    }

    SECTION("check dims(S<1>, S<2>, S<3>)") {
        auto d = dims(S<1>, S<2>, S<3>);
        d = dims(1, 2, 3);
        d = dims(S<1>, 2, 3);
        d = dims(1, S<2>, 3);
        d = dims(1, 2, S<3>);

        CHECK_THROWS(d = dims(99, 2, 3));
        CHECK_THROWS(d = dims(1, 99, 3));
        CHECK_THROWS(d = dims(1, 2, 99));
        CHECK_THROWS(d = dims(S<99>, 2, 3));
        CHECK_THROWS(d = dims(1, S<99>, 3));
        CHECK_THROWS(d = dims(1, 2, S<99>));
        CHECK(d == dims(1, 2, 3));
    }

    SECTION("operator=") {
        auto d = dims(1, 2, S<3>);

        d = std::make_tuple<int>(4, 5, 3);
        CHECK(d == dims(4, 5, 3));

        d = std::array<size_t, 3> {6, 7, 3};
        CHECK(d == dims(6, 7, 3));

        d = Dimensions<ConstSize<8>, ConstSize<9>, ConstSize<3>>(8, 9, 3);
        CHECK(d == dims(8, 9, 3));
    }
}