#pragma once

#include <array>
#include <sstream>
#include <tuple>

#include "const_int.h"
#include "types.h"

namespace capybara {
template<typename... Ts>
struct Tuple;

namespace detail {
    template<index_t i, typename... Ts>
    struct TupleElementAtHelper;

    template<typename First, typename... Rest>
    struct TupleElementAtHelper<0, First, Rest...> {
        using type = First;
    };

    template<index_t i, typename First, typename... Rest>
    struct TupleElementAtHelper<i, First, Rest...>:
        TupleElementAtHelper<i - 1, Rest...> {};

    template<index_t i, typename... Ts>
    using TupleElementAt = typename TupleElementAtHelper<i, Ts...>::type;

    template<typename... Ts>
    struct TupleElement;

    template<typename T, typename... Rest>
    struct TupleElement<T, Rest...> {
        template<typename... Rs>
        friend struct TupleElement;

        CAPYBARA_INLINE
        TupleElement() = default;

        CAPYBARA_INLINE
        TupleElement(T first, Rest... rest) :
            head_(std::move(first)),
            tail_(std::move(rest)...) {
            //
        }

        template<typename R>
        CAPYBARA_INLINE TupleElement(const R* ptr) :
            head_(*ptr),
            tail_(ptr + 1) {}

        template<typename F>
        CAPYBARA_INLINE void for_each(F fun) const {
            fun(head_);
            tail_.for_each(fun);
        }

        template<typename F>
        CAPYBARA_INLINE void for_each(F fun) {
            fun(head_);
            tail_.for_each(fun);
        }

        template<typename F, typename R>
        CAPYBARA_INLINE auto fold(F fun, R initial) const {
            return tail_.fold(fun, fun(std::move(initial), head_));
        }

        template<typename R, typename... Rs>
        CAPYBARA_INLINE bool equals(const TupleElement<R, Rs...>& rhs) const {
            if (rhs.head_ != head_) {
                return false;
            }

            return tail_.equals(rhs.tail_);
        }

        CAPYBARA_INLINE
        bool equals(const TupleElement<>& rhs) const {
            return false;
        }

        template<typename R>
        CAPYBARA_INLINE bool equals(const R* iter) const {
            if (head_ != *iter) {
                return false;
            }

            return tail_.equals(iter + 1);
        }

        template<index_t current, size_t rank, typename R, typename F>
        CAPYBARA_INLINE R visit_dynamic(DynIndex<rank> index, F fun) {
            if (current == index) {
                return fun(head_);
            } else {
                return tail_.template visit_dynamic<current + 1, rank, R>(
                    index,
                    fun);
            }
        }

        template<index_t current, size_t rank, typename R, typename F>
        CAPYBARA_INLINE R visit_dynamic(DynIndex<rank> index, F fun) const {
            if (current == index) {
                return fun(head_);
            } else {
                return tail_.template visit_dynamic<current + 1, rank, R>(
                    index,
                    fun);
            }
        }

        template<index_t current, size_t rank, typename R>
        CAPYBARA_INLINE R get_dynamic(DynIndex<rank> index) const {
            if (current == index) {
                return (R)head_;
            } else {
                return tail_.template get_dynamic<current + 1, rank, R>(index);
            }
        }

        template<index_t current, size_t rank, typename R>
        CAPYBARA_INLINE void set_dynamic(DynIndex<rank> index, R value) {
            if (current == index) {
                head_ = value;
            } else {
                return tail_.template set_dynamic<current + 1, rank, R>(
                    index,
                    std::move(value));
            }
        }

        CAPYBARA_INLINE
        const T& get_ref(ConstIndex<0>) const {
            return head_;
        }

        CAPYBARA_INLINE
        T& get_ref(ConstIndex<0>) {
            return head_;
        }

        template<index_t current>
        CAPYBARA_INLINE const TupleElementAt<current, T, Rest...>&
        get_ref(ConstIndex<current>) const {
            return tail_.get_ref(ConstIndex<current - 1> {});
        }

        template<index_t current>
        CAPYBARA_INLINE TupleElementAt<current, T, Rest...>&
        get_ref(ConstIndex<current>) {
            return tail_.get_ref(ConstIndex<current - 1> {});
        }

        void print(std::ostream& out) const {
            out << head_;

            if (sizeof...(Rest) > 0) {
                out << ", ";
                tail_.print(out);
            }
        }

      private:
        T head_;
        TupleElement<Rest...> tail_;
    };

    template<>
    struct TupleElement<> {
        CAPYBARA_INLINE
        TupleElement() = default;

        template<typename R>
        CAPYBARA_INLINE TupleElement(const R* ptr) {}

        template<typename F>
        CAPYBARA_INLINE void for_each(F fun) const {}

        template<typename F, typename R>
        CAPYBARA_INLINE auto fold(F fun, R initial) const {
            return initial;
        }

        template<typename R, typename... Rs>
        CAPYBARA_INLINE bool equals(const TupleElement<R, Rs...>& rhs) const {
            return false;
        }

