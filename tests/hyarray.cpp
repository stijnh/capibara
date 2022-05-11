/*
#include <iostream>

#include "capybara/hybrid_array.h"
#include "capybara/literals.h"
#include "catch.hpp"

#define CHECK_SAME(...) CHECK(std::is_same<__VA_ARGS__>::value)

TEST_CASE("hybrid array") {
    using namespace capybara;
    using namespace capybara::literals;

    SECTION("make_hybrid_array") {
        auto a = make_hybrid_array<index_t>(1, 2, 3);
        CHECK_SAME(
            decltype(a),
            HybridArray<index_t, hybrid_array::List<Dyn, Dyn, Dyn>>);

        auto b = make_hybrid_array<index_t>(
            std::tuple<index_t, index_t, index_t> {1, 2, 3});
        CHECK_SAME(
            decltype(b),
            HybridArray<index_t, hybrid_array::List<Dyn, Dyn, Dyn>>);
        CHECK(a == b);

        auto c = make_hybrid_array<index_t>(std::array<index_t, 3> {1, 2, 3});
        CHECK_SAME(
            decltype(c),
            HybridArray<index_t, hybrid_array::List<Dyn, Dyn, Dyn>>);
        CHECK(a == c);

        auto d = make_hybrid_array<index_t>(c);
        CHECK_SAME(
            decltype(d),
            HybridArray<index_t, hybrid_array::List<Dyn, Dyn, Dyn>>);
        CHECK(a == d);
    }

    auto x = make_hybrid_array<index_t>(1, 2, 3);

    SECTION("dyn") {
        CHECK_SAME(
            decltype(x),
            HybridArray<index_t, hybrid_array::List<Dyn, Dyn, Dyn>>);

        CHECK(x[0] == 1);
        CHECK(x[1] == 2);
        CHECK(x[2] == 3);

        CHECK(x[0_axis] == 1);
        CHECK(x[1_axis] == 2);
        CHECK(x[2_axis] == 3);

        CHECK(x == x);
        CHECK_FALSE(x != x);
    }

    auto y = make_hybrid_array<index_t>(1, 2_c, 4);

    SECTION("mixed dyn and const") {
        CHECK_SAME(
            decltype(y),
            HybridArray<index_t, hybrid_array::List<Dyn, ConstIndex<2>, Dyn>>);

        CHECK(y[0] == 1);
        CHECK(y[1] == 2);
        CHECK(y[2] == 4);

        CHECK(y[0_axis] == 1);
        CHECK(y[1_axis] == 2);
        CHECK(y[2_axis] == 4);

        CHECK_SAME(decltype(y[0_axis]), index_t);
        CHECK_SAME(decltype(y[1_axis]), ConstIndex<2>);
        CHECK_SAME(decltype(y[2_axis]), index_t);

        CHECK_FALSE(x == y);
        CHECK(x != y);

        CHECK_NOTHROW(x = y);
        CHECK(x == y);
        CHECK_FALSE(x != y);
    }

    SECTION("assign") {
        auto z = make_hybrid_array<index_t>(1, 2_c, 5_c);
        CHECK_SAME(
            decltype(z),
            HybridArray<
                index_t,
                hybrid_array::List<Dyn, ConstIndex<2>, ConstIndex<5>>>);

        CHECK_THROWS(z = y);
        CHECK_NOTHROW(y = z);

        CHECK_NOTHROW(z = std::tuple<index_t, index_t, index_t>(8, 2, 5));
        CHECK_THROWS(z = std::tuple<index_t, index_t, index_t>(1, 2, 6));

        CHECK_NOTHROW(z = std::array<index_t, 3> {8, 2, 5});
        CHECK_THROWS(z = std::array<index_t, 3> {1, 2, 6});
    }
}*/