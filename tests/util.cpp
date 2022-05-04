#include "capybara/util.h"

#include <cstring>
#include <iostream>

#include "catch.hpp"

TEST_CASE("test safe comparison") {
    using namespace capybara;

    SECTION("int vs unsigned int") {
        int a = -1;
        unsigned int b = std::numeric_limits<unsigned int>::max();
        CHECK(cmp_less(a, b));
        CHECK(cmp_less_equal(a, b));
        CHECK_FALSE(cmp_greater(a, b));
        CHECK_FALSE(cmp_greater_equal(a, b));
        CHECK(cmp_not_equal(a, b));
        CHECK_FALSE(cmp_equal(a, b));
    }

    SECTION("unsigned int vs int") {
        unsigned int a = std::numeric_limits<unsigned int>::max();
        int b = -1;
        CHECK_FALSE(cmp_less(a, b));
        CHECK_FALSE(cmp_less_equal(a, b));
        CHECK(cmp_greater(a, b));
        CHECK(cmp_greater_equal(a, b));
        CHECK(cmp_not_equal(a, b));
        CHECK_FALSE(cmp_equal(a, b));
    }

    SECTION("int min vs unsigned int") {
        int a = std::numeric_limits<int>::min();
        unsigned int b;
        memcpy(&b, &a, sizeof(int));

        CHECK(cmp_less(a, b));
        CHECK(cmp_less_equal(a, b));
        CHECK_FALSE(cmp_greater(a, b));
        CHECK_FALSE(cmp_greater_equal(a, b));
        CHECK(cmp_not_equal(a, b));
        CHECK_FALSE(cmp_equal(a, b));
    }

    SECTION("int min vs unsigned int") {
        int b = std::numeric_limits<int>::min();
        unsigned int a;
        memcpy(&a, &b, sizeof(int));

        CHECK_FALSE(cmp_less(a, b));
        CHECK_FALSE(cmp_less_equal(a, b));
        CHECK(cmp_greater(a, b));
        CHECK(cmp_greater_equal(a, b));
        CHECK(cmp_not_equal(a, b));
        CHECK_FALSE(cmp_equal(a, b));
    }

    SECTION("int min vs unsigned int") {
        unsigned int b = std::numeric_limits<unsigned int>::max();
        int a;
        memcpy(&a, &b, sizeof(int));

        CHECK(cmp_less(a, b));
        CHECK(cmp_less_equal(a, b));
        CHECK_FALSE(cmp_greater(a, b));
        CHECK_FALSE(cmp_greater_equal(a, b));
        CHECK(cmp_not_equal(a, b));
        CHECK_FALSE(cmp_equal(a, b));
    }

    SECTION("int min vs unsigned int") {
        unsigned int a = std::numeric_limits<unsigned int>::max();
        int b;
        memcpy(&b, &a, sizeof(int));

        CHECK_FALSE(cmp_less(a, b));
        CHECK_FALSE(cmp_less_equal(a, b));
        CHECK(cmp_greater(a, b));
        CHECK(cmp_greater_equal(a, b));
        CHECK(cmp_not_equal(a, b));
        CHECK_FALSE(cmp_equal(a, b));
    }
}

TEST_CASE("test safe arithmetic") {}