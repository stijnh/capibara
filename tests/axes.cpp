#include "capybara/eval.h"
#include "catch.hpp"

#define CHECK_SAME(...) CHECK(std::is_same<__VA_ARGS__>::value)

// Quick and dirty shorthand
template<size_t... Is>
using A = capybara::AxesOrder<Is...>;

TEST_CASE("axes") {
    using namespace capybara;

    CHECK_SAME(axes::seq<0>, A<>);
    CHECK_SAME(axes::seq<1>, A<0>);
    CHECK_SAME(axes::seq<2>, A<0, 1>);
    CHECK_SAME(axes::seq<3>, A<0, 1, 2>);

    CHECK_SAME(axes::join<A<1, 2, 3>, A<4, 5, 6>>, A<1, 2, 3, 4, 5, 6>);
    CHECK_SAME(axes::join<A<1, 2, 3>, A<>>, A<1, 2, 3>);
    CHECK_SAME(axes::join<A<>, A<4, 5, 6>>, A<4, 5, 6>);
    CHECK_SAME(axes::join<A<1, 2, 3>, A<9>>, A<1, 2, 3, 9>);
    CHECK_SAME(axes::join<A<9>, A<4, 5, 6>>, A<9, 4, 5, 6>);

    CHECK_SAME(axes::prepend<A<1, 2, 3>>, A<1, 2, 3>);
    CHECK_SAME(axes::prepend<A<1, 2, 3>, 4>, A<4, 1, 2, 3>);
    CHECK_SAME(axes::prepend<A<1, 2, 3>, 4, 5>, A<4, 5, 1, 2, 3>);
    CHECK_SAME(axes::prepend<A<>>, A<>);
    CHECK_SAME(axes::prepend<A<>, 4>, A<4>);
    CHECK_SAME(axes::prepend<A<>, 4, 5>, A<4, 5>);

    CHECK_SAME(axes::append<A<1, 2, 3>>, A<1, 2, 3>);
    CHECK_SAME(axes::append<A<1, 2, 3>, 4>, A<1, 2, 3, 4>);
    CHECK_SAME(axes::append<A<1, 2, 3>, 4, 5>, A<1, 2, 3, 4, 5>);
    CHECK_SAME(axes::append<A<>>, A<>);
    CHECK_SAME(axes::append<A<>, 4>, A<4>);
    CHECK_SAME(axes::append<A<>, 4, 5>, A<4, 5>);

    CHECK_SAME(axes::difference<A<1, 2, 3>, A<>>, A<1, 2, 3>);
    CHECK_SAME(axes::difference<A<1, 2, 3>, A<1>>, A<2, 3>);
    CHECK_SAME(axes::difference<A<1, 2, 3>, A<2, 1>>, A<3>);
    CHECK_SAME(axes::difference<A<1, 2, 3>, A<2, 1, 3>>, A<>);
    CHECK_SAME(axes::difference<A<>, A<>>, A<>);

    CHECK_SAME(axes::remove<A<1, 2, 3>>, A<1, 2, 3>);
    CHECK_SAME(axes::remove<A<1, 2, 3>, 1>, A<2, 3>);
    CHECK_SAME(axes::remove<A<1, 2, 3>, 2, 1>, A<3>);
    CHECK_SAME(axes::remove<A<1, 2, 3>, 2, 1, 3>, A<>);
    CHECK_SAME(axes::remove<A<>>, A<>);

    CHECK_SAME(axes::reverse<A<>>, A<>);
    CHECK_SAME(axes::reverse<A<1>>, A<1>);
    CHECK_SAME(axes::reverse<A<1, 2>>, A<2, 1>);
    CHECK_SAME(axes::reverse<A<1, 2, 3>>, A<3, 2, 1>);
    CHECK_SAME(axes::reverse<A<1, 2, 2>>, A<2, 2, 1>);

    CHECK_SAME(axes::insert_at<axes::seq<0>, 0, 8>, A<8>);
    CHECK_SAME(axes::insert_at<axes::seq<1>, 0, 8>, A<8, 0>);
    CHECK_SAME(axes::insert_at<axes::seq<1>, 1, 8>, A<0, 8>);
    CHECK_SAME(axes::insert_at<axes::seq<2>, 0, 8>, A<8, 0, 1>);
    CHECK_SAME(axes::insert_at<axes::seq<2>, 1, 8>, A<0, 8, 1>);
    CHECK_SAME(axes::insert_at<axes::seq<2>, 2, 8>, A<0, 1, 8>);
    CHECK_SAME(axes::insert_at<axes::seq<3>, 0, 8>, A<8, 0, 1, 2>);
    CHECK_SAME(axes::insert_at<axes::seq<3>, 1, 8>, A<0, 8, 1, 2>);
    CHECK_SAME(axes::insert_at<axes::seq<3>, 2, 8>, A<0, 1, 8, 2>);
    CHECK_SAME(axes::insert_at<axes::seq<3>, 3, 8>, A<0, 1, 2, 8>);

    CHECK_SAME(axes::remove_at<axes::seq<1>, 0>, A<>);
    CHECK_SAME(axes::remove_at<axes::seq<2>, 0>, A<1>);
    CHECK_SAME(axes::remove_at<axes::seq<2>, 1>, A<0>);
    CHECK_SAME(axes::remove_at<axes::seq<3>, 0>, A<1, 2>);
    CHECK_SAME(axes::remove_at<axes::seq<3>, 1>, A<0, 2>);
    CHECK_SAME(axes::remove_at<axes::seq<3>, 2>, A<0, 1>);
    CHECK_SAME(axes::remove_at<axes::seq<4>, 0>, A<1, 2, 3>);
    CHECK_SAME(axes::remove_at<axes::seq<4>, 1>, A<0, 2, 3>);
    CHECK_SAME(axes::remove_at<axes::seq<4>, 2>, A<0, 1, 3>);
    CHECK_SAME(axes::remove_at<axes::seq<4>, 3>, A<0, 1, 2>);

    CHECK((axes::at<A<7>, 0>) == 7);
    CHECK((axes::at<A<7, 1>, 0>) == 7);
    CHECK((axes::at<A<7, 1>, 1>) == 1);
    CHECK((axes::at<A<7, 1, 5>, 0>) == 7);
    CHECK((axes::at<A<7, 1, 5>, 1>) == 1);
    CHECK((axes::at<A<7, 1, 5>, 2>) == 5);

    CHECK((axes::is_distinct<A<>>));
    CHECK((axes::is_distinct<A<0>>));
    CHECK_FALSE((axes::is_distinct<A<3, 3>>));
    CHECK_FALSE((axes::is_distinct<A<0, 0>>));
    CHECK((axes::is_distinct<A<1, 2, 3>>));
    CHECK_FALSE((axes::is_distinct<A<3, 2, 3>>));
    CHECK((axes::is_distinct<A<1, 3>>));

    CHECK((axes::is_permutation<A<>>));
    CHECK((axes::is_permutation<A<0>>));
    CHECK_FALSE((axes::is_permutation<A<0, 0>>));
    CHECK_FALSE((axes::is_permutation<A<1, 2, 3>>));
    CHECK_FALSE((axes::is_permutation<A<3, 2, 3>>));
    CHECK_FALSE((axes::is_permutation<A<1, 3>>));
}