#pragma once

#include <array>
#include <tuple>

#include "axis.h"
#include "const_int.h"

namespace capybara {
namespace hybrid_array {
    struct Dyn {};

    template<typename... Ts>
    struct List;

    template<typename T, typename Items>
    struct HybridArray;

    namespace detail {
        template<typename Items>
        struct LengthHelper;

        template<typename... Items>
        struct LengthHelper<List<Items...>> {
            static constexpr size_t value = sizeof...(Items);
        };

        template<typename Items>
        static constexpr size_t Length = LengthHelper<Items>::value;

        template<size_t I, typename Items>
        struct AtHelper {
            using type = void;
        };

        template<typename First, typename... Items>
        struct AtHelper<0, List<First, Items...>> {
            using type = First;
        };

        template<size_t I, typename First, typename... Items>
        struct AtHelper<I, List<First, Items...>>:
            AtHelper<I - 1, List<Items...>> {};

        template<size_t I, typename Items>
        using At = typename AtHelper<I, Items>::type;

        template<size_t I, size_t D, typename Items>
        struct StorageOffsetHelper;

        template<size_t D>
        struct StorageOffsetHelper<0, D, List<>> {
            static constexpr size_t value = D;
        };

        template<size_t D, typename... Items>
        struct StorageOffsetHelper<0, D, List<Dyn, Items...>> {
            static constexpr size_t value = D;
        };

        template<size_t D, typename... Items, typename T, T const_value>
        struct StorageOffsetHelper<
            0,
            D,
            List<ConstInt<T, const_value>, Items...>> {
            static constexpr size_t value = D;
        };

        template<size_t I, size_t D, typename... Items>
        struct StorageOffsetHelper<I, D, List<Dyn, Items...>>:
            StorageOffsetHelper<I - 1, D + 1, List<Items...>> {};

        template<size_t I, size_t D, typename... Items, typename T, T value>
        struct StorageOffsetHelper<I, D, List<ConstInt<T, value>, Items...>>:
            StorageOffsetHelper<I - 1, D, List<Items...>> {};

        template<size_t I, typename Items>
        static constexpr size_t storage_offset =
            StorageOffsetHelper<I, 0, Items>::value;

        template<typename T, typename Items>
        using Storage = std::array<T, storage_offset<Length<Items>, Items>>;

        template<size_t I, typename Items, typename Current = At<I, Items>>
        struct Access;

        template<size_t I, typename Items>
        struct Access<I, Items, Dyn> {
            template<typename T>
            CAPYBARA_INLINE static T get(const T* storage) {
                return storage[storage_offset<I, Items>];
            }

            template<typename T>
            CAPYBARA_INLINE static T
            get_dynamic(const T* storage, index_t index) {
                if (index == I) {
                    static constexpr size_t offset =
                        I - storage_offset<I, Items>;
                    return storage[index - offset];
                } else {
                    return Access<I + 1, Items>::get_dynamic(storage, index);
                }
            }

            template<typename T>
            CAPYBARA_INLINE static void set(T* storage, T value) {
                storage[storage_offset<I, Items>] = std::move(value);
            }

            template<typename T>
            CAPYBARA_INLINE static void
            set_dynamic(T* storage, index_t index, T input) {
                if (index == I) {
                    static constexpr size_t offset =
                        I - storage_offset<I, Items>;
                    storage[index - offset] = std::move(input);
                } else {
                    Access<I + 1, Items>::set_dynamic(storage, index, input);
                }
            }
        };

        template<size_t I, typename Items, typename T, T value>
        struct Access<I, Items, ConstInt<T, value>> {
            CAPYBARA_INLINE static ConstInt<T, value> get(const T* storage) {
                return {};
            }

            CAPYBARA_INLINE
            static T get_dynamic(const T* storage, index_t index) {
                if (index == I) {
                    return value;
                } else {
                    return Access<I + 1, Items>::get_dynamic(storage, index);
                }
            }

            CAPYBARA_INLINE
            static void set(const T* storage, T input) {
                if (value != input) {
                    throw std::runtime_error("invalid value given");
                }
            }

            CAPYBARA_INLINE
            static void set_dynamic(T* storage, index_t index, T input) {
                if (index == I) {
                    set(storage, input);
                } else {
                    Access<I + 1, Items>::set_dynamic(storage, index, input);
                }
            }
        };

        template<size_t I, typename Items>
        struct Access<I, Items, void> {
            template<typename T>
            CAPYBARA_INLINE static T get(const T* storage) {
                throw std::runtime_error("index out of bounds");
            }

