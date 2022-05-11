#include "capybara/array.h"

#include "capybara/dimensions.h"
#include "capybara/nullary.h"
#include "catch.hpp"

#define CHECK_SAME(...) CHECK(std::is_same<__VA_ARGS__>::value)
#define CHECK_IDENTICAL(a, b)             \
    CHECK_SAME(decltype(a), decltype(b)); \
    CHECK(a == b)

TEST_CASE("array") {
    using namespace capybara;

    ArrayBase<int, Dimensions<Dyn, 2>> x = {1, 2};

    using D = Dimensions<Dyn, 2>;
    using X = ArrayBase<int, D>;

    CHECK(is_expr<X>);
    CHECK(is_expr<const X>);
    CHECK(is_expr<X&>);
    CHECK(is_expr<const X&>);
    CHECK(is_expr<X&&>);
    CHECK(is_expr<const X&&>);

    CHECK_SAME(IntoExpr<X>, ArrayRef<int, D>);
    CHECK_SAME(IntoExpr<X&>, ArrayRef<int, D>);
    CHECK_SAME(IntoExpr<X&&>, ArrayRef<int, D>);
    CHECK_SAME(IntoExpr<const X>, ArrayRef<const int, D>);
    CHECK_SAME(IntoExpr<const X&>, ArrayRef<const int, D>);
    CHECK_SAME(IntoExpr<const X&&>, ArrayRef<const int, D>);

    CHECK_SAME(IntoExpr<int>, ValueExpr<int>);
    CHECK_SAME(IntoExpr<int&>, ValueExpr<int>);
    CHECK_SAME(IntoExpr<int&&>, ValueExpr<int>);
    CHECK_SAME(IntoExpr<const int>, ValueExpr<int>);
    CHECK_SAME(IntoExpr<const int&>, ValueExpr<int>);
    CHECK_SAME(IntoExpr<const int&&>, ValueExpr<int>);
}
