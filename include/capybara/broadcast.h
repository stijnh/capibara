#pragma once

#include <sstream>

#include "dimensions.h"
#include "expr.h"
#include "forwards.h"
#include "functional.h"
#include "tuple.h"

namespace capybara {

namespace detail {

    template<typename E, typename D>
    void throw_broadcast_failed(const E& lhs, const D& rhs) {
        std::stringstream ss;
        ss << "cannot broadcast dimensions " << lhs.dims() << " to " << rhs;
        throw std::runtime_error(ss.str());
    }

    template<typename D, typename E, typename = void>
    struct BroadcastHelper {
        static constexpr size_t k = D::rank - E::rank;
        using type = BroadcastExpr<k, E>;

        static bool valid(const E& expr, const D& dims) {
            return seq::all_n<E::rank>(
                [&](auto i) { return expr.dim(i) == dims[k + i]; });
        }

        static type call(E&& expr, const D& dims) {
            if (!valid(expr, dims)) {
                throw_broadcast_failed(expr, dims);
            }

            std::array<index_t, k> prefix;
            seq::for_each_n<k>([&](auto i) { prefix[i] = dims[i]; });

            return {prefix, std::forward<E>(expr)};
        }
    };

    template<typename D, typename E>
    struct BroadcastHelper<D, E, enable_t<(D::rank == E::rank)>> {
        using type = E;

        static bool valid(const E& expr, const D& dims) {
            return seq::all_n<E::rank>(
                [&](auto i) { return expr.dim(i) == dims[i]; });
        }

        static type call(E&& expr, const D& dims) {
            if (!valid(expr, dims)) {
                throw_broadcast_failed(expr, dims);
            }

            return expr;
        }
    };

    template<typename D, typename E>
    struct BroadcastHelper<D, E, enable_t<(D::rank < E::rank)>> {
        using type = E;

        static bool valid(const E& expr, const D& dims) {
            return false;
        }

        static type call(E&& expr, const D& dims) {
            throw_broadcast_failed(expr, dims);
            while (true)
                ;  // above should throw
        }
    };

    template<typename Es, typename = void>
    struct LargestDimensions;

    template<typename E, typename... Es>
    struct LargestDimensions<
        Tuple<E, Es...>,
        enable_t<(
            is_expr<
                E> && E::rank >= LargestDimensions<Tuple<Es...>>::type::rank)>> {
        using type = typename E::Dims;

        static type call(const E& first, const Es&... rest) {
            return first.dims();
        }
    };

    template<typename E, typename... Es>
    struct LargestDimensions<
        Tuple<E, Es...>,
        enable_t<(
            !is_expr<
                E> || IntoExpr<E>::rank < LargestDimensions<Tuple<Es...>>::type::rank)>> {
        using Base = LargestDimensions<Tuple<Es...>>;
        using type = typename Base::type;

        static type call(const E& first, const Es&... rest) {
            return Base::call(rest...);
        }
    };

    template<>
    struct LargestDimensions<Tuple<>> {
        using type = Dimensions<>;

        static type call() {
            return {};
        }
    };
}  // namespace detail

template<typename... Es>
using LargestDims = typename detail::LargestDimensions<Tuple<Es...>>::type;

template<typename... Es>
LargestDims<Es...> largest_dims(const Es&... exprs) {
    return detail::LargestDimensions<Tuple<Es...>>::call(exprs...);
}

template<typename E, typename D>
using Broadcast = typename detail::BroadcastHelper<D, IntoExpr<E>>::type;

template<typename E, typename... Es>
using BroadcastTo = Broadcast<E, LargestDims<decay_t<Es>...>>;

template<typename E, size_t n>
using BroadcastToRank = Broadcast<E, DimensionsN<n>>;

template<typename E, typename D>
Broadcast<E, D> broadcast(E&& expr, const D& dims) {
    return detail::BroadcastHelper<D, IntoExpr<E>>::call(
        into_expr(std::forward<E>(expr)),
        dims);
}

template<size_t k, typename C>
struct BroadcastCursor;

template<size_t k, typename E, size_t axis>
struct ExprConstDim<BroadcastCursor<k, E>, axis, enable_t<axis >= k>>:
    ExprConstDim<E, axis - k> {};

template<size_t k, typename E, size_t axis>
struct ExprConstStride<BroadcastCursor<k, E>, axis, enable_t<axis >= k>>:
    ExprConstStride<E, axis - k> {};

template<size_t k, typename E, size_t axis>
struct ExprConstStride<BroadcastCursor<k, E>, axis, enable_t<(axis < k)>>:
    ConstIndex<0> {};

template<size_t k, typename E>
struct ExprTraits<BroadcastExpr<k, E>> {
    static constexpr size_t rank = k + ExprTraits<E>::rank;
    static constexpr bool is_view = ExprTraits<E>::is_view;
    static constexpr bool is_readable = ExprTraits<E>::is_readable;
    static constexpr bool is_writeable = ExprTraits<E>::is_writable;
    using Value = typename ExprTraits<E>::Value;
    using Cursor = BroadcastCursor<k, typename E::Cursor>;
};

template<size_t k, typename E>
struct BroadcastExpr: Expr<BroadcastExpr<k, E>> {
    template<size_t, typename>
    friend struct BroadcastCursor;

    BroadcastExpr(std::array<index_t, k> dims, E expr) :
        dims_(std::move(dims)),
        nested_(std::move(expr)) {}

    template<typename Axis>
    index_t stride_impl(Axis axis) const {
        if (axis < const_index<k>) {
            return 0;
        } else {
            return nested_.stride(into_index<E::rank>(axis - const_index<k>));
        }
    }

    template<typename Axis>
    index_t dim_impl(Axis axis) const {
        if (axis < const_index<k>) {
            return dims_[axis];
        } else {
            return nested_.dim(into_index<E::rank>(axis - const_index<k>));
        }
    }

  private:
    std::array<index_t, k> dims_;
    E nested_;
};

template<size_t k, typename C>
struct BroadcastCursor {
    using Value = decltype(std::declval<C>().eval());

    template<typename E>
    BroadcastCursor(const BroadcastExpr<k, E>& e) :
        cursor_(e.nested_.cursor()) {}

    template<typename E>
    BroadcastCursor(BroadcastExpr<k, E>& e) : cursor_(e.nested_.cursor()) {}

    template<typename Axis, typename Steps>
    void advance(Axis axis, Steps steps) {
        if (axis >= const_index<k>) {
            cursor_.advance(axis - const_index<k>, steps);
        }
    }

    template<typename T>
    void store(T value) {
        return cursor_.store(std::move(value));
    }

    Value eval() {
        return cursor_.eval();
    }

  private:
    C cursor_;
};

}  // namespace capybara