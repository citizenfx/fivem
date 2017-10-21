/*
* Define useful compiler-specific macros
* (C) 2016 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

/* This header is included in both C++ and C (via ffi.h) and should only
   contain macro definitions.
*/

#ifndef BOTAN_UTIL_COMPILER_FLAGS_H_
#define BOTAN_UTIL_COMPILER_FLAGS_H_

/* Should we use GCC-style inline assembler? */
#if !defined(BOTAN_USE_GCC_INLINE_ASM) && (defined(__GNUC__) || defined(__xlc__))
  #define BOTAN_USE_GCC_INLINE_ASM 1
#endif

/**
* Used to annotate API exports which are public and supported.
* These APIs will not be broken/removed unless strictly required for
* functionality or security, and only in new major versions.
* @param maj The major version this public API was released in
* @param min The minor version this public API was released in
*/
#define BOTAN_PUBLIC_API(maj,min) BOTAN_DLL

/**
* Used to annotate API exports which are public and can be used by
* applications if needed, but which are intentionally not documented,
* and which may change incompatibly in a future major version.
*/
#define BOTAN_UNSTABLE_API BOTAN_DLL

/**
* Used to annotate API exports which are exported but only for the
* purposes of testing. They should not be used by applications and
* may be removed or changed without notice.
*/
#define BOTAN_TEST_API BOTAN_DLL

/*
* Define BOTAN_GCC_VERSION
*/
#ifdef __GNUC__
  #define BOTAN_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__ * 10 + __GNUC_PATCHLEVEL__)
#else
  #define BOTAN_GCC_VERSION 0
#endif

/*
* Define BOTAN_CLANG_VERSION
*/
#ifdef __clang__
  #define BOTAN_CLANG_VERSION (__clang_major__ * 10 + __clang_minor__)
#else
  #define BOTAN_CLANG_VERSION 0
#endif

/*
* Define special macro when building under MSVC 2013 since there are
* many compiler workarounds required for that version.
*/
#if defined(_MSC_VER) && (_MSC_VER < 1900)
  #define BOTAN_BUILD_COMPILER_IS_MSVC_2013
#endif

/*
* Define BOTAN_FUNC_ISA
*/
#if defined(__GNUG__) || (BOTAN_CLANG_VERSION > 38)
  #define BOTAN_FUNC_ISA(isa) __attribute__ ((target(isa)))
#else
  #define BOTAN_FUNC_ISA(isa)
#endif

/*
* Define BOTAN_WARN_UNUSED_RESULT
*/
#if defined(__GNUG__) || defined(__clang__)
  #define BOTAN_WARN_UNUSED_RESULT __attribute__ ((warn_unused_result))
#else
  #define BOTAN_WARN_UNUSED_RESULT
#endif

/*
* Define BOTAN_MALLOC_FN
*/
#if defined(__GNUG__) || defined(__clang__)
  #define BOTAN_MALLOC_FN __attribute__ ((malloc))
#elif defined(_MSC_VER)
  #define BOTAN_MALLOC_FN __declspec(restrict)
#else
  #define BOTAN_MALLOC_FN
#endif

/*
* Define BOTAN_DEPRECATED
*/
#if !defined(BOTAN_NO_DEPRECATED_WARNINGS)

  #if defined(__clang__)
    #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated))

  #elif defined(_MSC_VER)
    #define BOTAN_DEPRECATED(msg) __declspec(deprecated(msg))

  #elif defined(__GNUG__)
    // msg supported since GCC 4.5, earliest we support is 4.8
    #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated(msg)))
  #endif

#endif

#if !defined(BOTAN_DEPRECATED)
  #define BOTAN_DEPRECATED(msg)
#endif

/*
* Define BOTAN_NORETURN
*/
#if !defined(BOTAN_NORETURN)

  #if defined (__clang__) || defined (__GNUG__)
    #define BOTAN_NORETURN __attribute__ ((__noreturn__))

  #elif defined (_MSC_VER)
    #define BOTAN_NORETURN __declspec(noreturn)

  #else
    #define BOTAN_NORETURN
  #endif

#endif

/*
* Define BOTAN_CURRENT_FUNCTION
*/
#if defined(_MSC_VER)
  #define BOTAN_CURRENT_FUNCTION __FUNCTION__
#else
  #define BOTAN_CURRENT_FUNCTION __func__
#endif

/*
* Define BOTAN_NOEXCEPT (for MSVC 2013)
*/
#if defined(BOTAN_BUILD_COMPILER_IS_MSVC_2013)
  // noexcept is not supported in VS 2013
  #include <yvals.h>
  #define BOTAN_NOEXCEPT _NOEXCEPT
#else
  #define BOTAN_NOEXCEPT noexcept
#endif

/*
* Define BOTAN_ALIGNAS (for MSVC 2013)
*/
#if defined(BOTAN_BUILD_COMPILER_IS_MSVC_2013)
  #define BOTAN_ALIGNAS(n) /**/
#else
  #define BOTAN_ALIGNAS(n) alignas(n)
#endif

/*
* Define BOTAN_PARALLEL_FOR
*/
#if !defined(BOTAN_PARALLEL_FOR)

#if defined(BOTAN_TARGET_HAS_CILKPLUS)
  #define BOTAN_PARALLEL_FOR _Cilk_for
#elif defined(BOTAN_TARGET_HAS_OPENMP)
  #define BOTAN_PARALLEL_FOR _Pragma("omp parallel for") for
#else
  #define BOTAN_PARALLEL_FOR for
#endif

#endif

/*
* Define BOTAN_PARALLEL_SIMD_FOR
*/
#if !defined(BOTAN_PARALLEL_SIMD_FOR)

#if defined(BOTAN_TARGET_HAS_CILKPLUS)
  #define BOTAN_PARALLEL_SIMD_FOR _Pragma("simd") for
#elif defined(BOTAN_TARGET_HAS_OPENMP)
  #define BOTAN_PARALLEL_SIMD_FOR _Pragma("omp simd") for
#elif defined(BOTAN_BUILD_COMPILER_IS_GCC) && (BOTAN_GCC_VERSION >= 490)
  #define BOTAN_PARALLEL_SIMD_FOR _Pragma("GCC ivdep") for
#else
  #define BOTAN_PARALLEL_SIMD_FOR for
#endif

#endif

/*
* Define BOTAN_PARALLEL_SPAWN
*/
#if !defined(BOTAN_PARALLEL_SPAWN)

#if defined(BOTAN_TARGET_HAS_CILKPLUS)
  #define BOTAN_PARALLEL_SPAWN _Cilk_spawn
#else
  #define BOTAN_PARALLEL_SPAWN
#endif

#endif

/*
* Define BOTAN_PARALLEL_SYNC
*/
#if !defined(BOTAN_PARALLEL_SYNC)

#if defined(BOTAN_TARGET_HAS_CILKPLUS)
  #define BOTAN_PARALLEL_SYNC _Cilk_sync
#else
  #define BOTAN_PARALLEL_SYNC BOTAN_FORCE_SEMICOLON
#endif

#endif

#endif