            template<typename T>
            CAPYBARA_INLINE static T
            get_dynamic(const T* storage, index_t index) {
                throw std::runtime_error("index out of bounds");
            }

            template<typename T>
            CAPYBARA_INLINE static void set(const T* storage, T value) {
                throw std::runtime_error("index out of bounds");
            }

            template<typename T>
            CAPYBARA_INLINE static void
            set_dynamic(const T* storage, index_t index, T input) {
                throw std::runtime_error("index out of bounds");
            }
        };

        template<typename T, typename Items>
        struct AccessAxis {
            template<index_t I>
            CAPYBARA_INLINE static auto get(const T* storage, Axis<I>) {
                return Access<I, Items>::get(storage);
            }

            CAPYBARA_INLINE
            static T get(const T* storage, DynAxis<Length<Items>> axis) {
                return Access<0, Items>::get_dynamic(storage, axis);
            }

            template<index_t I>
            CAPYBARA_INLINE static void set(T* storage, Axis<I>, T value) {
                return Access<I, Items>::set(storage, value);
            }

            CAPYBARA_INLINE
            static void
            set_dynamic(T* storage, DynAxis<Length<Items>> axis, T value) {
                return Access<0, Items>::set_dynamic(storage, axis, value);
            }
        };

        template<typename T, typename Items>
        using Storage = std::array<T, storage_offset<Length<Items>, Items>>;

        template<size_t N, typename Output = List<>>
        struct RepeatDyn;

        template<size_t N, typename... Items>
        struct RepeatDyn<N, List<Items...>>:
            RepeatDyn<N - 1, List<Dyn, Items...>> {};

        template<typename... Items>
        struct RepeatDyn<0, List<Items...>> {
            using type = List<Items...>;
        };

        template<typename T, typename Input>
        struct ConvertItem;

        template<typename T>
        struct ConvertItem<T, T> {
            using type = T;
        };

        template<typename T, T value>
        struct ConvertItem<T, ConstInt<T, value>> {
            using type = ConstInt<T, value>;
        };

        template<typename T, typename... Inputs>
        struct Convert {
            using type =
                std::tuple<typename ConvertItem<T, decayed<Inputs>>::type...>;
        };

        template<typename T, size_t N>
        struct Convert<T, std::array<T, N>> {
            using type = typename RepeatDyn<N>::type;
        };

        template<typename T, typename Items>
        struct Convert<T, HybridArray<T, Items>> {
            using type = Items;
        };

        template<typename Items, size_t I = 0, size_t N = Length<Items>>
        struct Foreach {
            template<typename F>
            CAPYBARA_INLINE static void call(F fun) {
                fun(const_axis<I>);
                Foreach<Items, I + 1, N>::call(std::move(fun));
            }

            template<typename F, typename R, typename T>
            CAPYBARA_INLINE static auto
            fold(F fun, R initial, const T* storage) {
                auto result =
                    fun(Access<I, Items>::get(storage), std::move(initial));
                return Foreach<Items, I + 1, N>::fold(
                    std::move(fun),
                    std::move(result));
            }

            template<typename F, typename T>
            CAPYBARA_INLINE static auto each(F fun, const T* storage) {
                fun(Access<I, Items>::get(storage));
                return Foreach<Items, I + 1, N>::each(std::move(fun));
            }
        };

        template<typename Items, size_t N>
        struct Foreach<Items, N, N> {
            template<typename F>
            CAPYBARA_INLINE static void call(F fun) {}

            template<typename F, typename R, typename T>
            CAPYBARA_INLINE static auto
            fold(F fun, R initial, const T* storage) {
                return initial;
            }

            template<typename F, typename T>
            CAPYBARA_INLINE static auto each(F fun, const T* storage) {}
        };

        template<typename T, typename Item>
        struct TupleElement {
            using type = Dyn;
        };

        template<typename T, typename R, R value>
        struct TupleElement<T, ConstInt<R, value>> {
            static_assert(in_range<T>(value), "value out of range for type");

            using type = ConstInt<T, (T)value>;
        };

        template<typename T, typename... Inputs>
        struct Builder {
            using type =
                List<typename TupleElement<T, decayed<Inputs>>::type...>;

            static std::tuple<Inputs...> call(Inputs... inputs) {
                return {inputs...};
            }
        };

        template<typename T, size_t N>
        struct Builder<T, std::array<T, N>> {
            using type = typename RepeatDyn<N>::type;

            static std::array<T, N> call(std::array<T, N> input) {
                return input;
            }
        };

        template<typename T, typename Items>
        struct Builder<T, HybridArray<T, Items>> {
            using type = Items;

            static HybridArray<T, Items> call(HybridArray<T, Items> input) {
                return input;
            }
        };

