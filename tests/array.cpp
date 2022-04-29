#include "capybara/array.h"

#include "capybara/binary.h"
#include "capybara/eval.h"
#include "capybara/for_each.h"
#include "capybara/indexed.h"
#include "capybara/nullary.h"
#include "capybara/unary.h"
#include "catch.hpp"

TEST_CASE("array") {
    using namespace capybara;

    for_each(arange(10, 10), [](auto x) {
        std::cout << x[0] << "," << x[1] << std::endl;
    });
}