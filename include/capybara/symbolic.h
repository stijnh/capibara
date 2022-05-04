#pragma once

namespace capybara {

struct First {
    template<typename Input>
    ConstSize<0> operator()(Input d) const {
        return {};
    }
};

struct Last {
    template<typename Input>
    Input operator()(Input d) const {
        return d;
    }
};

template<typename F>
struct Symbolic {
    using function_type = F;

    Symbolic() {}
    Symbolic(F fun) : fun_(std::move(fun)) {}

    template<typename Input>
    auto eval(Input d) const {
        return fun_(d);
    }

  private:
    F fun_;
};

template<typename F, typename = typename std::result_of<F(size_t)>::type>
Symbolic<F> make_index_fun(F fun) {
    return Symbolic<F>(fun);
}

template<typename F>
Symbolic<F> make_index_fun(const Symbolic<F>& fun) {
    return fun;
}

auto make_index_fun(size_t val) {
    return make_index_fun([=](auto in) { return val; });
}

auto make_index_fun(ConstSize<0> val) {
    return Symbolic<First> {};
}

template<size_t N>
auto make_index_fun(ConstSize<N> val) {
    return make_index_fun([=](auto in) { return ConstSize<N>{}; });
}

#define MAKE_INDEX_FUN_OP(op)                                                \
    template<typename F>                                                     \
    auto operator op(Symbolic<F> lhs, Symbolic<F> rhs) {                     \
        return make_index_fun(                                               \
            [=](auto in) { return lhs.eval(in) op rhs.eval(in); });          \
    }                                                                        \
    template<typename F>                                                     \
    auto operator op(Symbolic<F> lhs, size_t rhs) {                          \
        return make_index_fun([=](auto in) { return lhs.eval(in) op rhs; }); \
    }                                                                        \
    template<typename F, size_t N>                                           \
    auto operator op(Symbolic<F> lhs, ConstSize<N> rhs) {                    \
        return make_index_fun([=](auto in) { return lhs.eval(in) op rhs; }); \
    }                                                                        \
    template<typename F>                                                     \
    auto operator op(size_t lhs, Symbolic<F> rhs) {                          \
        return make_index_fun([=](auto in) { return lhs op rhs.eval(in); }); \
    }                                                                        \
    template<typename F, size_t N>                                           \
    auto operator op(ConstSize<N> lhs, Symbolic<F> rhs) {                    \
        return make_index_fun([=](auto in) { return lhs op rhs.eval(in); }); \
    }

MAKE_INDEX_FUN_OP(+)
MAKE_INDEX_FUN_OP(-)
MAKE_INDEX_FUN_OP(*)
MAKE_INDEX_FUN_OP(/)

}  // namespace capybara