        template<typename T, typename... Items>
        struct Builder<T, std::tuple<Items...>> {
            using type =
                List<typename TupleElement<T, decayed<Items>>::type...>;

            static std::tuple<Items...> call(std::tuple<Items...> input) {
                return input;
            }
        };

    }  // namespace detail

    template<typename T, typename Storage>
    struct HybridArray {
        using Value = T;
        using Items = Storage;
        static constexpr size_t rank = detail::Length<Items>;

        HybridArray() = default;
        HybridArray(const HybridArray&) = default;

        template<typename OtherItems>
        HybridArray(const HybridArray<Value, OtherItems>& other) {
            if (other.size() != rank) {
                throw std::runtime_error("invalid number of values given");
            }

            detail::Foreach<Items>::call([&](auto i) { set(i, other.get(i)); });
        }

        HybridArray(const Value* begin, const Value* end) {
            if (std::distance(begin, end) != rank) {
                throw std::runtime_error("invalid number of values given");
            }

            detail::Foreach<Items>::call([&](auto i) { set(i, *(begin + i)); });
        }

        template<size_t N>
        HybridArray(const std::array<Value, N>& input) :
            HybridArray(input.begin(), input.end()) {}

        HybridArray(std::initializer_list<Value> input) :
            HybridArray(input.begin(), input.end()) {}

        template<typename... Ts>
        HybridArray(const std::tuple<Ts...>& input) {
            if (sizeof...(Ts) != rank) {
                throw std::runtime_error("invalid number of values given");
            }

            detail::Foreach<Items>::call([&](auto axis) {
                set(axis, std::get<decltype(axis)::value>(input));
            });
        }

        CAPYBARA_INLINE
        HybridArray& operator=(const HybridArray&) = default;

        template<typename OtherItems>
        CAPYBARA_INLINE HybridArray&
        operator=(const HybridArray<Value, OtherItems>& other) {
            *this = HybridArray(other);
            return *this;
        }

        template<typename... Ts>
        CAPYBARA_INLINE HybridArray& operator=(std::tuple<Ts...> input) {
            *this = HybridArray(input);
            return *this;
        }

        template<typename OtherItems>
        CAPYBARA_INLINE HybridArray&
        operator=(const std::array<Value, rank>& other) {
            *this = HybridArray(other);
            return *this;
        }

        template<size_t start = 0, size_t end = rank, typename F>
        void for_each(F fun) const {
            detail::Foreach<Items, start, end>::call(fun);
        }

        template<size_t start = 0, size_t end = rank, typename F, typename R>
        auto fold(F fun, R initial) const {
            return detail::Foreach<Items, start, end>::fold(
                fun,
                std::move(initial));
        }

        CAPYBARA_INLINE
        constexpr size_t size() const {
            return rank;
        }

        template<typename Axis>
        CAPYBARA_INLINE void set(Axis axis, T value) {
            detail::AccessAxis<T, Items>::set(
                storage_.data(),
                into_axis<rank>(axis),
                value);
        }

        template<typename Axis>
        CAPYBARA_INLINE auto get(Axis axis) const {
            return detail::AccessAxis<T, Items>::get(
                storage_.data(),
                into_axis<rank>(axis));
        }

        template<typename Axis>
        CAPYBARA_INLINE auto operator[](Axis axis) const {
            return get(axis);
        }

        template<typename OtherItems>
        bool operator==(const HybridArray<Value, OtherItems>& rhs) const {
            if (size() != rhs.size()) {
                return false;
            }

            for (size_t i = 0; i < size(); i++) {
                if (get(i) != rhs.get(i)) {
                    return false;
                }
            }

            return true;
        }

        template<typename OtherItems>
        bool operator!=(const HybridArray<Value, OtherItems>& rhs) const {
            return !(*this == rhs);
        }

        std::array<Value, rank> to_array() const {
            std::array<Value, rank> output;

            detail::Foreach<Items>::call([&](auto i) { output[i] = get(i); });

            return output;
        }

      private:
        detail::Storage<Value, Items> storage_ = {};
    };

}  // namespace hybrid_array

using hybrid_array::Dyn;
using hybrid_array::HybridArray;

template<typename T, size_t N>
using HybridArrayN =
    HybridArray<T, typename hybrid_array::detail::RepeatDyn<N>::type>;

template<typename T, typename... Ts>
HybridArray<T, typename hybrid_array::detail::Builder<T, decayed<Ts>...>::type>
make_hybrid_array(Ts&&... inputs) {
    return hybrid_array::detail::Builder<T, decayed<Ts>...>::call(inputs...);
}
}  // namespace capybara