        CAPYBARA_INLINE
        bool equals(const TupleElement<>& rhs) const {
            return true;
        }

        template<typename R>
        CAPYBARA_INLINE bool equals(const R* iter) const {
            return true;
        }

        template<index_t current, size_t rank, typename R, typename F>
        CAPYBARA_INLINE R visit_dynamic(DynIndex<rank> index, F fun) const {
            throw std::runtime_error("internal error");
        }

        template<index_t current, size_t rank, typename R>
        CAPYBARA_INLINE R get_dynamic(DynIndex<rank> index) const {
            throw std::runtime_error("internal error");
        }

        template<index_t current, size_t rank, typename R>
        CAPYBARA_INLINE void set_dynamic(DynIndex<rank> index, R value) {
            throw std::runtime_error("internal error");
        }

        void print(std::ostream& out) const {}
    };

    template<typename R>
    const R* assert_array_length(const R* begin, const R* end, size_t n) {
        auto expected = n;
        auto gotten = std::distance(begin, end);

        if (cmp_not_equal(expected, gotten)) {
            std::stringstream ss;
            ss << "invalid length: expecting " << expected << " items, got "
               << gotten << " items";
            throw std::runtime_error(ss.str());
        }

        return begin;
    }

    template<
        typename Tuple,
        typename Indices = std::make_index_sequence<Tuple::rank>>
    struct TupleSequenceHelper;

    template<typename... Ts, size_t... indices>
    struct TupleSequenceHelper<Tuple<Ts...>, std::index_sequence<indices...>> {
        template<typename... Rs>
        static Tuple<Ts...> from_other_tuple(const Tuple<Rs...>& other) {
            return {other.get_ref(ConstIndex<indices> {})...};
        }

        template<typename... Rs>
        static Tuple<Ts...> from_std_tuple(const std::tuple<Rs...>& other) {
            return {std::get<indices>(other)...};
        }

        template<typename R>
        static std::array<R, sizeof...(Ts)>
        to_std_array(const Tuple<Ts...>& self) {
            return {self.get_ref(ConstIndex<indices> {})...};
        }

        static std::tuple<Ts...> to_std_tuple(const Tuple<Ts...>& self) {
            return {self.get_ref(ConstIndex<indices> {})...};
        }

        template<typename F>
        static Tuple<apply_t<F, const Ts&>...>
        map(F fun, const Tuple<Ts...>& self) {
            return {fun(self.get_ref(ConstIndex<indices> {}))...};
        }
    };

    template<typename... Ts>
    struct CommonTypeHelper {
        using type = void;
    };

    template<typename T>
    struct CommonTypeHelper<T> {
        using type = T;
    };

    template<typename T, typename... Ts>
    struct CommonTypeHelper<T, Ts...> {
        using type = typename std::
            common_type<T, typename CommonTypeHelper<Ts...>::type>::type;
    };
}  // namespace detail

template<size_t i, typename... Ts>
using TupleElementAt = detail::TupleElementAt<i, Ts...>;

template<typename... Ts>
using CommonType = typename detail::CommonTypeHelper<Ts...>::type;

template<typename... Ts>
struct Tuple {
    template<typename... Rs>
    friend struct Tuple;

    static constexpr size_t rank = sizeof...(Ts);
    using Self = Tuple<Ts...>;
    using Helper = detail::TupleSequenceHelper<Self>;

    Tuple() = default;
    Tuple(const Tuple& other) = default;

    template<typename = enable_t<sizeof...(Ts) == rank>>
    Tuple(Ts... items) : inner_(std::move(items)...) {
        //
    }

    template<typename R>
    Tuple(const R* begin, const R* end) :
        inner_(detail::assert_array_length(begin, end, rank)) {}

    template<typename R, size_t length>
    Tuple(const std::array<R, length>& items) :
        Tuple(items.begin(), items.end()) {}

    template<typename... Rs, typename = enable_t<sizeof...(Rs) == rank>>
    Tuple(const std::tuple<Rs...>& other) :
        Tuple(Helper::from_std_tuple(other)) {
        //
    }

    template<typename... Rs, typename = enable_t<sizeof...(Rs) == rank>>
    Tuple(const Tuple<Rs...>& other) : Tuple(Helper::from_other_tuple(other)) {
        //
    }

    template<index_t i>
    CAPYBARA_INLINE TupleElementAt<i, Ts...>&
    get_ref(ConstIndex<i> index = {}) {
        return inner_.get_ref(index);
    }

    template<index_t i>
    CAPYBARA_INLINE const TupleElementAt<i, Ts...>&
    get_ref(ConstIndex<i> index = {}) const {
        return inner_.get_ref(index);
    }

    template<
        typename R = void,
        index_t i,
        typename F,
        typename = enable_t<(i >= 0 && i < rank)>>
    CAPYBARA_INLINE R visit(ConstIndex<i> index, F fun) {
        return fun(get_ref(index));
    }

