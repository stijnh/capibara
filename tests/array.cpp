#include "capibara/array.h"

#include "capibara/nullary.h"
#include "catch.hpp"

TEST_CASE("array") {
    using namespace capibara;

    Array<int, Dyn, 20, 20> x {{8, 20, 20}};

    CHECK(x.size() == 8 * 20 * 20);
    x = fill(1, x.dims());
    CHECK(x(1, 2, 3) == 1);
}