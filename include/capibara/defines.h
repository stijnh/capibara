#pragma once

#if defined(__GNUC__)
    #define CAPIBARA_GCC_VERSION \
        ((__GNUC__ * 100 + __GNUC_MINOR__) * 100 + __GNUC_PATCHLEVEL__)
#else
    #define CAPIBARA_GCC_VERSION 0
#endif

#if defined(_MSC_VER)
    #define CAPIBARA_MSVC_VERSION _MSC_VER
#else
    #define CAPIBARA_MSVC_VERSION 0
#endif

#if defined(__INTEL_COMPILER)
    #define CAPIBARA_ICC_VERSION __INTEL_COMPILER
#else
    #define CAPIBARA_ICC_VERSION 0
#endif

#if CAPIBARA_GCC_VERSION >= 40200
    #define CAPIBARA_INLINE __attribute__((always_inline)) inline
#elif CAPIBARA_MSVC_VERSION > 0 || CAPIBARA_ICC_VERSION > 0
    #define CAPIBARA_INLINE __forceinline
#else
    #define CAPIBARA_INLINE inline
#endif

#if CAPIBARA_GCC_VERSION >= 40500
    #define CAPIBARA_UNREACHABLE __builtin_unreachable()
#else
    #define CAPIBARA_UNREACHABLE \
        do {                     \
        } while (0)
#endif