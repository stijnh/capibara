#include "capybara/const_int.h"

#include "catch.hpp"

#define CHECK_SAME(...) CHECK(std::is_same<__VA_ARGS__>::value)
#define CHECK_IDENTICAL(a, b)             \
    CHECK_SAME(decltype(a), decltype(b)); \
    CHECK(a == b)

template<size_t Rank, typename T, typename R>
auto test_axis_type(T input, R output) {
    auto axis = capybara::into_index<Rank>(input);
    CHECK_IDENTICAL(axis, output);
}

template<size_t Rank, typename T, typename R>
auto test_index_types(T input, R output) {
    test_axis_type<Rank, T, R>(input, output);
    test_axis_type<Rank, const T, R>(input, output);
    test_axis_type<Rank, T&, R>(input, output);
    test_axis_type<Rank, const T&, R>(input, output);

    T input_copy = input;
    test_axis_type<Rank, T&&, R>(std::move(input_copy), output);

    T input_copy2 = input;
    test_axis_type<Rank, const T&&, R>(std::move(input_copy2), output);
}

TEST_CASE("test ConstIndex") {
    using namespace capybara;

    constexpr int rank = 3;
    test_index_types<rank>((int)1, DynIndex<rank>(1));
    test_index_types<rank>((size_t)1, DynIndex<rank>(1));
    test_index_types<rank>((DynIndex<rank>)1, DynIndex<rank>(1));
    test_index_types<rank>((DynIndex<rank - 1>)1, DynIndex<rank>(1));
    test_index_types<rank>((DynIndex<rank + 1>)1, DynIndex<rank>(1));

    test_index_types<rank>(axis1, axis1);
    test_index_types<rank>(std::integral_constant<int, 1> {}, axis1);
    test_index_types<rank>(std::integral_constant<size_t, 1> {}, axis1);
    test_index_types<rank>(std::integral_constant<bool, true> {}, axis1);

    test_index_types<rank>(ConstInt<int, 1> {}, axis1);
    test_index_types<rank>(ConstInt<size_t, 1> {}, axis1);
    test_index_types<rank>(ConstInt<bool, true> {}, axis1);

    SECTION("check into_index") {
        CHECK(into_index<3>(axis0) == axis0);
        CHECK_THROWS(into_index<3>(axis4));

        CHECK_THROWS(into_index<3>(-1));
        CHECK(into_index<3>(0) == DynIndex<3>(0));
        CHECK(into_index<3>(1) == DynIndex<3>(1));
        CHECK(into_index<3>(2) == DynIndex<3>(2));
        CHECK_THROWS(into_index<3>(3));
        CHECK_THROWS(into_index<3>(10));
        CHECK_THROWS(into_index<3>(1000));
    }

    SECTION("check DynIndex operator=") {
        DynIndex<3> x {2};
        CHECK((x = 1) == 1);
        CHECK_THROWS(x = 10);
    }

    SECTION("check comparison operators") {
        // ConstIndex vs ConstIndex
        CHECK(ConstIndex<2> {} == ConstIndex<2> {});
        CHECK(ConstIndex<2> {} != ConstIndex<3> {});

        // DynIndex vs DynIndex
        CHECK(DynIndex<3> {2} == DynIndex<3> {2});
        CHECK(DynIndex<3> {2} == DynIndex<4> {2});
        CHECK(DynIndex<3> {2} != DynIndex<4> {1});

        // ConstIndex vs int
        CHECK(ConstIndex<2> {} == 2);
        CHECK(ConstIndex<2> {} != 1);

        // DynIndex vs int
        CHECK(DynIndex<3> {2} == 2);
        CHECK(DynIndex<3> {2} != 1);

        // ConstIndex vs DynIndex
        CHECK(ConstIndex<2> {} == DynIndex<4> {2});
        CHECK(ConstIndex<2> {} != DynIndex<4> {3});
    }

    SECTION("check DynIndexImpl") {
        CHECK(std::is_empty<DynIndex<1>>::value);
        CHECK_FALSE(std::is_empty<DynIndex<2>>::value);

        CHECK(sizeof(DynIndex<1>) == 1);
        CHECK(sizeof(DynIndex<2>) == 1);
        CHECK(sizeof(DynIndex<500>) == sizeof(index_t));
    }

    SECTION("get DynIndex") {
        auto axis = DynIndex<3>(1);
        CHECK(axis.get() == 1);
        CHECK((index_t)(axis) == 1);
    }

    SECTION("get ConstIndex") {
        auto axis = const_index<1>;
        CHECK(axis.get() == 1);
        CHECK(axis() == 1);
        CHECK((index_t)(axis()) == 1);
    }

    SECTION("check operator+") {
        CHECK_IDENTICAL(ConstIndex<1> {} + ConstIndex<2> {}, ConstIndex<3> {});
        CHECK_IDENTICAL(ConstIndex<1> {} + DynIndex<5> {2}, DynIndex<6> {3});
        CHECK_IDENTICAL(ConstIndex<1> {} + (index_t)2, (index_t)3);

        CHECK_IDENTICAL(DynIndex<5> {2} + ConstIndex<2> {}, DynIndex<7> {4});
        CHECK_IDENTICAL(DynIndex<5> {2} + DynIndex<5> {4}, DynIndex<9> {6});
        CHECK_IDENTICAL(DynIndex<5> {2} + (index_t)2, (index_t)4);

        CHECK_IDENTICAL((index_t)2 + ConstIndex<2> {}, (index_t)4);
        CHECK_IDENTICAL((index_t)2 + DynIndex<5> {4}, (index_t)6);
        CHECK_IDENTICAL((index_t)2 + (index_t)2, (index_t)4);
    }
}