#pragma once

#if defined(__GNUC__)
    #define CAPYBARA_GCC_VERSION \
        ((__GNUC__ * 100 + __GNUC_MINOR__) * 100 + __GNUC_PATCHLEVEL__)
#else
    #define CAPYBARA_GCC_VERSION 0
#endif

#if defined(_MSC_VER)
    #define CAPYBARA_MSVC_VERSION _MSC_VER
#else
    #define CAPYBARA_MSVC_VERSION 0
#endif

#if defined(__INTEL_COMPILER)
    #define CAPYBARA_ICC_VERSION __INTEL_COMPILER
#else
    #define CAPYBARA_ICC_VERSION 0
#endif

#if CAPYBARA_GCC_VERSION >= 40200
    #define CAPYBARA_INLINE __attribute__((always_inline)) inline
#elif CAPYBARA_MSVC_VERSION > 0 || CAPYBARA_ICC_VERSION > 0
    #define CAPYBARA_INLINE __forceinline
#else
    #define CAPYBARA_INLINE inline
#endif

#define CAPYBARA_NOINLINE __attribute__((noinline)) inline

#if CAPYBARA_GCC_VERSION >= 40500
    #define CAPYBARA_UNREACHABLE __builtin_unreachable()
#else
    #define CAPYBARA_UNREACHABLE \
        do {                     \
        } while (1)
#endif

#define CAPYBARA_TODO(msg)             \
    do {                               \
        throw std::runtime_error(msg); \
        CAPYBARA_UNREACHABLE;          \
    } while (0)
