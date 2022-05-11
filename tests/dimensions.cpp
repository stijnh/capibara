#include "capybara/dimensions.h"

#include "capybara/literals.h"
#include "catch.hpp"

#define CHECK_SAME(...) CHECK(std::is_same<__VA_ARGS__>::value)
#define CHECK_IDENTICAL(a, b)             \
    CHECK_SAME(decltype(a), decltype(b)); \
    CHECK(a == b)

TEST_CASE("dimensions") {
    using namespace capybara;
    using namespace capybara::literals;

    SECTION("constructors") {
        // init list
        Dimensions<> {};
        CHECK_THROWS(Dimensions<> {1, 4, 4});
        Dimensions<1, 2, Dyn> {1, 2, 3};
        CHECK_THROWS(Dimensions<1, 2, Dyn> {1, 4, 4});

        // Tuple
        Dimensions<> {Tuple<> {}};
        Dimensions<1, 2, Dyn> {Tuple<int, long, int> {1, 2, 3}};
        CHECK_THROWS(Dimensions<1, 2, Dyn> {Tuple<int, long, int> {3, 2, 3}});

        // Dimensions
        Dimensions<> {Dimensions0 {}};
        Dimensions<1, 2, Dyn> {Dimensions3 {1, 2, 3}};

        // Array
        Dimensions<1, 2, Dyn> {std::array<index_t, 3> {1, 2, 3}};
        CHECK_THROWS(Dimensions<1, 2, Dyn> {std::array<index_t, 2> {1, 2}});
    }

    SECTION("assignment") {
        Dimensions<> empty;
        empty = {};
        empty = capybara::into_tuple();
        empty = std::array<index_t, 0> {};
        empty = Dimensions0 {};

        Dimensions3 dyns;
        dyns = {1, 2, 3};
        dyns = capybara::into_tuple(1, 2, 3);
        dyns = std::array<index_t, 3> {1, 2, 3};
        dyns = Dimensions<1, 2, Dyn> {1, 2, 3};

        Dimensions<1, 2, Dyn> mixed;
        dyns = {1, 2, 3};
        dyns = capybara::into_tuple(1, 2, 3);
        dyns = std::array<index_t, 3> {1, 2, 3};
        dyns = Dimensions3 {1, 2, 3};
    }

    SECTION("get/set") {
        Dimensions<1, 2, Dyn> x = {1, 2, 3};

        SECTION("dynamic") {
            CHECK(x.get(0) == 1);
            CHECK(x.get(1) == 2);
            CHECK(x.get(2) == 3);
            CHECK_THROWS(x.get(-1));
            CHECK_THROWS(x.get(3));

            CHECK_NOTHROW(x.set(0, 1));
            CHECK_THROWS(x.set(0, 10));
            CHECK_NOTHROW(x.set(2, 10));

            CHECK(x.get(2) == 10);
        }

        SECTION("static") {
            CHECK_IDENTICAL(x.get(0_c), 1_c);
            CHECK_IDENTICAL(x.get(1_c), 2_c);
            CHECK_IDENTICAL(x.get(2_c), (index_t)3);

            CHECK_NOTHROW(x.set(0_c, 1));
            CHECK_THROWS(x.set(0_c, 10));
            CHECK_NOTHROW(x.set(2_c, 10));

            CHECK(x.get(2_c) == 10);
        }
    }

    SECTION("into_dims") {
        // array
        CHECK_IDENTICAL(
            (into_dims(std::array<index_t, 5> {1, 2, 3, 4, 5})),
            (Dimensions5 {1, 2, 3, 4, 5}));

        CHECK_IDENTICAL((into_dims(std::array<index_t, 0> {})), Dimensions0 {});

        // Dimensions
        CHECK_IDENTICAL(
            (into_dims(Dimensions<1, 2, Dyn> {1, 2, 3})),
            (Dimensions<1, 2, Dyn> {1, 2, 3}));

        CHECK_IDENTICAL(into_dims(Dimensions<> {}), Dimensions<> {});

        // list of integers
        CHECK_IDENTICAL(into_dims(1, 2, 3), (Dimensions3 {1, 2, 3}));
        CHECK_IDENTICAL(into_dims(), (Dimensions<> {}));

        CHECK_IDENTICAL(
            (into_dims(capybara::into_tuple(1_c, 2_c, 3))),
            (Dimensions<1, 2, Dyn> {1, 2, 3}));
        CHECK_IDENTICAL(into_dims(capybara::into_tuple()), Dimensions0 {});

        // inequality
        CHECK(Dimensions1 {} != Dimensions0 {});
        CHECK(Dimensions1 {} != Dimensions<1> {});
        CHECK(Dimensions1 {} != Dimensions<1, 2> {});
    }
}