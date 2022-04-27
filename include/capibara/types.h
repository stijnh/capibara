#pragma once

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 2) || __GNUC__ > 4)
    #define CAPIBARA_INLINE __attribute__((always_inline)) inline
#elif EIGEN_COMP_MSVC || EIGEN_COMP_ICC
    #define CAPIBARA_INLINE __forceinline
#else
    #define CAPIBARA_INLINE inline
#endif

#include "axis.h"

namespace capibara {}  // namespace capibara