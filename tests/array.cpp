#include "capybara/array.h"

#include "catch.hpp"

TEST_CASE("array") {
    using namespace capybara;

    size_t n = 10;
    Array1<int> x(n);

    CHECK(x(all).dims() == dims(n));
    CHECK(x(reverse).dims() == dims(n));
    CHECK(x(first(3)).dims() == dims(3));
    CHECK(x(last(3)).dims() == dims(3));
    CHECK(x(range(3, 6)).dims() == dims(6));
    CHECK(x(newaxis).dims() == dims(1, n));
    CHECK(x(newaxis(5)).dims() == dims(5, n));
    CHECK(x(all, newaxis).dims() == dims(n, 1));
    CHECK(x(all, newaxis(5)).dims() == dims(n, 5));
    CHECK(x(1).dims() == dims());
    CHECK(x(const_int<size_t, 5>).dims() == dims());
}