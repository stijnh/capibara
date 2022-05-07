#pragma once

#include "hybrid_array.h"
#include "types.h"

namespace capybara {

template<typename... Ts>
using Dimensions = HybridArray<index_t, hybrid_array::List<Ts...>>;

template<size_t N>
using DimensionsN = HybridArrayN<index_t, N>;

}  // namespace capybara