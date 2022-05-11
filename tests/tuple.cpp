#include "capybara/tuple.h"

#include <iostream>

#include "capybara/literals.h"
#include "catch.hpp"

using capybara::decay_t;

#define CHECK_SAME(...) CHECK(std::is_same<__VA_ARGS__>::value);
#define CHECK_IDENTICAL(a, b)                               \
    CHECK_SAME(decay_t<decltype(a)>, decay_t<decltype(b)>); \
    CHECK(a == b);

TEST_CASE("tuple") {
    using namespace capybara;
    using namespace capybara::literals;

    SECTION("constructors") {
        SECTION("default constructors") {
            Tuple<> t1;
            Tuple<int> t2;
            Tuple<int, float> t3;
            Tuple<int, float, std::string> t4;
        }

        SECTION("array constructors") {
            std::array<int, 0> a1 = {};
            std::array<int, 1> a2 = {1};
            std::array<int, 2> a3 = {1, 2};
            std::array<int, 3> a4 = {1, 2, 3};

            Tuple<> r1(a1);
            Tuple<> t1(a1.begin(), a1.end());
            CHECK_THROWS(Tuple<> {a2});

            Tuple<int> r2(a2);
            Tuple<int> t2(a2.begin(), a2.end());
            CHECK_THROWS(Tuple<int> {a1});
            CHECK_THROWS(Tuple<int> {a3});

            Tuple<int, float> t3(a3);
            Tuple<int, float> r3(a3.begin(), a3.end());
            CHECK_THROWS(Tuple<int, float> {a2});
            CHECK_THROWS(Tuple<int, float> {a4});

            Tuple<int, float, ConstIndex<3>> t4(a4);
            Tuple<int, float, ConstIndex<3>> r4(a4.begin(), a4.end());
            CHECK_THROWS(Tuple<int, float, ConstIndex<3>> {a1});
            CHECK_THROWS(Tuple<int, float, ConstIndex<3>> {a3});
        }

        SECTION("direct constructors") {
            Tuple<> r1 {};
            Tuple<int> r2 {123};
            Tuple<int, float> t3 {123, 456};
            Tuple<int, float, ConstIndex<3>> t4 {1, 2, 3};

            CHECK_THROWS(Tuple<int, float, ConstIndex<3>> {1, 2, 4});
        }

        SECTION("tuple constructors") {
            Tuple<int> r2 {Tuple<short> {1}};
            Tuple<int, long> t3 {Tuple<short, short> {1, 2}};
            Tuple<int, long, ConstIndex<3>> t4 {
                Tuple<short, short, short> {1, 2, 3}};

            CHECK_THROWS(Tuple<int, long, ConstIndex<3>> {
                Tuple<int, int, int> {1, 2, 4}});
        }

        SECTION("get/set dynamic") {
            Tuple<int, long, ConstIndex<3>> t {1, 2, 3};
            CHECK(t.get<long>(0) == 1);
            CHECK(t.get<long>(1) == 2);
            CHECK(t.get<long>(2) == 3);
            CHECK_THROWS(t.get<long>(3));
            CHECK_THROWS(t.get<long>(4));

            t.set(0, 8);
            CHECK(t.get<long>(0));

            t.set(1, 11);
            CHECK(t.get<long>(0));

            CHECK_THROWS(t.set(2, 10));  // ConstIndex cannot be set
            CHECK_THROWS(t.set(-1, 10));  // Negative index
            CHECK_THROWS(t.set(3, 10));  // Large index

            CHECK(t.get<long>(0) == 8);
            CHECK(t.get<long>(1) == 11);
            CHECK(t.get<long>(2) == 3);
        }

        SECTION("get/set static") {
            Tuple<int, long, ConstIndex<3>> t {1, 2, 3};
            CHECK(t.get<long>(0_c) == 1);
            CHECK(t.get<long>(1_c) == 2);
            CHECK(t.get<long>(2_c) == 3);
            CHECK_THROWS(t.get<long>(3_c));
            CHECK_THROWS(t.get<long>(4_c));

            CHECK_IDENTICAL(t.get_ref(0_c), 1);
            CHECK_IDENTICAL(t.get_ref(1_c), 2l);
            CHECK_IDENTICAL(t.get_ref(2_c), 3_c);

            t.set(0, 8);
            t.set(1, 11);
            CHECK_THROWS(t.set(2, 123));

            CHECK_IDENTICAL(t.get_ref(0_c), 8);
            CHECK_IDENTICAL(t.get_ref(1_c), 11l);
            CHECK_IDENTICAL(t.get_ref(2_c), 3_c);
        }

        SECTION("equality") {
            SECTION("length of 3") {
                Tuple<int, long, ConstIndex<3>> t {1, 2, 3};
                CHECK(t == t);
                CHECK_FALSE(t != t);

                CHECK(t != Tuple<> {});
                CHECK(t != Tuple<int> {1});
                CHECK(t == Tuple<int, float, int> {1, 2, 3});
                CHECK(t != Tuple<int, int, float, int> {1, 2, 3, 4});

                CHECK(t != std::tuple<> {});
                CHECK(t != std::tuple<int> {1});
                CHECK(t == std::tuple<int, float, int> {1, 2, 3});
                CHECK(t != std::tuple<int, int, float, int> {1, 2, 3, 4});

                CHECK(t != std::array<int, 0> {});
                CHECK(t != std::array<int, 1> {1});
                CHECK(t == std::array<int, 3> {1, 2, 3});
                CHECK(t != std::array<int, 4> {1, 2, 3, 4});
            }

            SECTION("length of 0") {
                Tuple<> t {};
                CHECK(t == t);
                CHECK_FALSE(t != t);

                CHECK(t == Tuple<> {});
                CHECK(t != Tuple<int> {1});
                CHECK(t != Tuple<int, float, int> {1, 2, 3});
                CHECK(t != Tuple<int, int, float, int> {1, 2, 3, 4});

                CHECK(t == std::tuple<> {});
                CHECK(t != std::tuple<int> {1});
                CHECK(t != std::tuple<int, float, int> {1, 2, 3});
                CHECK(t != std::tuple<int, int, float, int> {1, 2, 3, 4});

                CHECK(t == std::array<int, 0> {});
                CHECK(t != std::array<int, 1> {1});
                CHECK(t != std::array<int, 3> {1, 2, 3});
                CHECK(t != std::array<int, 4> {1, 2, 3, 4});
            }
        }

        SECTION("visit") {
            Tuple<int, long, ConstIndex<3>> t {1, 2, 3};
            auto to_string = [](auto x) { return std::to_string(x); };

            SECTION("dynamic") {
                CHECK(t.visit<std::string>(0, to_string) == "1");
                CHECK(t.visit<std::string>(1, to_string) == "2");
                CHECK(t.visit<std::string>(2, to_string) == "3");
                CHECK_THROWS(t.visit<std::string>(-1, to_string));
                CHECK_THROWS(t.visit<std::string>(3, to_string));

                t.visit(1, [](auto& x) { x = -1; });
                CHECK(t.get<int>(1) == -1);
            }

            SECTION("static") {
                CHECK(t.visit<std::string>(0, to_string) == "1");
                CHECK(t.visit<std::string>(1, to_string) == "2");
                CHECK(t.visit<std::string>(2, to_string) == "3");
                CHECK_THROWS(t.visit<std::string>(-1, to_string));
                CHECK_THROWS(t.visit<std::string>(3, to_string));

                t.visit(1_c, [](auto& x) { x = -1; });

                CHECK(t.get_ref(1_c) == -1);
            }
        }

        SECTION("functional") {
            Tuple<int, long, ConstIndex<3>> t {1, 2, 3};

            auto to_string = [](auto x) { return std::to_string(x); };
            CHECK(t.map(to_string) == into_tuple("1", "2", "3"));

            std::vector<std::string> result;
            t.visit_all([&](const auto& x) { result.push_back(to_string(x)); });
            CHECK(result == std::vector<std::string> {"1", "2", "3"});

            auto result2 =
                t.fold(0, [&](auto lhs, auto rhs) { return lhs * 10 + rhs; });
            CHECK(result2 == 123);

            // Check if fold works with constants
            Tuple<ConstIndex<1>, ConstIndex<2>, ConstIndex<3>> t2 {1, 2, 3};
            auto result3 = t2.fold(0_c, [&](auto lhs, auto rhs) {
                return lhs * 10_c + rhs;
            });
            CHECK_IDENTICAL(result3, 123_c);
        }
    }
}