    template<
        typename R = void,
        index_t i,
        typename F,
        typename = enable_t<(i >= 0 && i < rank)>>
    CAPYBARA_INLINE R visit(ConstIndex<i> index, F fun) const {
        return fun(get_ref(index));
    }

    template<typename R, index_t i, typename = enable_t<(i >= 0 && i < rank)>>
    CAPYBARA_INLINE R get(ConstIndex<i> index) const {
        return get_ref(index);
    }

    template<index_t i, typename = enable_t<(i >= 0 && i < rank)>>
    CAPYBARA_INLINE void
    set(ConstIndex<i> index, TupleElementAt<i, Ts...> value) {
        get_ref(index) = std::move(value);
    }

    template<typename R = void, typename F>
    R visit(DynIndex<rank> index, F fun) {
        return inner_.template visit_dynamic<0, rank, R>(index, fun);
    }

    template<typename R = void, typename F>
    R visit(DynIndex<rank> index, F fun) const {
        return inner_.template visit_dynamic<0, rank, R>(index, fun);
    }

    template<typename R>
    R get(DynIndex<rank> index) const {
        return inner_.template get_dynamic<0, rank, R>(index);
    }

    template<typename R>
    void set(DynIndex<rank> index, R value) {
        inner_.template set_dynamic<0, rank, R>(index, value);
    }

    template<typename... Rs>
    bool operator==(const Tuple<Rs...>& other) const {
        return inner_.equals(other.inner_);
    }

    template<typename... Rs>
    bool operator!=(const Tuple<Rs...>& other) const {
        return !(*this == other);
    }

    template<typename R, size_t length>
    bool operator==(const std::array<R, length>& other) const {
        if (length != sizeof...(Ts)) {
            return false;
        }

        return inner_.equals(other.begin());
    }

    template<typename R, size_t length>
    bool operator!=(const std::array<R, length>& other) const {
        return !(*this == other);
    }

    template<typename R, size_t length>
    friend bool
    operator==(const std::array<R, length>& lhs, const Tuple<Ts...>& rhs) {
        return rhs == lhs;
    }

    template<typename R, size_t length>
    friend bool
    operator!=(const std::array<R, length>& lhs, const Tuple<Ts...>& rhs) {
        return !(lhs == rhs);
    }

    template<typename... Rs>
    bool operator==(const std::tuple<Rs...>& other) const {
        return *this == Tuple<Rs...> {other};
    }

    template<typename... Rs>
    bool operator!=(const std::tuple<Rs...>& other) const {
        return !(*this == other);
    }

    template<typename... Rs>
    friend bool
    operator==(const std::tuple<Rs...>& lhs, const Tuple<Ts...>& rhs) {
        return rhs == lhs;
    }

    template<typename... Rs>
    friend bool
    operator!=(const std::tuple<Rs...>& lhs, const Tuple<Ts...>& rhs) {
        return !(lhs == rhs);
    }

    CAPYBARA_INLINE
    static constexpr size_t size() {
        return sizeof...(Ts);
    }

    CAPYBARA_INLINE
    static constexpr bool empty() {
        return size() == 0;
    }

    template<typename R>
    CAPYBARA_INLINE std::array<R, rank> to_array() const {
        return Helper::template to_std_array<R>(*this);
    }

    template<typename R>
    CAPYBARA_INLINE operator std::array<R, rank>() const {
        return Helper::template to_std_array<R>(*this);
    }

    CAPYBARA_INLINE
    operator std::tuple<Ts...>() const {
        return Helper::template to_std_tuple(*this);
    }

    template<typename F>
    CAPYBARA_INLINE Tuple<apply_t<F, Ts&>...> map(F fun) const {
        return Helper::map(fun, *this);
    }

    template<typename F>
    CAPYBARA_INLINE void visit_all(F fun) const {
        return inner_.for_each(fun);
    }

    template<typename F, typename R>
    CAPYBARA_INLINE auto fold(R initial, F fun) const {
        return inner_.fold(fun, std::move(initial));
    }

    friend std::ostream& operator<<(std::ostream& out, const Self& self) {
        out << "{";
        self.inner_.print(out);
        out << "}";
        return out;
    }

  private:
    detail::TupleElement<Ts...> inner_;
};

template<typename... Ts>
Tuple<Ts...> into_tuple(Ts... item) {
    return {item...};
}

}  // namespace capybara

namespace std {
// These make structured binding possible
template<typename... Ts>
struct tuple_size<capybara::Tuple<Ts...>>: tuple_size<std::tuple<Ts...>> {};

template<size_t i, typename... Ts>
struct tuple_element<i, capybara::Tuple<Ts...>>:
    tuple_element<i, std::tuple<Ts...>> {};

template<size_t i, typename... Ts>
typename tuple_element<i, capybara::Tuple<Ts...>>::type&
get(capybara::Tuple<Ts...>& t) {
    return t.template get_ref<i>();
}

template<size_t i, typename... Ts>
const typename tuple_element<i, capybara::Tuple<Ts...>>::type&
get(const capybara::Tuple<Ts...>& t) {
    return t.template get_ref<i>();
}
}  // namespace std