#include "capybara/apply.h"

#include "capybara/array.h"
#include "capybara/functional.h"
#include "capybara/literals.h"
#include "capybara/ops.h"
#include "capybara/tuple.h"
#include "catch.hpp"

TEST_CASE("apply") {
    using namespace capybara;
    using namespace capybara::literals;
    auto f = [](auto x, auto y, auto z) { return x + y + z; };

    using X = capybara::ArrayBase<int, Dimensions<Dyn, Dyn, 2, Dyn>>;
    using Y = capybara::ArrayBase<int, Dimensions<Dyn, Dyn, Dyn>>;
    using Z = capybara::ArrayBase<int, Dimensions<Dyn, Dyn, 2, 3>>;

    /*
    using E = ApplyExpr<decltype(f), X, Y, Z>;

    CHECK_FALSE(is_expr_dim_const<E, 0>);
    CHECK(is_expr_dim_const<E, 1>);
    CHECK(is_expr_dim_const<E, 2>);
    CHECK(is_expr_dim_const<E, 3>);

    CHECK(expr_dim_const<E, 1> == 1);
    CHECK(expr_dim_const<E, 2> == 2);
    */
    X x = {1, 1, 2, 3};
    Y y = {1, 2, 3};
    Z z {x.dims()};

    x.dim(0);

    auto e = capybara::map(
        [](auto i, auto j, auto k) { return i + j + k; },
        x,
        y,
        1.1);

    e.cursor().eval();

    (x + 1) = "abc";
}