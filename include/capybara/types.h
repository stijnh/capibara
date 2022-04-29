#pragma once

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
    #define CAPYBARA_INLINE __attribute__((always_inline)) inline
#elif EIGEN_COMP_MSVC || EIGEN_COMP_ICC
    #define CAPYBARA_INLINE __forceinline
#else
    #define CAPYBARA_INLINE inline
#endif

#include "axis.h"

namespace capybara {}  // namespace capybara