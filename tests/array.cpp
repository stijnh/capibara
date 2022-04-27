#include "capibara/array.h"

#include "capibara/binary.h"
#include "capibara/eval.h"
#include "capibara/nullary.h"
#include "capibara/unary.h"
#include "catch.hpp"

struct Cursor {
    template<typename Steps>
    void advance(capibara::DynAxis<2> axis, Steps steps) {
        printf("dyn %d] %d\n", (int)axis, (int)steps);
        index_[axis] += steps;
    }

    template<size_t I, typename Steps>
    void advance(capibara::Axis<I> axis, Steps steps) {
        printf("const %d] %d\n", (int)axis, (int)steps);
        index_[axis] += steps;
    }

    capibara::ControlFlow eval() const {
        printf("eval (%d, %d)\n", (int)index_[0], (int)index_[1]);
        return capibara::ControlFlow::Continue;
    }

  private:
    std::array<int, 2> index_ = {0, 0};
};

TEST_CASE("array") {
    using namespace capibara;
    Cursor cursor;

    evaluate(cursor, dims(3, 3, 3), into_axes(A1, A2, A0));
}