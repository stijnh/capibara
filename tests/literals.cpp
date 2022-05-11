#include "capybara/literals.h"

#include "catch.hpp"

#define CHECK_SAME(...) CHECK(std::is_same<__VA_ARGS__>::value)

TEST_CASE("check literals") {
    using namespace capybara;
    using namespace capybara::literals;

    CHECK_SAME(decltype(1_axis), ConstIndex<1>);
    CHECK_SAME(decltype(5_axis), ConstIndex<5>);
    CHECK_SAME(decltype(10_axis), ConstIndex<10>);
    CHECK_SAME(decltype(27_axis), ConstIndex<27>);

    CHECK_SAME(decltype(0_c), ConstIndex<0>);
    CHECK_SAME(decltype(1_c), ConstIndex<1>);
    CHECK_SAME(decltype(5_c), ConstIndex<5>);
    CHECK_SAME(decltype(10_c), ConstIndex<10>);
    CHECK_SAME(decltype(27_c), ConstIndex<27>);
    CHECK_SAME(decltype(123_c), ConstIndex<123>);
    CHECK_SAME(decltype(1234_c), ConstIndex<1234>);

    CHECK_SAME(decltype(-0_c), ConstIndex<-0>);
    CHECK_SAME(decltype(-1_c), ConstIndex<-1>);
    CHECK_SAME(decltype(-27_c), ConstIndex<-27>);
}