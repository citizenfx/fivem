/*
* Botan 2.0.1 Amalgamation
* (C) 1999-2013,2014,2015,2016 Jack Lloyd and others
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_AMALGAMATION_H__
#define BOTAN_AMALGAMATION_H__

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <exception>
#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <istream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <ostream>
#include <set>
#include <sstream>
#include <stddef.h>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

/*
* This file was automatically generated running
* 'configure.py --amalgamation --disable-modules=win32_stats --cc=msvc'
*
* Target
*  - Compiler: cl  /MD /bigobj /EHs /GR /O2
*  - Arch: x86_64/x86_64
*  - OS: windows
*/

#define BOTAN_VERSION_MAJOR 2
#define BOTAN_VERSION_MINOR 0
#define BOTAN_VERSION_PATCH 1
#define BOTAN_VERSION_DATESTAMP 0

#define BOTAN_VERSION_RELEASE_TYPE "unreleased"

#define BOTAN_VERSION_VC_REVISION "git:55a1d935e736716480407378565939b9e6c829b9"

#define BOTAN_DISTRIBUTION_INFO "unspecified"

/* How many bits per limb in a BigInt */
#define BOTAN_MP_WORD_BITS 64


#define BOTAN_INSTALL_PREFIX R"(c:\Botan)"
#define BOTAN_INSTALL_HEADER_DIR "include/botan-2"
#define BOTAN_INSTALL_LIB_DIR "lib"
#define BOTAN_LIB_LINK "advapi32.lib"

#ifndef BOTAN_DLL
  #define BOTAN_DLL __declspec(dllimport)
#endif

/* Target identification and feature test macros */
#define BOTAN_TARGET_OS_IS_WINDOWS
#define BOTAN_TARGET_OS_TYPE_IS_WINDOWS
#define BOTAN_TARGET_OS_HAS_CRYPTGENRANDOM
#define BOTAN_TARGET_OS_HAS_FILESYSTEM
#define BOTAN_TARGET_OS_HAS_GMTIME_S
#define BOTAN_TARGET_OS_HAS_LOADLIBRARY
#define BOTAN_TARGET_OS_HAS_MKGMTIME
#define BOTAN_TARGET_OS_HAS_QUERY_PERF_COUNTER
#define BOTAN_TARGET_OS_HAS_RTLSECUREZEROMEMORY
#define BOTAN_TARGET_OS_HAS_STL_FILESYSTEM_MSVC
#define BOTAN_TARGET_OS_HAS_THREADS
#define BOTAN_TARGET_OS_HAS_VIRTUAL_LOCK

#define BOTAN_TARGET_ARCH_IS_X86_64
#define BOTAN_TARGET_SUPPORTS_AESNI
#define BOTAN_TARGET_SUPPORTS_AVX2
#define BOTAN_TARGET_SUPPORTS_BMI2
#define BOTAN_TARGET_SUPPORTS_CLMUL
#define BOTAN_TARGET_SUPPORTS_RDRAND
#define BOTAN_TARGET_SUPPORTS_RDSEED
#define BOTAN_TARGET_SUPPORTS_SHA
#define BOTAN_TARGET_SUPPORTS_SSE2
#define BOTAN_TARGET_SUPPORTS_SSE41
#define BOTAN_TARGET_SUPPORTS_SSE42
#define BOTAN_TARGET_SUPPORTS_SSSE3
#define BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN
#define BOTAN_TARGET_CPU_IS_X86_FAMILY
#define BOTAN_TARGET_CPU_NATIVE_WORD_SIZE 64
#define BOTAN_TARGET_CPU_HAS_NATIVE_64BIT
#define BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK 1

#define BOTAN_BUILD_COMPILER_IS_MSVC

/*
* Module availability definitions
*/
#define BOTAN_HAS_ADLER32 20131128
#define BOTAN_HAS_AEAD_CCM 20131128
#define BOTAN_HAS_AEAD_CHACHA20_POLY1305 20141228
#define BOTAN_HAS_AEAD_EAX 20131128
#define BOTAN_HAS_AEAD_GCM 20131128
#define BOTAN_HAS_AEAD_MODES 20131128
#define BOTAN_HAS_AEAD_OCB 20131128
#define BOTAN_HAS_AEAD_SIV 20131202
#define BOTAN_HAS_AES 20131128
#define BOTAN_HAS_AES_NI 20131128
#define BOTAN_HAS_AES_SSSE3 20131128
#define BOTAN_HAS_ANSI_X919_MAC 20131128
#define BOTAN_HAS_ASN1 20161102
#define BOTAN_HAS_AUTO_RNG 20161126
#define BOTAN_HAS_AUTO_SEEDING_RNG 20160821
#define BOTAN_HAS_BASE64_CODEC 20131128
#define BOTAN_HAS_BCRYPT 20131128
#define BOTAN_HAS_BIGINT 20131128
#define BOTAN_HAS_BIGINT_MP 20151225
#define BOTAN_HAS_BLAKE2B 20130131
#define BOTAN_HAS_BLOCK_CIPHER 20131128
#define BOTAN_HAS_BLOWFISH 20131128
#define BOTAN_HAS_CAMELLIA 20150922
#define BOTAN_HAS_CASCADE 20131128
#define BOTAN_HAS_CAST 20131128
#define BOTAN_HAS_CBC_MAC 20131128
#define BOTAN_HAS_CECPQ1 20161116
#define BOTAN_HAS_CERTSTOR_SQL 20160818
#define BOTAN_HAS_CHACHA 20140103
#define BOTAN_HAS_CHACHA_SSE2 20160831
#define BOTAN_HAS_CIPHER_MODE_PADDING 20131128
#define BOTAN_HAS_CMAC 20131128
#define BOTAN_HAS_CODEC_FILTERS 20131128
#define BOTAN_HAS_COMB4P 20131128
#define BOTAN_HAS_CRC24 20131128
#define BOTAN_HAS_CRC32 20131128
#define BOTAN_HAS_CRYPTO_BOX 20131128
#define BOTAN_HAS_CTR_BE 20131128
#define BOTAN_HAS_CURVE_25519 20141227
#define BOTAN_HAS_DES 20131128
#define BOTAN_HAS_DIFFIE_HELLMAN 20131128
#define BOTAN_HAS_DLIES 20160713
#define BOTAN_HAS_DL_GROUP 20131128
#define BOTAN_HAS_DL_PUBLIC_KEY_FAMILY 20131128
#define BOTAN_HAS_DSA 20131128
#define BOTAN_HAS_ECC_GROUP 20131128
#define BOTAN_HAS_ECC_PUBLIC_KEY_CRYPTO 20131128
#define BOTAN_HAS_ECDH 20131128
#define BOTAN_HAS_ECDSA 20131128
#define BOTAN_HAS_ECGDSA 20160301
#define BOTAN_HAS_ECIES 20160128
#define BOTAN_HAS_ECKCDSA 20160413
#define BOTAN_HAS_EC_CURVE_GFP 20131128
#define BOTAN_HAS_ELGAMAL 20131128
#define BOTAN_HAS_EME_OAEP 20140118
#define BOTAN_HAS_EME_PKCS1v15 20131128
#define BOTAN_HAS_EME_RAW 20150313
#define BOTAN_HAS_EMSA1 20131128
#define BOTAN_HAS_EMSA_PKCS1 20140118
#define BOTAN_HAS_EMSA_PSSR 20131128
#define BOTAN_HAS_EMSA_RAW 20131128
#define BOTAN_HAS_EMSA_X931 20140118
#define BOTAN_HAS_ENTROPY_SOURCE 20151120
#define BOTAN_HAS_ENTROPY_SRC_CAPI 20131128
#define BOTAN_HAS_ENTROPY_SRC_RDRAND 20131128
#define BOTAN_HAS_ENTROPY_SRC_RDSEED 20151218
#define BOTAN_HAS_FFI 20151015
#define BOTAN_HAS_FILTERS 20160415
#define BOTAN_HAS_FPE_FE1 20131128
#define BOTAN_HAS_GCM_CLMUL 20131227
#define BOTAN_HAS_GMAC 20160207
#define BOTAN_HAS_GOST_28147_89 20131128
#define BOTAN_HAS_GOST_34_10_2001 20131128
#define BOTAN_HAS_GOST_34_11 20131128
#define BOTAN_HAS_HASH_ID 20131128
#define BOTAN_HAS_HEX_CODEC 20131128
#define BOTAN_HAS_HKDF 20131128
#define BOTAN_HAS_HMAC 20131128
#define BOTAN_HAS_HMAC_DRBG 20140319
#define BOTAN_HAS_HTTP_UTIL 20131128
#define BOTAN_HAS_IDEA 20131128
#define BOTAN_HAS_IDEA_SSE2 20131128
#define BOTAN_HAS_ISO_9796 20161121
#define BOTAN_HAS_KASUMI 20131128
#define BOTAN_HAS_KDF1 20131128
#define BOTAN_HAS_KDF1_18033 20160128
#define BOTAN_HAS_KDF2 20131128
#define BOTAN_HAS_KDF_BASE 20131128
#define BOTAN_HAS_KECCAK 20131128
#define BOTAN_HAS_KEYPAIR_TESTING 20131128
#define BOTAN_HAS_LION 20131128
#define BOTAN_HAS_LOCKING_ALLOCATOR 20131128
#define BOTAN_HAS_MAC 20150626
#define BOTAN_HAS_MCEIES 20150706
#define BOTAN_HAS_MCELIECE 20150922
#define BOTAN_HAS_MD4 20131128
#define BOTAN_HAS_MD5 20131128
#define BOTAN_HAS_MDX_HASH_FUNCTION 20131128
#define BOTAN_HAS_MGF1 20140118
#define BOTAN_HAS_MISTY1 20131128
#define BOTAN_HAS_MODES 20150626
#define BOTAN_HAS_MODE_CBC 20131128
#define BOTAN_HAS_MODE_CFB 20131128
#define BOTAN_HAS_MODE_XTS 20131128
#define BOTAN_HAS_NEWHOPE 20161018
#define BOTAN_HAS_NOEKEON 20131128
#define BOTAN_HAS_NOEKEON_SIMD 20160903
#define BOTAN_HAS_NUMBERTHEORY 20131128
#define BOTAN_HAS_OCSP 20161118
#define BOTAN_HAS_OFB 20131128
#define BOTAN_HAS_PACKAGE_TRANSFORM 20131128
#define BOTAN_HAS_PARALLEL_HASH 20131128
#define BOTAN_HAS_PASSHASH9 20131128
#define BOTAN_HAS_PBKDF 20150626
#define BOTAN_HAS_PBKDF1 20131128
#define BOTAN_HAS_PBKDF2 20131128
#define BOTAN_HAS_PEM_CODEC 20131128
#define BOTAN_HAS_PKCS5_PBES2 20141119
#define BOTAN_HAS_PK_PADDING 20131128
#define BOTAN_HAS_POLY1305 20141227
#define BOTAN_HAS_PUBLIC_KEY_CRYPTO 20131128
#define BOTAN_HAS_RC4 20131128
#define BOTAN_HAS_RDRAND_RNG 20160619
#define BOTAN_HAS_RFC3394_KEYWRAP 20131128
#define BOTAN_HAS_RFC6979_GENERATOR 20140321
#define BOTAN_HAS_RIPEMD_160 20131128
#define BOTAN_HAS_RSA 20160730
#define BOTAN_HAS_SALSA20 20131128
#define BOTAN_HAS_SEED 20131128
#define BOTAN_HAS_SERPENT 20131128
#define BOTAN_HAS_SERPENT_SIMD 20160903
#define BOTAN_HAS_SHA1 20131128
#define BOTAN_HAS_SHA1_SSE2 20160803
#define BOTAN_HAS_SHA2_32 20131128
#define BOTAN_HAS_SHA2_64 20131128
#define BOTAN_HAS_SHA3 20161018
#define BOTAN_HAS_SHAKE 20161009
#define BOTAN_HAS_SHAKE_CIPHER 20161018
#define BOTAN_HAS_SIMD_32 20131128
#define BOTAN_HAS_SIPHASH 20150110
#define BOTAN_HAS_SKEIN_512 20131128
#define BOTAN_HAS_SP800_108 20160128
#define BOTAN_HAS_SP800_56C 20160211
#define BOTAN_HAS_SRP6 20161017
#define BOTAN_HAS_STATEFUL_RNG 20160819
#define BOTAN_HAS_STREAM_CIPHER 20131128
#define BOTAN_HAS_SYSTEM_RNG 20141202
#define BOTAN_HAS_THREEFISH_512 20131224
#define BOTAN_HAS_THREEFISH_512_AVX2 20160903
#define BOTAN_HAS_THRESHOLD_SECRET_SHARING 20131128
#define BOTAN_HAS_TIGER 20131128
#define BOTAN_HAS_TLS 20150319
#define BOTAN_HAS_TLS_CBC 20161008
#define BOTAN_HAS_TLS_SESSION_MANAGER_SQL_DB 20141219
#define BOTAN_HAS_TLS_V10_PRF 20131128
#define BOTAN_HAS_TLS_V12_PRF 20131128
#define BOTAN_HAS_TWOFISH 20131128
#define BOTAN_HAS_UTIL_FUNCTIONS 20161127
#define BOTAN_HAS_WHIRLPOOL 20131128
#define BOTAN_HAS_X509_CERTIFICATES 20151023
#define BOTAN_HAS_X942_PRF 20131128
#define BOTAN_HAS_XMSS 20161008
#define BOTAN_HAS_XTEA 20131128

/*
* Local/misc configuration options (if any) follow
*/



/*
* Things you can edit (but probably shouldn't)
*/

/* How much to allocate for a buffer of no particular size */
#define BOTAN_DEFAULT_BUFFER_SIZE 1024

/* Minimum and maximum sizes to allocate out of the mlock pool (bytes)
   Default min is 16 as smaller values are easily bruteforceable and thus
   likely not cryptographic keys.
*/
#define BOTAN_MLOCK_ALLOCATOR_MIN_ALLOCATION 16
#define BOTAN_MLOCK_ALLOCATOR_MAX_ALLOCATION 128

/*
* Total maximum amount of RAM (in KiB) we will lock into memory, even
* if the OS would let us lock more
*/
#define BOTAN_MLOCK_ALLOCATOR_MAX_LOCKED_KB 512

/*
* If enabled uses memset via volatile function pointer to zero memory,
* otherwise does a byte at a time write via a volatile pointer.
*/
#define BOTAN_USE_VOLATILE_MEMSET_FOR_ZERO 1

/*
* If enabled the ECC implementation will use Montgomery ladder
* instead of a fixed window implementation.
*/
#define BOTAN_POINTGFP_BLINDED_MULTIPLY_USE_MONTGOMERY_LADDER 0

/*
* Set number of bits used to generate mask for blinding the scalar of
* a point multiplication. Set to zero to disable this side-channel
* countermeasure.
*/
#define BOTAN_POINTGFP_SCALAR_BLINDING_BITS 20

/*
* Set number of bits used to generate mask for blinding the
* representation of an ECC point. Set to zero to diable this
* side-channel countermeasure.
*/
#define BOTAN_POINTGFP_RANDOMIZE_BLINDING_BITS 80

/*
* Normally blinding is performed by choosing a random starting point (plus
* its inverse, of a form appropriate to the algorithm being blinded), and
* then choosing new blinding operands by successive squaring of both
* values. This is much faster than computing a new starting point but
* introduces some possible coorelation
*
* To avoid possible leakage problems in long-running processes, the blinder
* periodically reinitializes the sequence. This value specifies how often
* a new sequence should be started.
*/
#define BOTAN_BLINDING_REINIT_INTERVAL 32

/*
* Userspace RNGs like HMAC_DRBG will reseed after a specified number
* of outputs are generated. Set to zero to disable automatic reseeding.
*/
#define BOTAN_RNG_DEFAULT_RESEED_INTERVAL 1024
#define BOTAN_RNG_RESEED_POLL_BITS 256
#define BOTAN_RNG_AUTO_RESEED_TIMEOUT std::chrono::milliseconds(10)
#define BOTAN_RNG_RESEED_DEFAULT_TIMEOUT std::chrono::milliseconds(50)

/*
* Specifies (in order) the list of entropy sources that will be used
* to seed an in-memory RNG. The first in the default list: "rdseed" and "rdrand"
* do not count as contributing any entropy
* but are included as they are fast and help protect against a
* seriously broken system RNG.
*/
#define BOTAN_ENTROPY_DEFAULT_SOURCES \
   { "rdseed", "rdrand", "darwin_secrandom", "dev_random", \
    "win32_cryptoapi", "proc_walk", "system_stats" }


/* Multiplier on a block cipher's native parallelism */
#define BOTAN_BLOCK_CIPHER_PAR_MULT 4

/*
* These control the RNG used by the system RNG interface
*/
#define BOTAN_SYSTEM_RNG_DEVICE "/dev/urandom"
#define BOTAN_SYSTEM_RNG_CRYPTOAPI_PROV_TYPE PROV_RSA_FULL

/*
* These paramaters control how many bytes to read from the system
* PRNG, and how long to block if applicable.
*
* Timeout is ignored on Windows as CryptGenRandom doesn't block
*/
#define BOTAN_SYSTEM_RNG_POLL_DEVICES { "/dev/urandom", "/dev/random", "/dev/srandom" }

/*
* This directory will be monitored by ProcWalking_EntropySource and
* the contents provided as entropy inputs to the RNG. May also be
* usefully set to something like "/sys", depending on the system being
* deployed to. Set to an empty string to disable.
*/
#define BOTAN_ENTROPY_PROC_FS_PATH "/proc"

#define BOTAN_SYSTEM_RNG_POLL_REQUEST 64
#define BOTAN_SYSTEM_RNG_POLL_TIMEOUT_MS 20


/*
How many times to read from the RDRAND/RDSEED RNGs.
Each read generates 32 bits of output
*/
#define BOTAN_ENTROPY_INTEL_RNG_POLLS 32

// According to Intel, RDRAND is guaranteed to generate a random number within 10 retries on a working CPU
#define BOTAN_ENTROPY_RDRAND_RETRIES 10

/*
* RdSeed is not guaranteed to generate a random number within a specific number of retries
* Define the number of retries here
*/
#define BOTAN_ENTROPY_RDSEED_RETRIES 20

/*
* If no way of dynamically determining the cache line size for the
* system exists, this value is used as the default. Used by the side
* channel countermeasures rather than for alignment purposes, so it is
* better to be on the smaller side if the exact value cannot be
* determined. Typically 32 or 64 bytes on modern CPUs.
*/
#if !defined(BOTAN_TARGET_CPU_DEFAULT_CACHE_LINE_SIZE)
  #define BOTAN_TARGET_CPU_DEFAULT_CACHE_LINE_SIZE 32
#endif

/**
* Controls how AutoSeeded_RNG is instantiated
*/
#if !defined(BOTAN_AUTO_RNG_HMAC)

  #if defined(BOTAN_HAS_SHA2_64)
    #define BOTAN_AUTO_RNG_HMAC "HMAC(SHA-384)"
  #elif defined(BOTAN_HAS_SHA2_32)
    #define BOTAN_AUTO_RNG_HMAC "HMAC(SHA-256)"
  #elif defined(BOTAN_HAS_SHA3)
    #define BOTAN_AUTO_RNG_HMAC "HMAC(SHA-3(256))"
  #elif defined(BOTAN_HAS_SHA1)
    #define BOTAN_AUTO_RNG_HMAC "HMAC(SHA-1)"
  #endif
  // Otherwise, no hash found: leave BOTAN_AUTO_RNG_HMAC undefined

#endif

// Append to macros living outside of functions, so that invocations must end with a semicolon.
// The struct is only declared to force the semicolon, it is never defined.
#define BOTAN_FORCE_SEMICOLON struct BOTAN_DUMMY_STRUCT

// Check for a common build problem:

#if defined(BOTAN_TARGET_ARCH_IS_X86_64) && ((defined(_MSC_VER) && !defined(_WIN64)) || \
                                             (defined(__clang__) && !defined(__x86_64__)) || \
                                             (defined(__GNUG__) && !defined(__x86_64__)))
    #error "Trying to compile Botan configured as x86_64 with non-x86_64 compiler."
#endif

#if defined(BOTAN_TARGET_ARCH_IS_X86_32) && ((defined(_MSC_VER) && defined(_WIN64)) || \
                                             (defined(__clang__) && !defined(__i386__)) || \
                                             (defined(__GNUG__) && !defined(__i386__)))

    #error "Trying to compile Botan configured as x86_32 with non-x86_32 compiler."
#endif

/* Should we use GCC-style inline assembler? */
#if !defined(BOTAN_USE_GCC_INLINE_ASM) && defined(__GNUC__)
  #define BOTAN_USE_GCC_INLINE_ASM 1
#endif

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
#if defined(_MSC_VER) && (_MSC_VER < 1900)
  // noexcept is not supported in VS 2013
  #include <yvals.h>
  #define BOTAN_NOEXCEPT _NOEXCEPT
#else
  #define BOTAN_NOEXCEPT noexcept
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
#elif defined(BOTAN_TARGET_COMPILER_IS_GCC)
  #define BOTAN_PARALLEL_FOR _Pragma("GCC ivdep") for
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



namespace Botan {

/**
* Called when an assertion fails
*/
BOTAN_NORETURN void BOTAN_DLL assertion_failure(const char* expr_str,
                                 const char* assertion_made,
                                 const char* func,
                                 const char* file,
                                 int line);

/**
* Make an assertion
*/
#define BOTAN_ASSERT(expr, assertion_made)                \
   do {                                                   \
      if(!(expr))                                         \
         Botan::assertion_failure(#expr,                  \
                                  assertion_made,         \
                                  BOTAN_CURRENT_FUNCTION, \
                                  __FILE__,               \
                                  __LINE__);              \
   } while(0)

/**
* Make an assertion
*/
#define BOTAN_ASSERT_NOMSG(expr)                          \
   do {                                                   \
      if(!(expr))                                         \
         Botan::assertion_failure(#expr,                  \
                                  "",                     \
                                  BOTAN_CURRENT_FUNCTION, \
                                  __FILE__,               \
                                  __LINE__);              \
   } while(0)

/**
* Assert that value1 == value2
*/
#define BOTAN_ASSERT_EQUAL(expr1, expr2, assertion_made)   \
   do {                                                    \
     if((expr1) != (expr2))                                \
       Botan::assertion_failure(#expr1 " == " #expr2,      \
                                  assertion_made,          \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Assert that expr1 (if true) implies expr2 is also true
*/
#define BOTAN_ASSERT_IMPLICATION(expr1, expr2, msg)        \
   do {                                                    \
     if((expr1) && !(expr2))                               \
       Botan::assertion_failure(#expr1 " implies " #expr2, \
                                msg,                       \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Assert that a pointer is not null
*/
#define BOTAN_ASSERT_NONNULL(ptr)                          \
   do {                                                    \
     if((ptr) == nullptr)                                  \
         Botan::assertion_failure(#ptr " is not null",     \
                                  "",                      \
                                  BOTAN_CURRENT_FUNCTION,  \
                                  __FILE__,                \
                                  __LINE__);               \
   } while(0)

/**
* Mark variable as unused
*/
#define BOTAN_UNUSED(v) static_cast<void>(v)

}


/**
* The primary namespace for the botan library
*/
namespace Botan {

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::int32_t;
using std::int64_t;
using std::size_t;

/*
* These typedefs are no longer used within the library headers
* or code. They are kept only for compatability with software
* written against older versions.
*/
using byte   = std::uint8_t;
using u16bit = std::uint16_t;
using u32bit = std::uint32_t;
using u64bit = std::uint64_t;
using s32bit = std::int32_t;

/**
* A default buffer size; typically a memory page
*/
static const size_t DEFAULT_BUFFERSIZE = BOTAN_DEFAULT_BUFFER_SIZE;

}


namespace Botan {

/**
* Scrub memory contents in a way that a compiler should not elide,
* using some system specific technique. Note that this function might
* not zero the memory (for example, in some hypothetical
* implementation it might combine the memory contents with the output
* of a system PRNG), but if you can detect any difference in behavior
* at runtime then the clearing is side-effecting and you can just
* use `clear_mem`.
*
* Use this function to scrub memory just before deallocating it, or on
* a stack buffer before returning from the function.
*
* @param ptr a pointer to memory to scrub
* @param n the number of bytes pointed to by ptr
*/
BOTAN_DLL void secure_scrub_memory(void* ptr, size_t n);

/**
* Zero out some bytes
* @param ptr a pointer to memory to zero
* @param bytes the number of bytes to zero in ptr
*/
inline void clear_bytes(void* ptr, size_t bytes)
   {
   if(bytes > 0)
      {
      std::memset(ptr, 0, bytes);
      }
   }

/**
* Zero memory before use. This simply calls memset and should not be
* used in cases where the compiler cannot see the call as a
* side-effecting operation (for example, if calling clear_mem before
* deallocating memory, the compiler would be allowed to omit the call
* to memset entirely under the as-if rule.)
*
* @param ptr a pointer to an array of Ts to zero
* @param n the number of Ts pointed to by ptr
*/
template<typename T> inline void clear_mem(T* ptr, size_t n)
   {
   clear_bytes(ptr, sizeof(T)*n);
   }

/**
* Copy memory
* @param out the destination array
* @param in the source array
* @param n the number of elements of in/out
*/
template<typename T> inline void copy_mem(T* out, const T* in, size_t n)
   {
   if(n > 0)
      {
      std::memmove(out, in, sizeof(T)*n);
      }
   }

/**
* Set memory to a fixed value
* @param ptr a pointer to an array
* @param n the number of Ts pointed to by ptr
* @param val the value to set each byte to
*/
template<typename T>
inline void set_mem(T* ptr, size_t n, uint8_t val)
   {
   if(n > 0)
      {
      std::memset(ptr, val, sizeof(T)*n);
      }
   }

/**
* Memory comparison, input insensitive
* @param p1 a pointer to an array
* @param p2 a pointer to another array
* @param n the number of Ts in p1 and p2
* @return true iff p1[i] == p2[i] forall i in [0...n)
*/
template<typename T> inline bool same_mem(const T* p1, const T* p2, size_t n)
   {
   volatile T difference = 0;

   for(size_t i = 0; i != n; ++i)
      difference |= (p1[i] ^ p2[i]);

   return difference == 0;
   }

/**
* XOR_ arrays. Postcondition out[i] = in[i] ^ out[i] forall i = 0...length
* @param out the input/output buffer
* @param in the read-only input buffer
* @param length the length of the buffers
*/
template<typename T>
void xor_buf(T out[], const T in[], size_t length)
   {
   for(size_t i = 0; i != length; ++i)
      {
      out[i] ^= in[i];
      }
   }

/**
* XOR arrays. Postcondition out[i] = in[i] ^ in2[i] forall i = 0...length
* @param out the output buffer
* @param in the first input buffer
* @param in2 the second output buffer
* @param length the length of the three buffers
*/
template<typename T> void xor_buf(T out[],
                                  const T in[],
                                  const T in2[],
                                  size_t length)
   {
   for(size_t i = 0; i != length; ++i)
      {
      out[i] = in[i] ^ in2[i];
      }
   }

template<typename Alloc, typename Alloc2>
void xor_buf(std::vector<uint8_t, Alloc>& out,
             const std::vector<uint8_t, Alloc2>& in,
             size_t n)
   {
   xor_buf(out.data(), in.data(), n);
   }

template<typename Alloc>
void xor_buf(std::vector<uint8_t, Alloc>& out,
             const uint8_t* in,
             size_t n)
   {
   xor_buf(out.data(), in, n);
   }

template<typename Alloc, typename Alloc2>
void xor_buf(std::vector<uint8_t, Alloc>& out,
             const uint8_t* in,
             const std::vector<uint8_t, Alloc2>& in2,
             size_t n)
   {
   xor_buf(out.data(), in, in2.data(), n);
   }

template<typename T, typename Alloc, typename Alloc2>
std::vector<T, Alloc>&
operator^=(std::vector<T, Alloc>& out,
           const std::vector<T, Alloc2>& in)
   {
   if(out.size() < in.size())
      out.resize(in.size());

   xor_buf(out.data(), in.data(), in.size());
   return out;
   }

}


#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)

#if defined(BOTAN_TARGET_OS_HAS_THREADS)


namespace Botan {

template<typename T> using lock_guard_type = std::lock_guard<T>;
typedef std::mutex mutex_type;

}

#elif defined(BOTAN_TARGET_OS_TYPE_IS_UNIKERNEL)

// No threads

namespace Botan {

template<typename Mutex>
class lock_guard
   {
   public:
      explicit lock_guard(Mutex& m) : m_mutex(m)
         { m_mutex.lock(); }

      ~lock_guard() { m_mutex.unlock(); }

      lock_guard(const lock_guard& other) = delete;
      lock_guard& operator=(const lock_guard& other) = delete;
   private:
      Mutex& m_mutex;
   };

class noop_mutex
   {
   public:
      void lock() {}
      void unlock() {}
   };

typedef noop_mutex mutex_type;
template<typename T> using lock_guard_type = lock_guard<T>;

}

#else
  #error "Threads unexpectedly disabled in non unikernel build"
#endif


namespace Botan {

class BOTAN_DLL mlock_allocator
   {
   public:
      static mlock_allocator& instance();

      void* allocate(size_t num_elems, size_t elem_size);

      bool deallocate(void* p, size_t num_elems, size_t elem_size);

      mlock_allocator(const mlock_allocator&) = delete;

      mlock_allocator& operator=(const mlock_allocator&) = delete;

   private:
      mlock_allocator();

      ~mlock_allocator();

      mutex_type m_mutex;
      std::vector<std::pair<size_t, size_t>> m_freelist;
      uint8_t* m_pool = nullptr;
      size_t m_poolsize = 0;
   };

}

#endif

namespace Botan {

template<typename T>
class secure_allocator
   {
   public:
      /*
      * Assert exists to prevent someone from doing something that will
      * probably crash anyway (like secure_vector<non_POD_t> where ~non_POD_t
      * deletes a member pointer which was zeroed before it ran).
      * MSVC in debug mode uses non-integral proxy types in container types
      * like std::vector, thus we disable the check there.
      */
#if !defined(_ITERATOR_DEBUG_LEVEL) || _ITERATOR_DEBUG_LEVEL == 0
      static_assert(std::is_integral<T>::value, "secure_allocator supports only integer types");
#endif

      typedef T          value_type;

      typedef T*         pointer;
      typedef const T*   const_pointer;

      typedef T&         reference;
      typedef const T&   const_reference;

      typedef std::size_t     size_type;
      typedef std::ptrdiff_t  difference_type;

      secure_allocator() BOTAN_NOEXCEPT {}

      template<typename U>
      secure_allocator(const secure_allocator<U>&) BOTAN_NOEXCEPT {}

      ~secure_allocator() BOTAN_NOEXCEPT {}

      pointer address(reference x) const BOTAN_NOEXCEPT
         { return std::addressof(x); }

      const_pointer address(const_reference x) const BOTAN_NOEXCEPT
         { return std::addressof(x); }

      pointer allocate(size_type n, const void* = 0)
         {
#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
         if(pointer p = static_cast<pointer>(mlock_allocator::instance().allocate(n, sizeof(T))))
            return p;
#endif

         pointer p = new T[n];
         clear_mem(p, n);
         return p;
         }

      void deallocate(pointer p, size_type n)
         {
         secure_scrub_memory(p, n);

#if defined(BOTAN_HAS_LOCKING_ALLOCATOR)
         if(mlock_allocator::instance().deallocate(p, n, sizeof(T)))
            return;
#endif

         delete [] p;
         }

      size_type max_size() const BOTAN_NOEXCEPT
         {
         return static_cast<size_type>(-1) / sizeof(T);
         }

      template<typename U, typename... Args>
      void construct(U* p, Args&&... args)
         {
         ::new(static_cast<void*>(p)) U(std::forward<Args>(args)...);
         }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100)
      template<typename U> void destroy(U* p) { p->~U(); }
#pragma warning(pop)
#endif
   };

template<typename T, typename U> inline bool
operator==(const secure_allocator<T>&, const secure_allocator<U>&)
   { return true; }

template<typename T, typename U> inline bool
operator!=(const secure_allocator<T>&, const secure_allocator<U>&)
   { return false; }

template<typename T> using secure_vector = std::vector<T, secure_allocator<T>>;
template<typename T> using secure_deque = std::deque<T, secure_allocator<T>>;

template<typename T>
std::vector<T> unlock(const secure_vector<T>& in)
   {
   std::vector<T> out(in.size());
   copy_mem(out.data(), in.data(), in.size());
   return out;
   }

template<typename T, typename Alloc>
size_t buffer_insert(std::vector<T, Alloc>& buf,
                     size_t buf_offset,
                     const T input[],
                     size_t input_length)
   {
   const size_t to_copy = std::min(input_length, buf.size() - buf_offset);
   if (to_copy > 0)
      {
      copy_mem(&buf[buf_offset], input, to_copy);
      }
   return to_copy;
   }

template<typename T, typename Alloc, typename Alloc2>
size_t buffer_insert(std::vector<T, Alloc>& buf,
                     size_t buf_offset,
                     const std::vector<T, Alloc2>& input)
   {
   const size_t to_copy = std::min(input.size(), buf.size() - buf_offset);
   if (to_copy > 0)
      {
      copy_mem(&buf[buf_offset], input.data(), to_copy);
      }
   return to_copy;
   }

template<typename T, typename Alloc, typename Alloc2>
std::vector<T, Alloc>&
operator+=(std::vector<T, Alloc>& out,
           const std::vector<T, Alloc2>& in)
   {
   const size_t copy_offset = out.size();
   out.resize(out.size() + in.size());
   if (in.size() > 0)
      {
      copy_mem(&out[copy_offset], in.data(), in.size());
      }
   return out;
   }

template<typename T, typename Alloc>
std::vector<T, Alloc>& operator+=(std::vector<T, Alloc>& out, T in)
   {
   out.push_back(in);
   return out;
   }

template<typename T, typename Alloc, typename L>
std::vector<T, Alloc>& operator+=(std::vector<T, Alloc>& out,
                                  const std::pair<const T*, L>& in)
   {
   const size_t copy_offset = out.size();
   out.resize(out.size() + in.second);
   if (in.second > 0)
      {
      copy_mem(&out[copy_offset], in.first, in.second);
      }
   return out;
   }

template<typename T, typename Alloc, typename L>
std::vector<T, Alloc>& operator+=(std::vector<T, Alloc>& out,
                                  const std::pair<T*, L>& in)
   {
   const size_t copy_offset = out.size();
   out.resize(out.size() + in.second);
   if (in.second > 0)
      {
      copy_mem(&out[copy_offset], in.first, in.second);
      }
   return out;
   }

/**
* Zeroise the values; length remains unchanged
* @param vec the vector to zeroise
*/
template<typename T, typename Alloc>
void zeroise(std::vector<T, Alloc>& vec)
   {
   clear_mem(vec.data(), vec.size());
   }

/**
* Zeroise the values then free the memory
* @param vec the vector to zeroise and free
*/
template<typename T, typename Alloc>
void zap(std::vector<T, Alloc>& vec)
   {
   zeroise(vec);
   vec.clear();
   vec.shrink_to_fit();
   }

}


namespace Botan {

/**
* Bit rotation left
* @param input the input word
* @param rot the number of bits to rotate
* @return input rotated left by rot bits
*/
template<typename T> inline T rotate_left(T input, size_t rot)
   {
   return (rot == 0) ? input : static_cast<T>((input << rot) | (input >> (8*sizeof(T)-rot)));;
   }

/**
* Bit rotation right
* @param input the input word
* @param rot the number of bits to rotate
* @return input rotated right by rot bits
*/
template<typename T> inline T rotate_right(T input, size_t rot)
   {
   return (rot == 0) ? input : static_cast<T>((input >> rot) | (input << (8*sizeof(T)-rot)));
   }

}


#if defined(BOTAN_TARGET_CPU_HAS_SSE2) && !defined(BOTAN_NO_SSE_INTRINSICS)
  #include <emmintrin.h>
#endif

namespace Botan {

/**
* Swap a 16 bit integer
*/
inline uint16_t reverse_bytes(uint16_t val)
   {
   return rotate_left(val, 8);
   }

/**
* Swap a 32 bit integer
*/
inline uint32_t reverse_bytes(uint32_t val)
   {
#if BOTAN_GCC_VERSION >= 430 && !defined(BOTAN_TARGET_ARCH_IS_ARM32)
   /*
   GCC intrinsic added in 4.3, works for a number of CPUs

   However avoid under ARM, as it branches to a function in libgcc
   instead of generating inline asm, so slower even than the generic
   rotate version below.
   */
   return __builtin_bswap32(val);

#elif defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

   // GCC-style inline assembly for x86 or x86-64
   asm("bswapl %0" : "=r" (val) : "0" (val));
   return val;

#elif defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_TARGET_ARCH_IS_ARM32)

   asm ("eor r3, %1, %1, ror #16\n\t"
        "bic r3, r3, #0x00FF0000\n\t"
        "mov %0, %1, ror #8\n\t"
        "eor %0, %0, r3, lsr #8"
        : "=r" (val)
        : "0" (val)
        : "r3", "cc");

   return val;

#elif defined(_MSC_VER) && defined(BOTAN_TARGET_ARCH_IS_X86_32)

   // Visual C++ inline asm for 32-bit x86, by Yves Jerschow
   __asm mov eax, val;
   __asm bswap eax;

#else

   // Generic implementation
   return (rotate_right(val, 8) & 0xFF00FF00) |
          (rotate_left (val, 8) & 0x00FF00FF);

#endif
   }

/**
* Swap a 64 bit integer
*/
inline uint64_t reverse_bytes(uint64_t val)
   {
#if BOTAN_GCC_VERSION >= 430

   // GCC intrinsic added in 4.3, works for a number of CPUs
   return __builtin_bswap64(val);

#elif defined(BOTAN_USE_GCC_INLINE_ASM) && defined(BOTAN_TARGET_ARCH_IS_X86_64)
   // GCC-style inline assembly for x86-64
   asm("bswapq %0" : "=r" (val) : "0" (val));
   return val;

#else
   /* Generic implementation. Defined in terms of 32-bit bswap so any
    * optimizations in that version can help here (particularly
    * useful for 32-bit x86).
    */

   uint32_t hi = static_cast<uint32_t>(val >> 32);
   uint32_t lo = static_cast<uint32_t>(val);

   hi = reverse_bytes(hi);
   lo = reverse_bytes(lo);

   return (static_cast<uint64_t>(lo) << 32) | hi;
#endif
   }

/**
* Swap 4 Ts in an array
*/
template<typename T>
inline void bswap_4(T x[4])
   {
   x[0] = reverse_bytes(x[0]);
   x[1] = reverse_bytes(x[1]);
   x[2] = reverse_bytes(x[2]);
   x[3] = reverse_bytes(x[3]);
   }

#if defined(BOTAN_TARGET_CPU_HAS_SSE2) && !defined(BOTAN_NO_SSE_INTRINSICS)

/**
* Swap 4 uint32_ts in an array using SSE2 shuffle instructions
*/
template<>
inline void bswap_4(uint32_t x[4])
   {
   __m128i T = _mm_loadu_si128(reinterpret_cast<const __m128i*>(x));

   T = _mm_shufflehi_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));
   T = _mm_shufflelo_epi16(T, _MM_SHUFFLE(2, 3, 0, 1));

   T =  _mm_or_si128(_mm_srli_epi16(T, 8), _mm_slli_epi16(T, 8));

   _mm_storeu_si128(reinterpret_cast<__m128i*>(x), T);
   }

#endif

}


#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK

#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)

#define BOTAN_ENDIAN_N2B(x) (x)
#define BOTAN_ENDIAN_B2N(x) (x)

#define BOTAN_ENDIAN_N2L(x) reverse_bytes(x)
#define BOTAN_ENDIAN_L2N(x) reverse_bytes(x)

#elif defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)

#define BOTAN_ENDIAN_N2L(x) (x)
#define BOTAN_ENDIAN_L2N(x) (x)

#define BOTAN_ENDIAN_N2B(x) reverse_bytes(x)
#define BOTAN_ENDIAN_B2N(x) reverse_bytes(x)

#endif

#endif

namespace Botan {

/**
* Byte extraction
* @param byte_num which byte to extract, 0 == highest byte
* @param input the value to extract from
* @return byte byte_num of input
*/
template<typename T> inline uint8_t get_byte(size_t byte_num, T input)
   {
   return static_cast<uint8_t>(
      input >> (((~byte_num)&(sizeof(T)-1)) << 3)
      );
   }

/**
* Make a uint16_t from two bytes
* @param i0 the first byte
* @param i1 the second byte
* @return i0 || i1
*/
inline uint16_t make_uint16(uint8_t i0, uint8_t i1)
   {
   return ((static_cast<uint16_t>(i0) << 8) | i1);
   }

/**
* Make a uint32_t from four bytes
* @param i0 the first byte
* @param i1 the second byte
* @param i2 the third byte
* @param i3 the fourth byte
* @return i0 || i1 || i2 || i3
*/
inline uint32_t make_uint32(uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3)
   {
   return ((static_cast<uint32_t>(i0) << 24) |
           (static_cast<uint32_t>(i1) << 16) |
           (static_cast<uint32_t>(i2) <<  8) |
           (static_cast<uint32_t>(i3)));
   }

/**
* Make a uint32_t from eight bytes
* @param i0 the first byte
* @param i1 the second byte
* @param i2 the third byte
* @param i3 the fourth byte
* @param i4 the fifth byte
* @param i5 the sixth byte
* @param i6 the seventh byte
* @param i7 the eighth byte
* @return i0 || i1 || i2 || i3 || i4 || i5 || i6 || i7
*/
inline uint64_t make_uint64(uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3,
                          uint8_t i4, uint8_t i5, uint8_t i6, uint8_t i7)
    {
   return ((static_cast<uint64_t>(i0) << 56) |
           (static_cast<uint64_t>(i1) << 48) |
           (static_cast<uint64_t>(i2) << 40) |
           (static_cast<uint64_t>(i3) << 32) |
           (static_cast<uint64_t>(i4) << 24) |
           (static_cast<uint64_t>(i5) << 16) |
           (static_cast<uint64_t>(i6) <<  8) |
           (static_cast<uint64_t>(i7)));
    }

/**
* Load a big-endian word
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th T of in, as a big-endian value
*/
template<typename T>
inline T load_be(const uint8_t in[], size_t off)
   {
   in += off * sizeof(T);
   T out = 0;
   for(size_t i = 0; i != sizeof(T); ++i)
      out = (out << 8) | in[i];
   return out;
   }

/**
* Load a little-endian word
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th T of in, as a litte-endian value
*/
template<typename T>
inline T load_le(const uint8_t in[], size_t off)
   {
   in += off * sizeof(T);
   T out = 0;
   for(size_t i = 0; i != sizeof(T); ++i)
      out = (out << 8) | in[sizeof(T)-1-i];
   return out;
   }

/**
* Load a big-endian uint16_t
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th uint16_t of in, as a big-endian value
*/
template<>
inline uint16_t load_be<uint16_t>(const uint8_t in[], size_t off)
   {
   in += off * sizeof(uint16_t);

#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   uint16_t x;
   std::memcpy(&x, in, sizeof(x));
   return BOTAN_ENDIAN_N2B(x);
#else
   return make_uint16(in[0], in[1]);
#endif
   }

/**
* Load a little-endian uint16_t
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th uint16_t of in, as a little-endian value
*/
template<>
inline uint16_t load_le<uint16_t>(const uint8_t in[], size_t off)
   {
   in += off * sizeof(uint16_t);

#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   uint16_t x;
   std::memcpy(&x, in, sizeof(x));
   return BOTAN_ENDIAN_N2L(x);
#else
   return make_uint16(in[1], in[0]);
#endif
   }

/**
* Load a big-endian uint32_t
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th uint32_t of in, as a big-endian value
*/
template<>
inline uint32_t load_be<uint32_t>(const uint8_t in[], size_t off)
   {
   in += off * sizeof(uint32_t);
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   uint32_t x;
   std::memcpy(&x, in, sizeof(x));
   return BOTAN_ENDIAN_N2B(x);
#else
   return make_uint32(in[0], in[1], in[2], in[3]);
#endif
   }

/**
* Load a little-endian uint32_t
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th uint32_t of in, as a little-endian value
*/
template<>
inline uint32_t load_le<uint32_t>(const uint8_t in[], size_t off)
   {
   in += off * sizeof(uint32_t);
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   uint32_t x;
   std::memcpy(&x, in, sizeof(x));
   return BOTAN_ENDIAN_N2L(x);
#else
   return make_uint32(in[3], in[2], in[1], in[0]);
#endif
   }

/**
* Load a big-endian uint64_t
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th uint64_t of in, as a big-endian value
*/
template<>
inline uint64_t load_be<uint64_t>(const uint8_t in[], size_t off)
   {
   in += off * sizeof(uint64_t);
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   uint64_t x;
   std::memcpy(&x, in, sizeof(x));
   return BOTAN_ENDIAN_N2B(x);
#else
   return make_uint64(in[0], in[1], in[2], in[3],
                      in[4], in[5], in[6], in[7]);
#endif
   }

/**
* Load a little-endian uint64_t
* @param in a pointer to some bytes
* @param off an offset into the array
* @return off'th uint64_t of in, as a little-endian value
*/
template<>
inline uint64_t load_le<uint64_t>(const uint8_t in[], size_t off)
   {
   in += off * sizeof(uint64_t);
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   uint64_t x;
   std::memcpy(&x, in, sizeof(x));
   return BOTAN_ENDIAN_N2L(x);
#else
   return make_uint64(in[7], in[6], in[5], in[4],
                      in[3], in[2], in[1], in[0]);
#endif
   }

/**
* Load two little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
*/
template<typename T>
inline void load_le(const uint8_t in[], T& x0, T& x1)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   }

/**
* Load four little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
*/
template<typename T>
inline void load_le(const uint8_t in[],
                    T& x0, T& x1, T& x2, T& x3)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   x2 = load_le<T>(in, 2);
   x3 = load_le<T>(in, 3);
   }

/**
* Load eight little-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
* @param x4 where the fifth word will be written
* @param x5 where the sixth word will be written
* @param x6 where the seventh word will be written
* @param x7 where the eighth word will be written
*/
template<typename T>
inline void load_le(const uint8_t in[],
                    T& x0, T& x1, T& x2, T& x3,
                    T& x4, T& x5, T& x6, T& x7)
   {
   x0 = load_le<T>(in, 0);
   x1 = load_le<T>(in, 1);
   x2 = load_le<T>(in, 2);
   x3 = load_le<T>(in, 3);
   x4 = load_le<T>(in, 4);
   x5 = load_le<T>(in, 5);
   x6 = load_le<T>(in, 6);
   x7 = load_le<T>(in, 7);
   }

/**
* Load a variable number of little-endian words
* @param out the output array of words
* @param in the input array of bytes
* @param count how many words are in in
*/
template<typename T>
inline void load_le(T out[],
                    const uint8_t in[],
                    size_t count)
   {
   if(count > 0)
      {
#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
      std::memcpy(out, in, sizeof(T)*count);
#elif defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
      std::memcpy(out, in, sizeof(T)*count);
      const size_t blocks = count - (count % 4);
      const size_t left = count - blocks;

      for(size_t i = 0; i != blocks; i += 4)
         bswap_4(out + i);

      for(size_t i = 0; i != left; ++i)
         out[blocks+i] = reverse_bytes(out[blocks+i]);
#else
      for(size_t i = 0; i != count; ++i)
         out[i] = load_le<T>(in, i);
#endif
      }
   }

/**
* Load two big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
*/
template<typename T>
inline void load_be(const uint8_t in[], T& x0, T& x1)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   }

/**
* Load four big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
*/
template<typename T>
inline void load_be(const uint8_t in[],
                    T& x0, T& x1, T& x2, T& x3)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   x2 = load_be<T>(in, 2);
   x3 = load_be<T>(in, 3);
   }

/**
* Load eight big-endian words
* @param in a pointer to some bytes
* @param x0 where the first word will be written
* @param x1 where the second word will be written
* @param x2 where the third word will be written
* @param x3 where the fourth word will be written
* @param x4 where the fifth word will be written
* @param x5 where the sixth word will be written
* @param x6 where the seventh word will be written
* @param x7 where the eighth word will be written
*/
template<typename T>
inline void load_be(const uint8_t in[],
                    T& x0, T& x1, T& x2, T& x3,
                    T& x4, T& x5, T& x6, T& x7)
   {
   x0 = load_be<T>(in, 0);
   x1 = load_be<T>(in, 1);
   x2 = load_be<T>(in, 2);
   x3 = load_be<T>(in, 3);
   x4 = load_be<T>(in, 4);
   x5 = load_be<T>(in, 5);
   x6 = load_be<T>(in, 6);
   x7 = load_be<T>(in, 7);
   }

/**
* Load a variable number of big-endian words
* @param out the output array of words
* @param in the input array of bytes
* @param count how many words are in in
*/
template<typename T>
inline void load_be(T out[],
                    const uint8_t in[],
                    size_t count)
   {
   if(count > 0)
      {
#if defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
      std::memcpy(out, in, sizeof(T)*count);
#elif defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN)
      std::memcpy(out, in, sizeof(T)*count);
      const size_t blocks = count - (count % 4);
      const size_t left = count - blocks;

      for(size_t i = 0; i != blocks; i += 4)
         bswap_4(out + i);

      for(size_t i = 0; i != left; ++i)
         out[blocks+i] = reverse_bytes(out[blocks+i]);
#else
      for(size_t i = 0; i != count; ++i)
         out[i] = load_be<T>(in, i);
#endif
      }
   }

/**
* Store a big-endian uint16_t
* @param in the input uint16_t
* @param out the byte array to write to
*/
inline void store_be(uint16_t in, uint8_t out[2])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   uint16_t o = BOTAN_ENDIAN_N2B(in);
   std::memcpy(out, &o, sizeof(o));
#else
   out[0] = get_byte(0, in);
   out[1] = get_byte(1, in);
#endif
   }

/**
* Store a little-endian uint16_t
* @param in the input uint16_t
* @param out the byte array to write to
*/
inline void store_le(uint16_t in, uint8_t out[2])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   uint16_t o = BOTAN_ENDIAN_N2L(in);
   std::memcpy(out, &o, sizeof(o));
#else
   out[0] = get_byte(1, in);
   out[1] = get_byte(0, in);
#endif
   }

/**
* Store a big-endian uint32_t
* @param in the input uint32_t
* @param out the byte array to write to
*/
inline void store_be(uint32_t in, uint8_t out[4])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   uint32_t o = BOTAN_ENDIAN_B2N(in);
   std::memcpy(out, &o, sizeof(o));
#else
   out[0] = get_byte(0, in);
   out[1] = get_byte(1, in);
   out[2] = get_byte(2, in);
   out[3] = get_byte(3, in);
#endif
   }

/**
* Store a little-endian uint32_t
* @param in the input uint32_t
* @param out the byte array to write to
*/
inline void store_le(uint32_t in, uint8_t out[4])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   uint32_t o = BOTAN_ENDIAN_L2N(in);
   std::memcpy(out, &o, sizeof(o));
#else
   out[0] = get_byte(3, in);
   out[1] = get_byte(2, in);
   out[2] = get_byte(1, in);
   out[3] = get_byte(0, in);
#endif
   }

/**
* Store a big-endian uint64_t
* @param in the input uint64_t
* @param out the byte array to write to
*/
inline void store_be(uint64_t in, uint8_t out[8])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   uint64_t o = BOTAN_ENDIAN_B2N(in);
   std::memcpy(out, &o, sizeof(o));
#else
   out[0] = get_byte(0, in);
   out[1] = get_byte(1, in);
   out[2] = get_byte(2, in);
   out[3] = get_byte(3, in);
   out[4] = get_byte(4, in);
   out[5] = get_byte(5, in);
   out[6] = get_byte(6, in);
   out[7] = get_byte(7, in);
#endif
   }

/**
* Store a little-endian uint64_t
* @param in the input uint64_t
* @param out the byte array to write to
*/
inline void store_le(uint64_t in, uint8_t out[8])
   {
#if BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK
   uint64_t o = BOTAN_ENDIAN_L2N(in);
   std::memcpy(out, &o, sizeof(o));
#else
   out[0] = get_byte(7, in);
   out[1] = get_byte(6, in);
   out[2] = get_byte(5, in);
   out[3] = get_byte(4, in);
   out[4] = get_byte(3, in);
   out[5] = get_byte(2, in);
   out[6] = get_byte(1, in);
   out[7] = get_byte(0, in);
#endif
   }

/**
* Store two little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
*/
template<typename T>
inline void store_le(uint8_t out[], T x0, T x1)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   }

/**
* Store two big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
*/
template<typename T>
inline void store_be(uint8_t out[], T x0, T x1)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   }

/**
* Store four little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
*/
template<typename T>
inline void store_le(uint8_t out[], T x0, T x1, T x2, T x3)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   store_le(x2, out + (2 * sizeof(T)));
   store_le(x3, out + (3 * sizeof(T)));
   }

/**
* Store four big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
*/
template<typename T>
inline void store_be(uint8_t out[], T x0, T x1, T x2, T x3)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   store_be(x2, out + (2 * sizeof(T)));
   store_be(x3, out + (3 * sizeof(T)));
   }

/**
* Store eight little-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
* @param x4 the fifth word
* @param x5 the sixth word
* @param x6 the seventh word
* @param x7 the eighth word
*/
template<typename T>
inline void store_le(uint8_t out[], T x0, T x1, T x2, T x3,
                                 T x4, T x5, T x6, T x7)
   {
   store_le(x0, out + (0 * sizeof(T)));
   store_le(x1, out + (1 * sizeof(T)));
   store_le(x2, out + (2 * sizeof(T)));
   store_le(x3, out + (3 * sizeof(T)));
   store_le(x4, out + (4 * sizeof(T)));
   store_le(x5, out + (5 * sizeof(T)));
   store_le(x6, out + (6 * sizeof(T)));
   store_le(x7, out + (7 * sizeof(T)));
   }

/**
* Store eight big-endian words
* @param out the output byte array
* @param x0 the first word
* @param x1 the second word
* @param x2 the third word
* @param x3 the fourth word
* @param x4 the fifth word
* @param x5 the sixth word
* @param x6 the seventh word
* @param x7 the eighth word
*/
template<typename T>
inline void store_be(uint8_t out[], T x0, T x1, T x2, T x3,
                                 T x4, T x5, T x6, T x7)
   {
   store_be(x0, out + (0 * sizeof(T)));
   store_be(x1, out + (1 * sizeof(T)));
   store_be(x2, out + (2 * sizeof(T)));
   store_be(x3, out + (3 * sizeof(T)));
   store_be(x4, out + (4 * sizeof(T)));
   store_be(x5, out + (5 * sizeof(T)));
   store_be(x6, out + (6 * sizeof(T)));
   store_be(x7, out + (7 * sizeof(T)));
   }

template<typename T>
void copy_out_be(uint8_t out[], size_t out_bytes, const T in[])
   {
   while(out_bytes >= sizeof(T))
      {
      store_be(in[0], out);
      out += sizeof(T);
      out_bytes -= sizeof(T);
      in += 1;
   }

   for(size_t i = 0; i != out_bytes; ++i)
      out[i] = get_byte(i%8, in[0]);
   }

template<typename T, typename Alloc>
void copy_out_vec_be(uint8_t out[], size_t out_bytes, const std::vector<T, Alloc>& in)
   {
   copy_out_be(out, out_bytes, in.data());
   }

template<typename T>
void copy_out_le(uint8_t out[], size_t out_bytes, const T in[])
   {
   while(out_bytes >= sizeof(T))
      {
      store_le(in[0], out);
      out += sizeof(T);
      out_bytes -= sizeof(T);
      in += 1;
   }

   for(size_t i = 0; i != out_bytes; ++i)
      out[i] = get_byte(sizeof(T) - 1 - (i % 8), in[0]);
   }

template<typename T, typename Alloc>
void copy_out_vec_le(uint8_t out[], size_t out_bytes, const std::vector<T, Alloc>& in)
   {
   copy_out_le(out, out_bytes, in.data());
   }

}


namespace Botan {

/**
* This class represents any kind of computation which uses an internal
* state, such as hash functions or MACs
*/
class BOTAN_DLL Buffered_Computation
   {
   public:
      /**
      * @return length of the output of this function in bytes
      */
      virtual size_t output_length() const = 0;

      /**
      * Add new input to process.
      * @param in the input to process as a byte array
      * @param length of param in in bytes
      */
      void update(const uint8_t in[], size_t length) { add_data(in, length); }

      /**
      * Add new input to process.
      * @param in the input to process as a secure_vector
      */
      void update(const secure_vector<uint8_t>& in)
         {
         add_data(in.data(), in.size());
         }

      /**
      * Add new input to process.
      * @param in the input to process as a std::vector
      */
      void update(const std::vector<uint8_t>& in)
         {
         add_data(in.data(), in.size());
         }

      /**
      * Add an integer in big-endian order
      * @param in the value
      */
      template<typename T> void update_be(const T in)
         {
         for(size_t i = 0; i != sizeof(T); ++i)
            {
            uint8_t b = get_byte(i, in);
            add_data(&b, 1);
            }
         }

      /**
      * Add new input to process.
      * @param str the input to process as a std::string. Will be interpreted
      * as a byte array based on the strings encoding.
      */
      void update(const std::string& str)
         {
         add_data(reinterpret_cast<const uint8_t*>(str.data()), str.size());
         }

      /**
      * Process a single byte.
      * @param in the byte to process
      */
      void update(uint8_t in) { add_data(&in, 1); }

      /**
      * Complete the computation and retrieve the
      * final result.
      * @param out The byte array to be filled with the result.
      * Must be of length output_length()
      */
      void final(uint8_t out[]) { final_result(out); }

      /**
      * Complete the computation and retrieve the
      * final result.
      * @return secure_vector holding the result
      */
      secure_vector<uint8_t> final()
         {
         secure_vector<uint8_t> output(output_length());
         final_result(output.data());
         return output;
         }

      std::vector<uint8_t> final_stdvec()
         {
         std::vector<uint8_t> output(output_length());
         final_result(output.data());
         return output;
         }

      template<typename Alloc>
         void final(std::vector<uint8_t, Alloc>& out)
         {
         out.resize(output_length());
         final_result(out.data());
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process as a byte array
      * @param length the length of the byte array
      * @result the result of the call to final()
      */
      secure_vector<uint8_t> process(const uint8_t in[], size_t length)
         {
         add_data(in, length);
         return final();
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process
      * @result the result of the call to final()
      */
      secure_vector<uint8_t> process(const secure_vector<uint8_t>& in)
         {
         add_data(in.data(), in.size());
         return final();
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process
      * @result the result of the call to final()
      */
      secure_vector<uint8_t> process(const std::vector<uint8_t>& in)
         {
         add_data(in.data(), in.size());
         return final();
         }

      /**
      * Update and finalize computation. Does the same as calling update()
      * and final() consecutively.
      * @param in the input to process as a string
      * @result the result of the call to final()
      */
      secure_vector<uint8_t> process(const std::string& in)
         {
         update(in);
         return final();
         }

      virtual ~Buffered_Computation() {}
   private:
      /**
      * Add more data to the computation
      * @param input is an input buffer
      * @param length is the length of input in bytes
      */
      virtual void add_data(const uint8_t input[], size_t length) = 0;

      /**
      * Write the final output to out
      * @param out is an output buffer of output_length()
      */
      virtual void final_result(uint8_t out[]) = 0;
   };

}


namespace Botan {

/**
* This class represents hash function (message digest) objects
*/
class BOTAN_DLL HashFunction : public Buffered_Computation
   {
   public:
      /**
      * Create an instance based on a name, or return null if the
      * algo/provider combination cannot be found. If provider is
      * empty then best available is chosen.
      */
      static std::unique_ptr<HashFunction>
         create(const std::string& algo_spec,
                const std::string& provider = "");

      /**
      * Create an instance based on a name
      * If provider is empty then best available is chosen.
      * @param algo_spec algorithm name
      * @param provider provider implementation to use
      * Throws Lookup_Error if not not found.
      */
      static std::unique_ptr<HashFunction>
         create_or_throw(const std::string& algo_spec,
                         const std::string& provider = "");

      /**
      * @return list of available providers for this algorithm, empty if not available
      * @param algo_spec algorithm name
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      /**
      * @return new object representing the same algorithm as *this
      */
      virtual HashFunction* clone() const = 0;

      /**
      * @return provider information about this implementation. Default is "base",
      * might also return "sse2", "avx2", "openssl", or some other arbitrary string.
      */
      virtual std::string provider() const { return "base"; }

      virtual ~HashFunction() {}

      /**
      * Reset the state.
      */
      virtual void clear() = 0;

      /**
      * @return the hash function name
      */
      virtual std::string name() const = 0;

      /**
      * @return hash block size as defined for this algorithm
      */
      virtual size_t hash_block_size() const { return 0; }
   };

}


namespace Botan {

/**
* The Adler32 checksum, used in zlib
*/
class BOTAN_DLL Adler32 final : public HashFunction
   {
   public:
      std::string name() const override { return "Adler32"; }
      size_t output_length() const override { return 4; }
      HashFunction* clone() const override { return new Adler32; }

      void clear() override { m_S1 = 1; m_S2 = 0; }

      Adler32() { clear(); }
      ~Adler32() { clear(); }
   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      uint16_t m_S1, m_S2;
   };

}


namespace Botan {

/**
* Represents the length requirements on an algorithm key
*/
class BOTAN_DLL Key_Length_Specification
   {
   public:
      /**
      * Constructor for fixed length keys
      * @param keylen the supported key length
      */
      explicit Key_Length_Specification(size_t keylen) :
         m_min_keylen(keylen),
         m_max_keylen(keylen),
         m_keylen_mod(1)
         {
         }

      /**
      * Constructor for variable length keys
      * @param min_k the smallest supported key length
      * @param max_k the largest supported key length
      * @param k_mod the number of bytes the key must be a multiple of
      */
      Key_Length_Specification(size_t min_k,
                               size_t max_k,
                               size_t k_mod = 1) :
         m_min_keylen(min_k),
         m_max_keylen(max_k ? max_k : min_k),
         m_keylen_mod(k_mod)
         {
         }

      /**
      * @param length is a key length in bytes
      * @return true iff this length is a valid length for this algo
      */
      bool valid_keylength(size_t length) const
         {
         return ((length >= m_min_keylen) &&
                 (length <= m_max_keylen) &&
                 (length % m_keylen_mod == 0));
         }

      /**
      * @return minimum key length in bytes
      */
      size_t minimum_keylength() const
         {
         return m_min_keylen;
         }

      /**
      * @return maximum key length in bytes
      */
      size_t maximum_keylength() const
         {
         return m_max_keylen;
         }

      /**
      * @return key length multiple in bytes
      */
      size_t keylength_multiple() const
         {
         return m_keylen_mod;
         }

      /*
      * Multiplies all length requirements with the given factor
      * @param n the multiplication factor
      * @return a key length specification multiplied by the factor
      */
      Key_Length_Specification multiple(size_t n) const
         {
         return Key_Length_Specification(n * m_min_keylen,
                                         n * m_max_keylen,
                                         n * m_keylen_mod);
         }

   private:
      size_t m_min_keylen, m_max_keylen, m_keylen_mod;
   };

}



namespace Botan {

/**
* Parse a SCAN-style algorithm name
* @param scan_name the name
* @return the name components
*/
BOTAN_DLL std::vector<std::string>
parse_algorithm_name(const std::string& scan_name);

/**
* Split a string
* @param str the input string
* @param delim the delimitor
* @return string split by delim
*/
BOTAN_DLL std::vector<std::string> split_on(
   const std::string& str, char delim);

/**
* Split a string on a character predicate
* @param str the input string
* @param pred the predicate
*/
BOTAN_DLL std::vector<std::string>
split_on_pred(const std::string& str,
              std::function<bool (char)> pred);

/**
* Erase characters from a string
*/
BOTAN_DLL std::string erase_chars(const std::string& str, const std::set<char>& chars);

/**
* Replace a character in a string
* @param str the input string
* @param from_char the character to replace
* @param to_char the character to replace it with
* @return str with all instances of from_char replaced by to_char
*/
BOTAN_DLL std::string replace_char(const std::string& str,
                                   char from_char,
                                   char to_char);

/**
* Replace a character in a string
* @param str the input string
* @param from_chars the characters to replace
* @param to_char the character to replace it with
* @return str with all instances of from_chars replaced by to_char
*/
BOTAN_DLL std::string replace_chars(const std::string& str,
                                    const std::set<char>& from_chars,
                                    char to_char);

/**
* Join a string
* @param strs strings to join
* @param delim the delimitor
* @return string joined by delim
*/
BOTAN_DLL std::string string_join(const std::vector<std::string>& strs,
                                  char delim);

/**
* Parse an ASN.1 OID
* @param oid the OID in string form
* @return OID components
*/
BOTAN_DLL std::vector<uint32_t> parse_asn1_oid(const std::string& oid);

/**
* Compare two names using the X.509 comparison algorithm
* @param name1 the first name
* @param name2 the second name
* @return true if name1 is the same as name2 by the X.509 comparison rules
*/
BOTAN_DLL bool x500_name_cmp(const std::string& name1,
                             const std::string& name2);

/**
* Convert a string to a number
* @param str the string to convert
* @return number value of the string
*/
BOTAN_DLL uint32_t to_u32bit(const std::string& str);

/**
* Convert a time specification to a number
* @param timespec the time specification
* @return number of seconds represented by timespec
*/
BOTAN_DLL uint32_t timespec_to_u32bit(const std::string& timespec);

/**
* Convert a string representation of an IPv4 address to a number
* @param ip_str the string representation
* @return integer IPv4 address
*/
BOTAN_DLL uint32_t string_to_ipv4(const std::string& ip_str);

/**
* Convert an IPv4 address to a string
* @param ip_addr the IPv4 address to convert
* @return string representation of the IPv4 address
*/
BOTAN_DLL std::string ipv4_to_string(uint32_t ip_addr);

std::map<std::string, std::string> BOTAN_DLL read_cfg(std::istream& is);

std::string BOTAN_DLL clean_ws(const std::string& s);

bool BOTAN_DLL host_wildcard_match(const std::string& wildcard, const std::string& host);


}


namespace Botan {

/**
* Base class for all exceptions thrown by the library
*/
class BOTAN_DLL Exception : public std::exception
   {
   public:
      explicit Exception(const std::string& msg) : m_msg(msg) {}
      Exception(const char* prefix, const std::string& msg) : m_msg(std::string(prefix) + " " + msg) {}
      const char* what() const BOTAN_NOEXCEPT override { return m_msg.c_str(); }
   private:
      std::string m_msg;
   };

/**
* An invalid argument
*/
class BOTAN_DLL Invalid_Argument : public Exception
   {
   public:
      explicit Invalid_Argument(const std::string& msg) :
         Exception("Invalid argument", msg) {}

      explicit Invalid_Argument(const std::string& msg, const std::string& where) :
         Exception("Invalid argument", msg + " in " + where) {}
};

#define BOTAN_ARG_CHECK(expr) \
   do { if(!(expr)) throw Invalid_Argument(#expr, BOTAN_CURRENT_FUNCTION); } while(0)

/**
* Unsupported_Argument Exception
*
* An argument that is invalid because it is not supported by Botan.
* It might or might not be valid in another context like a standard.
*/
struct BOTAN_DLL Unsupported_Argument : public Invalid_Argument
   {
   explicit Unsupported_Argument(const std::string& msg) : Invalid_Argument(msg) {}
   };

/**
* Invalid_State Exception
*/
struct BOTAN_DLL Invalid_State : public Exception
   {
   explicit Invalid_State(const std::string& err) :
      Exception(err)
      {}
   };

/**
* Lookup_Error Exception
*/
struct BOTAN_DLL Lookup_Error : public Exception
   {
   explicit Lookup_Error(const std::string& err) :
      Exception(err)
      {}

   Lookup_Error(const std::string& type,
                const std::string& algo,
                const std::string& provider) :
      Exception("Unavailable " + type + " " + algo +
                (provider.empty() ? std::string("") : (" for provider " + provider)))
      {}
   };

/**
* Internal_Error Exception
*/
struct BOTAN_DLL Internal_Error : public Exception
   {
   explicit Internal_Error(const std::string& err) :
      Exception("Internal error: " + err)
      {}
   };

/**
* Invalid_Key_Length Exception
*/
struct BOTAN_DLL Invalid_Key_Length : public Invalid_Argument
   {
   Invalid_Key_Length(const std::string& name, size_t length) :
      Invalid_Argument(name + " cannot accept a key of length " +
                       std::to_string(length))
      {}
   };

/**
* Invalid_IV_Length Exception
*/
struct BOTAN_DLL Invalid_IV_Length : public Invalid_Argument
   {
   Invalid_IV_Length(const std::string& mode, size_t bad_len) :
      Invalid_Argument("IV length " + std::to_string(bad_len) +
                       " is invalid for " + mode)
      {}
   };

/**
* PRNG_Unseeded Exception
*/
struct BOTAN_DLL PRNG_Unseeded : public Invalid_State
   {
   explicit PRNG_Unseeded(const std::string& algo) :
      Invalid_State("PRNG not seeded: " + algo)
      {}
   };

/**
* Policy_Violation Exception
*/
struct BOTAN_DLL Policy_Violation : public Invalid_State
   {
   explicit Policy_Violation(const std::string& err) :
      Invalid_State("Policy violation: " + err)
      {}
   };

/**
* Algorithm_Not_Found Exception
*/
struct BOTAN_DLL Algorithm_Not_Found : public Lookup_Error
   {
   explicit Algorithm_Not_Found(const std::string& name) :
      Lookup_Error("Could not find any algorithm named \"" + name + "\"")
      {}
   };

/**
* No_Provider_Found Exception
*/
struct BOTAN_DLL No_Provider_Found : public Exception
   {
   explicit No_Provider_Found(const std::string& name) :
      Exception("Could not find any provider for algorithm named \"" + name + "\"")
      {}
   };

/**
* Provider_Not_Found is thrown when a specific provider was requested
* but that provider is not available.
*/
struct BOTAN_DLL Provider_Not_Found : public Lookup_Error
   {
   Provider_Not_Found(const std::string& algo, const std::string& provider) :
      Lookup_Error("Could not find provider '" + provider + "' for " + algo) {}
   };

/**
* Invalid_Algorithm_Name Exception
*/
struct BOTAN_DLL Invalid_Algorithm_Name : public Invalid_Argument
   {
   explicit Invalid_Algorithm_Name(const std::string& name):
      Invalid_Argument("Invalid algorithm name: " + name)
      {}
   };

/**
* Encoding_Error Exception
*/
struct BOTAN_DLL Encoding_Error : public Invalid_Argument
   {
   explicit Encoding_Error(const std::string& name) :
      Invalid_Argument("Encoding error: " + name) {}
   };

/**
* Decoding_Error Exception
*/
struct BOTAN_DLL Decoding_Error : public Invalid_Argument
   {
   explicit Decoding_Error(const std::string& name) :
      Invalid_Argument("Decoding error: " + name) {}
   };

/**
* Integrity_Failure Exception
*/
struct BOTAN_DLL Integrity_Failure : public Exception
   {
   explicit Integrity_Failure(const std::string& msg) :
      Exception("Integrity failure: " + msg) {}
   };

/**
* Invalid_OID Exception
*/
struct BOTAN_DLL Invalid_OID : public Decoding_Error
   {
   explicit Invalid_OID(const std::string& oid) :
      Decoding_Error("Invalid ASN.1 OID: " + oid) {}
   };

/**
* Stream_IO_Error Exception
*/
struct BOTAN_DLL Stream_IO_Error : public Exception
   {
   explicit Stream_IO_Error(const std::string& err) :
      Exception("I/O error: " + err)
      {}
   };

/**
* No_Filesystem_Access Exception
*/
struct BOTAN_DLL No_Filesystem_Access : public Exception
   {
   No_Filesystem_Access() : Exception("No filesystem access enabled.") {}
   };

/**
* Self Test Failure Exception
*/
struct BOTAN_DLL Self_Test_Failure : public Internal_Error
   {
   explicit Self_Test_Failure(const std::string& err) :
      Internal_Error("Self test failed: " + err)
      {}
   };

/**
* Not Implemented Exception
*/
struct BOTAN_DLL Not_Implemented : public Exception
   {
   explicit Not_Implemented(const std::string& err) :
      Exception("Not implemented", err)
      {}
   };

}


namespace Botan {

/**
* Octet String
*/
class BOTAN_DLL OctetString
   {
   public:
      /**
      * @return size of this octet string in bytes
      */
      size_t length() const { return m_data.size(); }
      size_t size() const { return m_data.size(); }

      /**
      * @return this object as a secure_vector<uint8_t>
      */
      secure_vector<uint8_t> bits_of() const { return m_data; }

      /**
      * @return start of this string
      */
      const uint8_t* begin() const { return m_data.data(); }

      /**
      * @return end of this string
      */
      const uint8_t* end() const   { return begin() + m_data.size(); }

      /**
      * @return this encoded as hex
      */
      std::string as_string() const;

      /**
      * XOR the contents of another octet string into this one
      * @param other octet string
      * @return reference to this
      */
      OctetString& operator^=(const OctetString& other);

      /**
      * Force to have odd parity
      */
      void set_odd_parity();

      /**
      * Create a new OctetString
      * @param str is a hex encoded string
      */
      explicit OctetString(const std::string& str = "");

      /**
      * Create a new random OctetString
      * @param rng is a random number generator
      * @param len is the desired length in bytes
      */
      OctetString(class RandomNumberGenerator& rng, size_t len);

      /**
      * Create a new OctetString
      * @param in is an array
      * @param len is the length of in in bytes
      */
      OctetString(const uint8_t in[], size_t len);

      /**
      * Create a new OctetString
      * @param in a bytestring
      */
      OctetString(const secure_vector<uint8_t>& in) : m_data(in) {}

      /**
      * Create a new OctetString
      * @param in a bytestring
      */
      OctetString(const std::vector<uint8_t>& in) : m_data(in.begin(), in.end()) {}

   private:
      secure_vector<uint8_t> m_data;
   };

/**
* Compare two strings
* @param x an octet string
* @param y an octet string
* @return if x is equal to y
*/
BOTAN_DLL bool operator==(const OctetString& x,
                          const OctetString& y);

/**
* Compare two strings
* @param x an octet string
* @param y an octet string
* @return if x is not equal to y
*/
BOTAN_DLL bool operator!=(const OctetString& x,
                          const OctetString& y);

/**
* Concatenate two strings
* @param x an octet string
* @param y an octet string
* @return x concatenated with y
*/
BOTAN_DLL OctetString operator+(const OctetString& x,
                                const OctetString& y);

/**
* XOR two strings
* @param x an octet string
* @param y an octet string
* @return x XORed with y
*/
BOTAN_DLL OctetString operator^(const OctetString& x,
                                const OctetString& y);


/**
* Alternate name for octet string showing intent to use as a key
*/
using SymmetricKey = OctetString;

/**
* Alternate name for octet string showing intent to use as an IV
*/
using InitializationVector = OctetString;

}


namespace Botan {

/**
* Interface for cipher modes
*/
class BOTAN_DLL Cipher_Mode
   {
   public:
      virtual ~Cipher_Mode() {}

      /*
      * Prepare for processing a message under the specified nonce
      */
      virtual void start_msg(const uint8_t nonce[], size_t nonce_len) = 0;

      /**
      * Begin processing a message.
      * @param nonce the per message nonce
      */
      template<typename Alloc>
      void start(const std::vector<uint8_t, Alloc>& nonce)
         {
         start_msg(nonce.data(), nonce.size());
         }

      /**
      * Begin processing a message.
      * @param nonce the per message nonce
      * @param nonce_len length of nonce
      */
      void start(const uint8_t nonce[], size_t nonce_len)
         {
         start_msg(nonce, nonce_len);
         }

      /**
      * Begin processing a message.
      */
      void start()
         {
         return start_msg(nullptr, 0);
         }

      /**
      * Process message blocks
      *
      * Input must be a multiple of update_granularity
      *
      * Processes msg in place and returns bytes written. Normally
      * this will be either msg_len (indicating the entire message was
      * processed) or for certain AEAD modes zero (indicating that the
      * mode requires the entire message be processed in one pass).
      *
      * @param msg the message to be processed
      * @param msg_len length of the message in bytes
      */
      virtual size_t process(uint8_t msg[], size_t msg_len) = 0;

      /**
      * Process some data. Input must be in size update_granularity() uint8_t blocks.
      * @param buffer in/out parameter which will possibly be resized
      * @param offset an offset into blocks to begin processing
      */
      void update(secure_vector<uint8_t>& buffer, size_t offset = 0)
         {
         BOTAN_ASSERT(buffer.size() >= offset, "Offset ok");
         uint8_t* buf = buffer.data() + offset;
         const size_t buf_size = buffer.size() - offset;

         const size_t written = process(buf, buf_size);
         buffer.resize(offset + written);
         }

      /**
      * Complete processing of a message.
      *
      * @param final_block in/out parameter which must be at least
      *        minimum_final_size() bytes, and will be set to any final output
      * @param offset an offset into final_block to begin processing
      */
      virtual void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) = 0;

      /**
      * Returns the size of the output if this transform is used to process a
      * message with input_length bytes. Will throw if unable to give a precise
      * answer.
      */
      virtual size_t output_length(size_t input_length) const = 0;

      /**
      * @return size of required blocks to update
      */
      virtual size_t update_granularity() const = 0;

      /**
      * @return required minimium size to finalize() - may be any
      *         length larger than this.
      */
      virtual size_t minimum_final_size() const = 0;

      /**
      * @return the default size for a nonce
      */
      virtual size_t default_nonce_length() const = 0;

      /**
      * @return true iff nonce_len is a valid length for the nonce
      */
      virtual bool valid_nonce_length(size_t nonce_len) const = 0;

      virtual std::string name() const = 0;

      /**
      * Zeroise all state
      * See also reset_msg()
      */
      virtual void clear() = 0;

      /**
      * Resets just the message specific state and allows encrypting again under the existing key
      */
      virtual void reset() = 0;

      /**
      * @return true iff this mode provides authentication as well as
      * confidentiality.
      */
      virtual bool authenticated() const { return false; }

      /**
      * @return the size of the authentication tag used (in bytes)
      */
      virtual size_t tag_size() const { return 0; }

      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * Check whether a given key length is valid for this algorithm.
      * @param length the key length to be checked.
      * @return true if the key length is valid.
      */
      bool valid_keylength(size_t length) const
         {
         return key_spec().valid_keylength(length);
         }

      /**
      * Set the symmetric key of this transform
      * @param key contains the key material
      */
      template<typename Alloc>
      void set_key(const std::vector<uint8_t, Alloc>& key)
         {
         set_key(key.data(), key.size());
         }

      /**
      * Set the symmetric key of this transform
      * @param key contains the key material
      */
      void set_key(const SymmetricKey& key)
         {
         set_key(key.begin(), key.length());
         }

      /**
      * Set the symmetric key of this transform
      * @param key contains the key material
      * @param length in bytes of key param
      */
      void set_key(const uint8_t key[], size_t length)
         {
         if(!valid_keylength(length))
            throw Invalid_Key_Length(name(), length);
         key_schedule(key, length);
         }

      /**
      * @return provider information about this implementation. Default is "base",
      * might also return "sse2", "avx2", "openssl", or some other arbitrary string.
      */
      virtual std::string provider() const { return "base"; }

   private:
      virtual void key_schedule(const uint8_t key[], size_t length) = 0;
   };

/**
* The two possible directions for cipher filters, determining whether they
* actually perform encryption or decryption.
*/
enum Cipher_Dir { ENCRYPTION, DECRYPTION };

/**
* Get a cipher mode by name (eg "AES-128/CBC" or "Serpent/XTS")
* @param algo_spec cipher name
* @param direction ENCRYPTION or DECRYPTION
*/
BOTAN_DLL Cipher_Mode* get_cipher_mode(const std::string& algo_spec, Cipher_Dir direction);

}


namespace Botan {

/**
* Interface for AEAD (Authenticated Encryption with Associated Data)
* modes. These modes provide both encryption and message
* authentication, and can authenticate additional per-message data
* which is not included in the ciphertext (for instance a sequence
* number).
*/
class BOTAN_DLL AEAD_Mode : public Cipher_Mode
   {
   public:
      bool authenticated() const override { return true; }

      /**
      * Set associated data that is not included in the ciphertext but
      * that should be authenticated. Must be called after set_key and
      * before start.
      *
      * Unless reset by another call, the associated data is kept
      * between messages. Thus, if the AD does not change, calling
      * once (after set_key) is the optimum.
      *
      * @param ad the associated data
      * @param ad_len length of add in bytes
      */
      virtual void set_associated_data(const uint8_t ad[], size_t ad_len) = 0;

      /**
      * Set associated data that is not included in the ciphertext but
      * that should be authenticated. Must be called after set_key and
      * before start.
      *
      * See @ref set_associated_data().
      *
      * @param ad the associated data
      */
      template<typename Alloc>
      void set_associated_data_vec(const std::vector<uint8_t, Alloc>& ad)
         {
         set_associated_data(ad.data(), ad.size());
         }

      /**
      * Set associated data that is not included in the ciphertext but
      * that should be authenticated. Must be called after set_key and
      * before start.
      *
      * See @ref set_associated_data().
      *
      * @param ad the associated data
      */
      template<typename Alloc>
      void set_ad(const std::vector<uint8_t, Alloc>& ad)
         {
         set_associated_data(ad.data(), ad.size());
         }

      /**
      * @return default AEAD nonce size (a commonly supported value among AEAD
      * modes, and large enough that random collisions are unlikely)
      */
      size_t default_nonce_length() const override { return 12; }

      virtual ~AEAD_Mode() {}
   };

/**
* Get an AEAD mode by name (eg "AES-128/GCM" or "Serpent/EAX")
* @param name AEAD name
* @param direction ENCRYPTION or DECRYPTION
*/
BOTAN_DLL AEAD_Mode* get_aead(const std::string& name, Cipher_Dir direction);

}


namespace Botan {

/**
* This class represents a symmetric algorithm object.
*/
class BOTAN_DLL SymmetricAlgorithm
   {
   public:
      virtual ~SymmetricAlgorithm() {}

      /**
      * Reset the state.
      */
      virtual void clear() = 0;

      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * @return minimum allowed key length
      */
      size_t maximum_keylength() const
         {
         return key_spec().maximum_keylength();
         }

      /**
      * @return maximum allowed key length
      */
      size_t minimum_keylength() const
         {
         return key_spec().minimum_keylength();
         }

      /**
      * Check whether a given key length is valid for this algorithm.
      * @param length the key length to be checked.
      * @return true if the key length is valid.
      */
      bool valid_keylength(size_t length) const
         {
         return key_spec().valid_keylength(length);
         }

      /**
      * Set the symmetric key of this object.
      * @param key the SymmetricKey to be set.
      */
      void set_key(const SymmetricKey& key)
         {
         set_key(key.begin(), key.length());
         }

      template<typename Alloc>
      void set_key(const std::vector<uint8_t, Alloc>& key)
         {
         set_key(key.data(), key.size());
         }

      /**
      * Set the symmetric key of this object.
      * @param key the to be set as a byte array.
      * @param length in bytes of key param
      */
      void set_key(const uint8_t key[], size_t length)
         {
         if(!valid_keylength(length))
            throw Invalid_Key_Length(name(), length);
         key_schedule(key, length);
         }

      /**
      * @return the algorithm name
      */
      virtual std::string name() const = 0;

   private:
      /**
      * Run the key schedule
      * @param key the key
      * @param length of key
      */
      virtual void key_schedule(const uint8_t key[], size_t length) = 0;
   };

}


namespace Botan {

/**
* This class represents a block cipher object.
*/
class BOTAN_DLL BlockCipher : public SymmetricAlgorithm
   {
   public:

      /**
      * Create an instance based on a name
      * If provider is empty then best available is chosen.
      * @param algo_spec algorithm name
      * @param provider provider implementation to choose
      * @return a null pointer if the algo/provider combination cannot be found
      */
      static std::unique_ptr<BlockCipher>
         create(const std::string& algo_spec,
                const std::string& provider = "");

      /**
      * Create an instance based on a name, or throw if the
      * algo/provider combination cannot be found. If provider is
      * empty then best available is chosen.
      */
      static std::unique_ptr<BlockCipher>
         create_or_throw(const std::string& algo_spec,
                         const std::string& provider = "");

      /**
      * @return list of available providers for this algorithm, empty if not available
      * @param algo_spec algorithm name
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      /**
      * @return block size of this algorithm
      */
      virtual size_t block_size() const = 0;

      /**
      * @return native parallelism of this cipher in blocks
      */
      virtual size_t parallelism() const { return 1; }

      /**
      * @return prefererred parallelism of this cipher in bytes
      */
      size_t parallel_bytes() const
         {
         return parallelism() * block_size() * BOTAN_BLOCK_CIPHER_PAR_MULT;
         }

      /**
      * @return provider information about this implementation. Default is "base",
      * might also return "sse2", "avx2", "openssl", or some other arbitrary string.
      */
      virtual std::string provider() const { return "base"; }

      /**
      * Encrypt a block.
      * @param in The plaintext block to be encrypted as a byte array.
      * Must be of length block_size().
      * @param out The byte array designated to hold the encrypted block.
      * Must be of length block_size().
      */
      void encrypt(const uint8_t in[], uint8_t out[]) const
         { encrypt_n(in, out, 1); }

      /**
      * Decrypt a block.
      * @param in The ciphertext block to be decypted as a byte array.
      * Must be of length block_size().
      * @param out The byte array designated to hold the decrypted block.
      * Must be of length block_size().
      */
      void decrypt(const uint8_t in[], uint8_t out[]) const
         { decrypt_n(in, out, 1); }

      /**
      * Encrypt a block.
      * @param block the plaintext block to be encrypted
      * Must be of length block_size(). Will hold the result when the function
      * has finished.
      */
      void encrypt(uint8_t block[]) const { encrypt_n(block, block, 1); }

      /**
      * Decrypt a block.
      * @param block the ciphertext block to be decrypted
      * Must be of length block_size(). Will hold the result when the function
      * has finished.
      */
      void decrypt(uint8_t block[]) const { decrypt_n(block, block, 1); }

      /**
      * Encrypt one or more blocks
      * @param block the input/output buffer (multiple of block_size())
      */
      template<typename Alloc>
      void encrypt(std::vector<uint8_t, Alloc>& block) const
         {
         return encrypt_n(block.data(), block.data(), block.size() / block_size());
         }

      /**
      * Decrypt one or more blocks
      * @param block the input/output buffer (multiple of block_size())
      */
      template<typename Alloc>
      void decrypt(std::vector<uint8_t, Alloc>& block) const
         {
         return decrypt_n(block.data(), block.data(), block.size() / block_size());
         }

      /**
      * Encrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      */
      template<typename Alloc, typename Alloc2>
      void encrypt(const std::vector<uint8_t, Alloc>& in,
                   std::vector<uint8_t, Alloc2>& out) const
         {
         return encrypt_n(in.data(), out.data(), in.size() / block_size());
         }

      /**
      * Decrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      */
      template<typename Alloc, typename Alloc2>
      void decrypt(const std::vector<uint8_t, Alloc>& in,
                   std::vector<uint8_t, Alloc2>& out) const
         {
         return decrypt_n(in.data(), out.data(), in.size() / block_size());
         }

      /**
      * Encrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      * @param blocks the number of blocks to process
      */
      virtual void encrypt_n(const uint8_t in[], uint8_t out[],
                             size_t blocks) const = 0;

      /**
      * Decrypt one or more blocks
      * @param in the input buffer (multiple of block_size())
      * @param out the output buffer (same size as in)
      * @param blocks the number of blocks to process
      */
      virtual void decrypt_n(const uint8_t in[], uint8_t out[],
                             size_t blocks) const = 0;

      /**
      * @return new object representing the same algorithm as *this
      */
      virtual BlockCipher* clone() const = 0;

      virtual ~BlockCipher() {}
   };

/**
* Represents a block cipher with a single fixed block size
*/
template<size_t BS, size_t KMIN, size_t KMAX = 0, size_t KMOD = 1>
class Block_Cipher_Fixed_Params : public BlockCipher
   {
   public:
      enum { BLOCK_SIZE = BS };
      size_t block_size() const override { return BS; }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(KMIN, KMAX, KMOD);
         }
   };

}


namespace Botan {

/**
* AES-128
*/
class BOTAN_DLL AES_128 final : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string provider() const override;
      std::string name() const override { return "AES-128"; }
      BlockCipher* clone() const override { return new AES_128; }
   private:
      void key_schedule(const uint8_t key[], size_t length) override;

#if defined(BOTAN_HAS_AES_SSSE3)
      void ssse3_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void ssse3_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void ssse3_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_NI)
      void aesni_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void aesni_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void aesni_key_schedule(const uint8_t key[], size_t length);
#endif

      secure_vector<uint32_t> m_EK, m_DK;
      secure_vector<uint8_t> m_ME, m_MD;
   };

/**
* AES-192
*/
class BOTAN_DLL AES_192 final : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string provider() const override;
      std::string name() const override { return "AES-192"; }
      BlockCipher* clone() const override { return new AES_192; }
   private:
#if defined(BOTAN_HAS_AES_SSSE3)
      void ssse3_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void ssse3_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void ssse3_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_NI)
      void aesni_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void aesni_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void aesni_key_schedule(const uint8_t key[], size_t length);
#endif

      void key_schedule(const uint8_t key[], size_t length) override;

      secure_vector<uint32_t> m_EK, m_DK;
      secure_vector<uint8_t> m_ME, m_MD;
   };

/**
* AES-256
*/
class BOTAN_DLL AES_256 final : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string provider() const override;

      std::string name() const override { return "AES-256"; }
      BlockCipher* clone() const override { return new AES_256; }
   private:
#if defined(BOTAN_HAS_AES_SSSE3)
      void ssse3_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void ssse3_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void ssse3_key_schedule(const uint8_t key[], size_t length);
#endif

#if defined(BOTAN_HAS_AES_NI)
      void aesni_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void aesni_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void aesni_key_schedule(const uint8_t key[], size_t length);
#endif

      void key_schedule(const uint8_t key[], size_t length) override;

      secure_vector<uint32_t> m_EK, m_DK;
      secure_vector<uint8_t> m_ME, m_MD;
   };

}


namespace Botan {

class BER_Decoder;
class DER_Encoder;

/**
* ASN.1 Type and Class Tags
*/
enum ASN1_Tag {
   UNIVERSAL        = 0x00,
   APPLICATION      = 0x40,
   CONTEXT_SPECIFIC = 0x80,

   CONSTRUCTED      = 0x20,

   PRIVATE          = CONSTRUCTED | CONTEXT_SPECIFIC,

   EOC              = 0x00,
   BOOLEAN          = 0x01,
   INTEGER          = 0x02,
   BIT_STRING       = 0x03,
   OCTET_STRING     = 0x04,
   NULL_TAG         = 0x05,
   OBJECT_ID        = 0x06,
   ENUMERATED       = 0x0A,
   SEQUENCE         = 0x10,
   SET              = 0x11,

   UTF8_STRING      = 0x0C,
   NUMERIC_STRING   = 0x12,
   PRINTABLE_STRING = 0x13,
   T61_STRING       = 0x14,
   IA5_STRING       = 0x16,
   VISIBLE_STRING   = 0x1A,
   BMP_STRING       = 0x1E,

   UTC_TIME                = 0x17,
   GENERALIZED_TIME        = 0x18,
   UTC_OR_GENERALIZED_TIME = 0x19,

   NO_OBJECT        = 0xFF00,
   DIRECTORY_STRING = 0xFF01
};

/**
* Basic ASN.1 Object Interface
*/
class BOTAN_DLL ASN1_Object
   {
   public:
      /**
      * Encode whatever this object is into to
      * @param to the DER_Encoder that will be written to
      */
      virtual void encode_into(DER_Encoder& to) const = 0;

      /**
      * Decode whatever this object is from from
      * @param from the BER_Decoder that will be read from
      */
      virtual void decode_from(BER_Decoder& from) = 0;

      virtual ~ASN1_Object() {}
   };

/**
* BER Encoded Object
*/
class BOTAN_DLL BER_Object
   {
   public:
      void assert_is_a(ASN1_Tag, ASN1_Tag);

      // public member variable:
      ASN1_Tag type_tag, class_tag;

      // public member variable:
      secure_vector<uint8_t> value;
   };

/*
* ASN.1 Utility Functions
*/
class DataSource;

namespace ASN1 {

std::vector<uint8_t> put_in_sequence(const std::vector<uint8_t>& val);
std::string to_string(const BER_Object& obj);

/**
* Heuristics tests; is this object possibly BER?
* @param src a data source that will be peeked at but not modified
*/
bool maybe_BER(DataSource& src);

}

/**
* General BER Decoding Error Exception
*/
struct BOTAN_DLL BER_Decoding_Error : public Decoding_Error
   {
   explicit BER_Decoding_Error(const std::string&);
   };

/**
* Exception For Incorrect BER Taggings
*/
struct BOTAN_DLL BER_Bad_Tag : public BER_Decoding_Error
   {
   BER_Bad_Tag(const std::string& msg, ASN1_Tag tag);
   BER_Bad_Tag(const std::string& msg, ASN1_Tag tag1, ASN1_Tag tag2);
   };

}


namespace Botan {

/**
* This class represents ASN.1 object identifiers.
*/
class BOTAN_DLL OID final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      /**
      * Find out whether this OID is empty
      * @return true is no OID value is set
      */
      bool empty() const { return m_id.size() == 0; }

      /**
      * Get this OID as list (vector) of its components.
      * @return vector representing this OID
      */
      const std::vector<uint32_t>& get_id() const { return m_id; }

      /**
      * Get this OID as a string
      * @return string representing this OID
      */
      std::string as_string() const;

      /**
      * Compare two OIDs.
      * @return true if they are equal, false otherwise
      */
      bool operator==(const OID&) const;

      /**
      * Reset this instance to an empty OID.
      */
      void clear();

      /**
      * Add a component to this OID.
      * @param new_comp the new component to add to the end of this OID
      * @return reference to *this
      */
      OID& operator+=(uint32_t new_comp);

      /**
      * Construct an OID from a string.
      * @param str a string in the form "a.b.c" etc., where a,b,c are numbers
      */
      OID(const std::string& str = "");
   private:
      std::vector<uint32_t> m_id;
   };

/**
* Append another component onto the OID.
* @param oid the OID to add the new component to
* @param new_comp the new component to add
*/
OID BOTAN_DLL operator+(const OID& oid, uint32_t new_comp);

/**
* Compare two OIDs.
* @param a the first OID
* @param b the second OID
* @return true if a is not equal to b
*/
bool BOTAN_DLL operator!=(const OID& a, const OID& b);

/**
* Compare two OIDs.
* @param a the first OID
* @param b the second OID
* @return true if a is lexicographically smaller than b
*/
bool BOTAN_DLL operator<(const OID& a, const OID& b);

}


namespace Botan {

/**
* Algorithm Identifier
*/
class BOTAN_DLL AlgorithmIdentifier final : public ASN1_Object
   {
   public:
      enum Encoding_Option { USE_NULL_PARAM };

      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      AlgorithmIdentifier() {}
      AlgorithmIdentifier(const OID&, Encoding_Option);
      AlgorithmIdentifier(const std::string&, Encoding_Option);

      AlgorithmIdentifier(const OID&, const std::vector<uint8_t>&);
      AlgorithmIdentifier(const std::string&, const std::vector<uint8_t>&);

      // public member variable:
      OID oid;

      // public member variable:
      std::vector<uint8_t> parameters;
   };

/*
* Comparison Operations
*/
bool BOTAN_DLL operator==(const AlgorithmIdentifier&,
                          const AlgorithmIdentifier&);
bool BOTAN_DLL operator!=(const AlgorithmIdentifier&,
                          const AlgorithmIdentifier&);

}


namespace Botan {

/**
* Simple String
*/
class BOTAN_DLL ASN1_String final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      std::string value() const;
      std::string iso_8859() const;

      ASN1_Tag tagging() const;

      explicit ASN1_String(const std::string& = "");
      ASN1_String(const std::string&, ASN1_Tag);
   private:
      std::string m_iso_8859_str;
      ASN1_Tag m_tag;
   };

}


namespace Botan {

/**
* Alternative Name
*/
class BOTAN_DLL AlternativeName final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      std::multimap<std::string, std::string> contents() const;

      void add_attribute(const std::string&, const std::string&);
      std::multimap<std::string, std::string> get_attributes() const;

      void add_othername(const OID&, const std::string&, ASN1_Tag);
      std::multimap<OID, ASN1_String> get_othernames() const;

      bool has_items() const;

      AlternativeName(const std::string& = "", const std::string& = "",
                      const std::string& = "", const std::string& = "");
   private:
      std::multimap<std::string, std::string> m_alt_info;
      std::multimap<OID, ASN1_String> m_othernames;
   };

}


namespace Botan {

/**
* Attribute
*/
class BOTAN_DLL Attribute final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder& to) const override;
      void decode_from(class BER_Decoder& from) override;

      // public member variable:
      OID oid;

      // public member variable:
      std::vector<uint8_t> parameters;

      Attribute() {}
      Attribute(const OID&, const std::vector<uint8_t>&);
      Attribute(const std::string&, const std::vector<uint8_t>&);
   };

}


namespace Botan {

/**
* X.509 Time
*/
class BOTAN_DLL X509_Time final : public ASN1_Object
   {
   public:
      /// DER encode a X509_Time
      void encode_into(DER_Encoder&) const override;

      // Decode a BER encoded X509_Time
      void decode_from(BER_Decoder&) override;

      /// Return an internal string representation of the time
      std::string to_string() const;

      /// Returns a human friendly string replesentation of no particular formatting
      std::string readable_string() const;

      /// Return if the time has been set somehow
      bool time_is_set() const;

      ///  Compare this time against another
      int32_t cmp(const X509_Time& other) const;

      /// Create an invalid X509_Time
      X509_Time() {}

      /// Create a X509_Time from a time point
      explicit X509_Time(const std::chrono::system_clock::time_point& time);

      /// Create an X509_Time from string
      X509_Time(const std::string& t_spec, ASN1_Tag tag);

      /// Returns a STL timepoint object
      std::chrono::system_clock::time_point to_std_timepoint() const;

   private:
      void set_to(const std::string& t_spec, ASN1_Tag);
      bool passes_sanity_check() const;

      uint32_t m_year = 0;
      uint32_t m_month = 0;
      uint32_t m_day = 0;
      uint32_t m_hour = 0;
      uint32_t m_minute = 0;
      uint32_t m_second = 0;
      ASN1_Tag m_tag = NO_OBJECT;
   };

/*
* Comparison Operations
*/
bool BOTAN_DLL operator==(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator!=(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator<=(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator>=(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator<(const X509_Time&, const X509_Time&);
bool BOTAN_DLL operator>(const X509_Time&, const X509_Time&);

}


namespace Botan {

template <typename T>
/**
 * Simple helper class to expand std::atomic with copy constructor and copy
 * assignment operator, i.e. for use as element in a container like
 * std::vector. The construction of instances of this wrapper is NOT atomic
 * and needs to be properly guarded.
 **/
class Atomic
   {
   public:
      Atomic() : m_data() {};
      Atomic(const Atomic& data) : m_data(data.m_data.load()) {}
      Atomic(const std::atomic<T>& data) : m_data(data.load()) {}

      Atomic& operator=(const Atomic& a)
         {
         m_data.store(a.m_data.load());
         return *this;
         }

      Atomic& operator=(const std::atomic<T>& a)
         {
         m_data.store(a.load());
         return *this;
         }

      operator std::atomic<T>& () { return m_data; }
      operator T() { return m_data.load(); }

   private:
      std::atomic<T> m_data;
   };

}


namespace Botan {

class RandomNumberGenerator;

/**
* Abstract interface to a source of entropy
*/
class BOTAN_DLL Entropy_Source
   {
   public:
      /*
      * Return a new entropy source of a particular type, or null
      * Each entropy source may require substantial resources (eg, a file handle
      * or socket instance), so try to share them among multiple RNGs, or just
      * use the preconfigured global list accessed by global_entropy_sources()
      */
      static std::unique_ptr<Entropy_Source> create(const std::string& type);

      /**
      * @return name identifying this entropy source
      */
      virtual std::string name() const = 0;

      /**
      * Perform an entropy gathering poll
      * @param rng will be provided with entropy via calls to add_entropy
      @ @return conservative estimate of actual entropy added to rng during poll
      */
      virtual size_t poll(RandomNumberGenerator& rng) = 0;

      virtual ~Entropy_Source() {}
   };

class BOTAN_DLL Entropy_Sources final
   {
   public:
      static Entropy_Sources& global_sources();

      void add_source(std::unique_ptr<Entropy_Source> src);

      std::vector<std::string> enabled_sources() const;

      size_t poll(RandomNumberGenerator& rng,
                  size_t bits,
                  std::chrono::milliseconds timeout);

      /**
      * Poll just a single named source. Ordinally only used for testing
      */
      size_t poll_just(RandomNumberGenerator& rng, const std::string& src);

      Entropy_Sources() {}
      explicit Entropy_Sources(const std::vector<std::string>& sources);

      ~Entropy_Sources();
   private:
      std::vector<Entropy_Source*> m_srcs;
   };

}


namespace Botan {

class Entropy_Sources;

/**
* An interface to a cryptographic random number generator
*/
class BOTAN_DLL RandomNumberGenerator
   {
   public:
      virtual ~RandomNumberGenerator() = default;

      RandomNumberGenerator() = default;

      /*
      * Never copy a RNG, create a new one
      */
      RandomNumberGenerator(const RandomNumberGenerator& rng) = delete;
      RandomNumberGenerator& operator=(const RandomNumberGenerator& rng) = delete;

      /**
      * Randomize a byte array.
      * @param output the byte array to hold the random output.
      * @param length the length of the byte array output in bytes.
      */
      virtual void randomize(uint8_t output[], size_t length) = 0;

      /**
      * Incorporate some additional data into the RNG state. For
      * example adding nonces or timestamps from a peer's protocol
      * message can help hedge against VM state rollback attacks.
      * A few RNG types do not accept any externally provided input,
      * in which case this function is a no-op.
      *
      * @param input a byte array containg the entropy to be added
      * @param length the length of the byte array in
      */
      virtual void add_entropy(const uint8_t input[], size_t length) = 0;

      /**
      * Incorporate some additional data into the RNG state.
      */
      template<typename T> void add_entropy_T(const T& t)
         {
         this->add_entropy(reinterpret_cast<const uint8_t*>(&t), sizeof(T));
         }

      /**
      * Incorporate entropy into the RNG state then produce output.
      * Some RNG types implement this using a single operation, default
      * calls add_entropy + randomize in sequence.
      *
      * Use this to further bind the outputs to your current
      * process/protocol state. For instance if generating a new key
      * for use in a session, include a session ID or other such
      * value. See NIST SP 800-90 A, B, C series for more ideas.
      *
      * @param output buffer to hold the random output
      * @param output_len size of the output buffer in bytes
      * @param input entropy buffer to incorporate
      * @param input_len size of the input buffer in bytes
      */
      virtual void randomize_with_input(uint8_t output[], size_t output_len,
                                        const uint8_t input[], size_t input_len);

      /**
      * This calls `randomize_with_input` using some timestamps as extra input.
      *
      * For a stateful RNG using non-random but potentially unique data the
      * extra input can help protect against problems with fork, VM state
      * rollback, or other cases where somehow an RNG state is duplicated. If
      * both of the duplicated RNG states later incorporate a timestamp (and the
      * timestamps don't themselves repeat), their outputs will diverge.
      */
      virtual void randomize_with_ts_input(uint8_t output[], size_t output_len);

      /**
      * @return the name of this RNG type
      */
      virtual std::string name() const = 0;

      /**
      * Clear all internally held values of this RNG
      * @post is_seeded() == false
      */
      virtual void clear() = 0;

      /**
      * Check whether this RNG is seeded.
      * @return true if this RNG was already seeded, false otherwise.
      */
      virtual bool is_seeded() const = 0;

      /**
      * Poll provided sources for up to poll_bits bits of entropy
      * or until the timeout expires. Returns estimate of the number
      * of bits collected.
      */
      virtual size_t reseed(Entropy_Sources& srcs,
                            size_t poll_bits = BOTAN_RNG_RESEED_POLL_BITS,
                            std::chrono::milliseconds poll_timeout = BOTAN_RNG_RESEED_DEFAULT_TIMEOUT);

      /**
      * Reseed by reading specified bits from the RNG
      */
      virtual void reseed_from_rng(RandomNumberGenerator& rng,
                                   size_t poll_bits = BOTAN_RNG_RESEED_POLL_BITS);

      // Some utility functions built on the interface above:

      /**
      * Return a random vector
      * @param bytes number of bytes in the result
      * @return randomized vector of length bytes
      */
      secure_vector<uint8_t> random_vec(size_t bytes)
         {
         secure_vector<uint8_t> output(bytes);
         this->randomize(output.data(), output.size());
         return output;
         }

      /**
      * Return a random byte
      * @return random byte
      */
      uint8_t next_byte()
         {
         uint8_t b;
         this->randomize(&b, 1);
         return b;
         }

      /**
      * @return a random byte that is greater than zero
      */
      uint8_t next_nonzero_byte()
         {
         uint8_t b = this->next_byte();
         while(b == 0)
            b = this->next_byte();
         return b;
         }

      /**
      * Create a seeded and active RNG object for general application use
      * Added in 1.8.0
      * Use AutoSeeded_RNG instead
      */
      BOTAN_DEPRECATED("Use AutoSeeded_RNG")
      static RandomNumberGenerator* make_rng();
   };

/**
* Convenience typedef
*/
typedef RandomNumberGenerator RNG;

/**
* Hardware RNG has no members but exists to tag hardware RNG types
*/
class BOTAN_DLL Hardware_RNG : public RandomNumberGenerator
   {
   };

/**
* Null/stub RNG - fails if you try to use it for anything
* This is not generally useful except for in certain tests
*/
class BOTAN_DLL Null_RNG final : public RandomNumberGenerator
   {
   public:
      bool is_seeded() const override { return false; }

      void clear() override {}

      void randomize(uint8_t[], size_t) override
         {
         throw PRNG_Unseeded("Null_RNG called");
         }

      void add_entropy(const uint8_t[], size_t) override {}

      std::string name() const override { return "Null_RNG"; }
   };

#if defined(BOTAN_TARGET_OS_HAS_THREADS)
/**
* Wraps access to a RNG in a mutex
* Note that most of the time it's much better to use a RNG per thread
* otherwise the RNG will act as an unnecessary contention point
*/
class BOTAN_DLL Serialized_RNG final : public RandomNumberGenerator
   {
   public:
      void randomize(uint8_t out[], size_t len) override
         {
         lock_guard_type<mutex_type> lock(m_mutex);
         m_rng->randomize(out, len);
         }

      bool is_seeded() const override
         {
         lock_guard_type<mutex_type> lock(m_mutex);
         return m_rng->is_seeded();
         }

      void clear() override
         {
         lock_guard_type<mutex_type> lock(m_mutex);
         m_rng->clear();
         }

      std::string name() const override
         {
         lock_guard_type<mutex_type> lock(m_mutex);
         return m_rng->name();
         }

      size_t reseed(Entropy_Sources& src,
                    size_t poll_bits = BOTAN_RNG_RESEED_POLL_BITS,
                    std::chrono::milliseconds poll_timeout = BOTAN_RNG_RESEED_DEFAULT_TIMEOUT) override
         {
         lock_guard_type<mutex_type> lock(m_mutex);
         return m_rng->reseed(src, poll_bits, poll_timeout);
         }

      void add_entropy(const uint8_t in[], size_t len) override
         {
         lock_guard_type<mutex_type> lock(m_mutex);
         m_rng->add_entropy(in, len);
         }

      BOTAN_DEPRECATED("Use Serialized_RNG(new AutoSeeded_RNG)") Serialized_RNG();

      explicit Serialized_RNG(RandomNumberGenerator* rng) : m_rng(rng) {}
   private:
      mutable mutex_type m_mutex;
      std::unique_ptr<RandomNumberGenerator> m_rng;
   };
#endif

}


namespace Botan {

class Stateful_RNG;

/**
* A userspace PRNG
*/
class BOTAN_DLL AutoSeeded_RNG final : public RandomNumberGenerator
   {
   public:
      void randomize(uint8_t out[], size_t len) override;

      void randomize_with_input(uint8_t output[], size_t output_len,
                                const uint8_t input[], size_t input_len) override;

      bool is_seeded() const override;

      /**
      * Mark state as requiring a reseed on next use
      */
      void force_reseed();

      size_t reseed(Entropy_Sources& srcs,
                    size_t poll_bits = BOTAN_RNG_RESEED_POLL_BITS,
                    std::chrono::milliseconds poll_timeout = BOTAN_RNG_RESEED_DEFAULT_TIMEOUT) override;

      void add_entropy(const uint8_t in[], size_t len) override;

      std::string name() const override;

      void clear() override;

      /**
      * Uses the system RNG (if available) or else a default group of
      * entropy sources (all other systems) to gather seed material.
      *
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      */
      AutoSeeded_RNG(size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL);

      /**
      * Uses the BOTAN_AUTO_RNG_DRBG RNG to gather seed material.
      *
      * @param underlying_rng is a reference to some RNG which will be used
      * to perform the periodic reseeding
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      */
      AutoSeeded_RNG(RandomNumberGenerator& underlying_rng,
                     size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL);

      /**
      * Uses the BOTAN_AUTO_RNG_DRBG RNG to gather seed material.
      *
      * @param entropy_sources will be polled to perform reseeding periodically
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      */
      AutoSeeded_RNG(Entropy_Sources& entropy_sources,
                     size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL);

      /**
      * Uses the BOTAN_AUTO_RNG_DRBG RNG to gather seed material.
      *
      * @param underlying_rng is a reference to some RNG which will be used
      * to perform the periodic reseeding
      * @param entropy_sources will be polled to perform reseeding periodically
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      */
      AutoSeeded_RNG(RandomNumberGenerator& underlying_rng,
                     Entropy_Sources& entropy_sources,
                     size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL);

      ~AutoSeeded_RNG();

   private:
      std::unique_ptr<Stateful_RNG> m_rng;
   };

}


namespace Botan {

/**
* This class represents general abstract filter objects.
*/
class BOTAN_DLL Filter
   {
   public:
      /**
      * @return descriptive name for this filter
      */
      virtual std::string name() const = 0;

      /**
      * Write a portion of a message to this filter.
      * @param input the input as a byte array
      * @param length the length of the byte array input
      */
      virtual void write(const uint8_t input[], size_t length) = 0;

      /**
      * Start a new message. Must be closed by end_msg() before another
      * message can be started.
      */
      virtual void start_msg() {}

      /**
      * Notify that the current message is finished; flush buffers and
      * do end-of-message processing (if any).
      */
      virtual void end_msg() {}

      /**
      * Check whether this filter is an attachable filter.
      * @return true if this filter is attachable, false otherwise
      */
      virtual bool attachable() { return true; }

      virtual ~Filter() {}
   protected:
      /**
      * @param in some input for the filter
      * @param length the length of in
      */
      virtual void send(const uint8_t in[], size_t length);

      /**
      * @param in some input for the filter
      */
      void send(uint8_t in) { send(&in, 1); }

      /**
      * @param in some input for the filter
      */
      void send(const secure_vector<uint8_t>& in) { send(in.data(), in.size()); }

      /**
      * @param in some input for the filter
      */
      void send(const std::vector<uint8_t>& in) { send(in.data(), in.size()); }

      /**
      * @param in some input for the filter
      * @param length the number of bytes of in to send
      */
      void send(const secure_vector<uint8_t>& in, size_t length)
         {
         send(in.data(), length);
         }

      /**
      * @param in some input for the filter
      * @param length the number of bytes of in to send
      */
      void send(const std::vector<uint8_t>& in, size_t length)
         {
         send(in.data(), length);
         }

      Filter();

      Filter(const Filter&) = delete;

      Filter& operator=(const Filter&) = delete;

   private:
      /**
      * Start a new message in *this and all following filters. Only for
      * internal use, not intended for use in client applications.
      */
      void new_msg();

      /**
      * End a new message in *this and all following filters. Only for
      * internal use, not intended for use in client applications.
      */
      void finish_msg();

      friend class Pipe;
      friend class Fanout_Filter;

      size_t total_ports() const;
      size_t current_port() const { return m_port_num; }

      /**
      * Set the active port
      * @param new_port the new value
      */
      void set_port(size_t new_port);

      size_t owns() const { return m_filter_owns; }

      /**
      * Attach another filter to this one
      * @param f filter to attach
      */
      void attach(Filter* f);

      /**
      * @param filters the filters to set
      * @param count number of items in filters
      */
      void set_next(Filter* filters[], size_t count);
      Filter* get_next() const;

      secure_vector<uint8_t> m_write_queue;
      std::vector<Filter*> m_next;
      size_t m_port_num, m_filter_owns;

      // true if filter belongs to a pipe --> prohibit filter sharing!
      bool m_owned;
   };

/**
* This is the abstract Fanout_Filter base class.
**/
class BOTAN_DLL Fanout_Filter : public Filter
   {
   protected:
      /**
      * Increment the number of filters past us that we own
      */
      void incr_owns() { ++m_filter_owns; }

      void set_port(size_t n) { Filter::set_port(n); }

      void set_next(Filter* f[], size_t n) { Filter::set_next(f, n); }

      void attach(Filter* f) { Filter::attach(f); }

   private:
      friend class Threaded_Fork;
      using Filter::m_write_queue;
      using Filter::total_ports;
      using Filter::m_next;
   };

/**
* The type of checking to be performed by decoders:
* NONE - no checks, IGNORE_WS - perform checks, but ignore
* whitespaces, FULL_CHECK - perform checks, also complain
* about white spaces.
*/
enum Decoder_Checking { NONE, IGNORE_WS, FULL_CHECK };

}


namespace Botan {

/**
* This class represents a Base64 encoder.
*/
class BOTAN_DLL Base64_Encoder final : public Filter
   {
   public:
      std::string name() const override { return "Base64_Encoder"; }

      /**
      * Input a part of a message to the encoder.
      * @param input the message to input as a byte array
      * @param length the length of the byte array input
      */
      void write(const uint8_t input[], size_t length) override;

      /**
      * Inform the Encoder that the current message shall be closed.
      */
      void end_msg() override;

      /**
      * Create a base64 encoder.
      * @param breaks whether to use line breaks in the output
      * @param length the length of the lines of the output
      * @param t_n whether to use a trailing newline
      */
      Base64_Encoder(bool breaks = false, size_t length = 72,
                     bool t_n = false);
   private:
      void encode_and_send(const uint8_t input[], size_t length,
                           bool final_inputs = false);
      void do_output(const uint8_t output[], size_t length);

      const size_t m_line_length;
      const bool m_trailing_newline;
      std::vector<uint8_t> m_in, m_out;
      size_t m_position, m_out_position;
   };

/**
* This object represents a Base64 decoder.
*/
class BOTAN_DLL Base64_Decoder final : public Filter
   {
   public:
      std::string name() const override { return "Base64_Decoder"; }

      /**
      * Input a part of a message to the decoder.
      * @param input the message to input as a byte array
      * @param length the length of the byte array input
      */
      void write(const uint8_t input[], size_t length) override;

      /**
      * Finish up the current message
      */
      void end_msg() override;

      /**
      * Create a base64 decoder.
      * @param checking the type of checking that shall be performed by
      * the decoder
      */
      explicit Base64_Decoder(Decoder_Checking checking = NONE);
   private:
      const Decoder_Checking m_checking;
      std::vector<uint8_t> m_in, m_out;
      size_t m_position;
   };

}


namespace Botan {

/**
* Perform base64 encoding
* @param output an array of at least input_length*4/3 bytes
* @param input is some binary data
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding chars will be applied if needed
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_encode(char output[],
                               const uint8_t input[],
                               size_t input_length,
                               size_t& input_consumed,
                               bool final_inputs);

/**
* Perform base64 encoding
* @param input some input
* @param input_length length of input in bytes
* @return base64adecimal representation of input
*/
std::string BOTAN_DLL base64_encode(const uint8_t input[],
                                    size_t input_length);

/**
* Perform base64 encoding
* @param input some input
* @return base64adecimal representation of input
*/
template<typename Alloc>
std::string base64_encode(const std::vector<uint8_t, Alloc>& input)
   {
   return base64_encode(input.data(), input.size());
   }

/**
* Perform base64 decoding
* @param output an array of at least input_length*3/4 bytes
* @param input some base64 input
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param final_inputs true iff this is the last input, in which case
         padding is allowed
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_decode(uint8_t output[],
                               const char input[],
                               size_t input_length,
                               size_t& input_consumed,
                               bool final_inputs,
                               bool ignore_ws = true);

/**
* Perform base64 decoding
* @param output an array of at least input_length*3/4 bytes
* @param input some base64 input
* @param input_length length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_decode(uint8_t output[],
                               const char input[],
                               size_t input_length,
                               bool ignore_ws = true);

/**
* Perform base64 decoding
* @param output an array of at least input_length/3*4 bytes
* @param input some base64 input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL base64_decode(uint8_t output[],
                               const std::string& input,
                               bool ignore_ws = true);

/**
* Perform base64 decoding
* @param input some base64 input
* @param input_length the length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded base64 output
*/
secure_vector<uint8_t> BOTAN_DLL base64_decode(const char input[],
                                           size_t input_length,
                                           bool ignore_ws = true);

/**
* Perform base64 decoding
* @param input some base64 input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded base64 output
*/
secure_vector<uint8_t> BOTAN_DLL base64_decode(const std::string& input,
                                           bool ignore_ws = true);

}


#if defined(BOTAN_TARGET_OS_HAS_THREADS)
  #include <thread>
#endif

namespace Botan {

/**
* BitBucket is a filter which simply discards all inputs
*/
struct BOTAN_DLL BitBucket final : public Filter
   {
   void write(const uint8_t[], size_t) override {}

   std::string name() const override { return "BitBucket"; }
   };

/**
* This class represents Filter chains. A Filter chain is an ordered
* concatenation of Filters, the input to a Chain sequentially passes
* through all the Filters contained in the Chain.
*/

class BOTAN_DLL Chain : public Fanout_Filter
   {
   public:
      void write(const uint8_t input[], size_t length) override { send(input, length); }

      std::string name() const override;

      /**
      * Construct a chain of up to four filters. The filters are set
      * up in the same order as the arguments.
      */
      Chain(Filter* = nullptr, Filter* = nullptr,
            Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a chain from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Chain(Filter* filter_arr[], size_t length);
   };

/**
* This class represents a fork filter, whose purpose is to fork the
* flow of data. It causes an input message to result in n messages at
* the end of the filter, where n is the number of forks.
*/
class BOTAN_DLL Fork : public Fanout_Filter
   {
   public:
      void write(const uint8_t input[], size_t length) override { send(input, length); }
      void set_port(size_t n) { Fanout_Filter::set_port(n); }

      std::string name() const override;

      /**
      * Construct a Fork filter with up to four forks.
      */
      Fork(Filter*, Filter*, Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Fork from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Fork(Filter* filter_arr[], size_t length);
   };

#if defined(BOTAN_TARGET_OS_HAS_THREADS)

/**
* This class is a threaded version of the Fork filter. While this uses
* threads, the class itself is NOT thread-safe. This is meant as a drop-
* in replacement for Fork where performance gains are possible.
*/
class BOTAN_DLL Threaded_Fork : public Fork
   {
   public:
      std::string name() const override;

      /**
      * Construct a Threaded_Fork filter with up to four forks.
      */
      Threaded_Fork(Filter*, Filter*, Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Threaded_Fork from range of filters
      * @param filter_arr the list of filters
      * @param length how many filters
      */
      Threaded_Fork(Filter* filter_arr[], size_t length);

      ~Threaded_Fork();

   protected:
      void set_next(Filter* f[], size_t n);
      void send(const uint8_t in[], size_t length) override;

   private:
      void thread_delegate_work(const uint8_t input[], size_t length);
      void thread_entry(Filter* filter);

      std::vector<std::shared_ptr<std::thread>> m_threads;
      std::unique_ptr<struct Threaded_Fork_Data> m_thread_data;
   };
#endif

}


namespace Botan {

/**
* Create a password hash using Bcrypt
* @param password the password
* @param rng a random number generator
* @param work_factor how much work to do to slow down guessing attacks
*
* @see http://www.usenix.org/events/usenix99/provos/provos_html/
*/
std::string BOTAN_DLL generate_bcrypt(const std::string& password,
                                      RandomNumberGenerator& rng,
                                      uint16_t work_factor = 10);

/**
* Check a previously created password hash
* @param password the password to check against
* @param hash the stored hash to check against
*/
bool BOTAN_DLL check_bcrypt(const std::string& password,
                            const std::string& hash);

}


namespace Botan {

/**
* This class represents an abstract data source object.
*/
class BOTAN_DLL DataSource
   {
   public:
      /**
      * Read from the source. Moves the internal offset so that every
      * call to read will return a new portion of the source.
      *
      * @param out the byte array to write the result to
      * @param length the length of the byte array out
      * @return length in bytes that was actually read and put
      * into out
      */
      virtual size_t read(uint8_t out[], size_t length) BOTAN_WARN_UNUSED_RESULT = 0;

      virtual bool check_available(size_t n) = 0;

      /**
      * Read from the source but do not modify the internal
      * offset. Consecutive calls to peek() will return portions of
      * the source starting at the same position.
      *
      * @param out the byte array to write the output to
      * @param length the length of the byte array out
      * @param peek_offset the offset into the stream to read at
      * @return length in bytes that was actually read and put
      * into out
      */
      virtual size_t peek(uint8_t out[], size_t length, size_t peek_offset) const BOTAN_WARN_UNUSED_RESULT = 0;

      /**
      * Test whether the source still has data that can be read.
      * @return true if there is still data to read, false otherwise
      */
      virtual bool end_of_data() const = 0;
      /**
      * return the id of this data source
      * @return std::string representing the id of this data source
      */
      virtual std::string id() const { return ""; }

      /**
      * Read one byte.
      * @param out the byte to read to
      * @return length in bytes that was actually read and put
      * into out
      */
      size_t read_byte(uint8_t& out);

      /**
      * Peek at one byte.
      * @param out an output byte
      * @return length in bytes that was actually read and put
      * into out
      */
      size_t peek_byte(uint8_t& out) const;

      /**
      * Discard the next N bytes of the data
      * @param N the number of bytes to discard
      * @return number of bytes actually discarded
      */
      size_t discard_next(size_t N);

      /**
      * @return number of bytes read so far.
      */
      virtual size_t get_bytes_read() const = 0;

      DataSource() {}
      virtual ~DataSource() {}
      DataSource& operator=(const DataSource&) = delete;
      DataSource(const DataSource&) = delete;
   };

/**
* This class represents a Memory-Based DataSource
*/
class BOTAN_DLL DataSource_Memory : public DataSource
   {
   public:
      size_t read(uint8_t[], size_t) override;
      size_t peek(uint8_t[], size_t, size_t) const override;
      bool check_available(size_t n) override;
      bool end_of_data() const override;

      /**
      * Construct a memory source that reads from a string
      * @param in the string to read from
      */
      explicit DataSource_Memory(const std::string& in);

      /**
      * Construct a memory source that reads from a byte array
      * @param in the byte array to read from
      * @param length the length of the byte array
      */
      DataSource_Memory(const uint8_t in[], size_t length) :
         m_source(in, in + length), m_offset(0) {}

      /**
      * Construct a memory source that reads from a secure_vector
      * @param in the MemoryRegion to read from
      */
      explicit DataSource_Memory(const secure_vector<uint8_t>& in) :
         m_source(in), m_offset(0) {}

      /**
      * Construct a memory source that reads from a std::vector
      * @param in the MemoryRegion to read from
      */
      explicit DataSource_Memory(const std::vector<uint8_t>& in) :
         m_source(in.begin(), in.end()), m_offset(0) {}

      size_t get_bytes_read() const override { return m_offset; }
   private:
      secure_vector<uint8_t> m_source;
      size_t m_offset;
   };

/**
* This class represents a Stream-Based DataSource.
*/
class BOTAN_DLL DataSource_Stream : public DataSource
   {
   public:
      size_t read(uint8_t[], size_t) override;
      size_t peek(uint8_t[], size_t, size_t) const override;
      bool check_available(size_t n) override;
      bool end_of_data() const override;
      std::string id() const override;

      DataSource_Stream(std::istream&,
                        const std::string& id = "<std::istream>");

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)
      /**
      * Construct a Stream-Based DataSource from filesystem path
      * @param file the path to the file
      * @param use_binary whether to treat the file as binary or not
      */
      DataSource_Stream(const std::string& file, bool use_binary = false);
#endif

      DataSource_Stream(const DataSource_Stream&) = delete;

      DataSource_Stream& operator=(const DataSource_Stream&) = delete;

      ~DataSource_Stream();

      size_t get_bytes_read() const override { return m_total_read; }
   private:
      const std::string m_identifier;

      std::unique_ptr<std::istream> m_source_memory;
      std::istream& m_source;
      size_t m_total_read;
   };

}


namespace Botan {

/**
* BER Decoding Object
*/
class BOTAN_DLL BER_Decoder
   {
   public:
      BER_Object get_next_object();

      std::vector<uint8_t> get_next_octet_string();

      void push_back(const BER_Object& obj);

      bool more_items() const;
      BER_Decoder& verify_end();
      BER_Decoder& discard_remaining();

      BER_Decoder  start_cons(ASN1_Tag type_tag, ASN1_Tag class_tag = UNIVERSAL);
      BER_Decoder& end_cons();

      BER_Decoder& get_next(BER_Object& ber);

      BER_Decoder& raw_bytes(secure_vector<uint8_t>& v);
      BER_Decoder& raw_bytes(std::vector<uint8_t>& v);

      BER_Decoder& decode_null();
      BER_Decoder& decode(bool& v);
      BER_Decoder& decode(size_t& v);
      BER_Decoder& decode(class BigInt& v);
      BER_Decoder& decode(std::vector<uint8_t>& v, ASN1_Tag type_tag);
      BER_Decoder& decode(secure_vector<uint8_t>& v, ASN1_Tag type_tag);

      BER_Decoder& decode(bool& v,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(size_t& v,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(class BigInt& v,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(std::vector<uint8_t>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(secure_vector<uint8_t>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      BER_Decoder& decode(class ASN1_Object& obj,
                          ASN1_Tag type_tag = NO_OBJECT,
                          ASN1_Tag class_tag = NO_OBJECT);

      BER_Decoder& decode_octet_string_bigint(class BigInt& b);

      uint64_t decode_constrained_integer(ASN1_Tag type_tag,
                                        ASN1_Tag class_tag,
                                        size_t T_bytes);

      template<typename T> BER_Decoder& decode_integer_type(T& out)
         {
         return decode_integer_type<T>(out, INTEGER, UNIVERSAL);
         }

      template<typename T>
         BER_Decoder& decode_integer_type(T& out,
                                          ASN1_Tag type_tag,
                                          ASN1_Tag class_tag = CONTEXT_SPECIFIC)
         {
         out = static_cast<T>(decode_constrained_integer(type_tag, class_tag, sizeof(out)));
         return (*this);
         }

      template<typename T>
         BER_Decoder& decode_optional(T& out,
                                      ASN1_Tag type_tag,
                                      ASN1_Tag class_tag,
                                      const T& default_value = T());

      template<typename T>
         BER_Decoder& decode_optional_implicit(
            T& out,
            ASN1_Tag type_tag,
            ASN1_Tag class_tag,
            ASN1_Tag real_type,
            ASN1_Tag real_class,
            const T& default_value = T());

      template<typename T>
         BER_Decoder& decode_list(std::vector<T>& out,
                                  ASN1_Tag type_tag = SEQUENCE,
                                  ASN1_Tag class_tag = UNIVERSAL);

      template<typename T>
         BER_Decoder& decode_and_check(const T& expected,
                                       const std::string& error_msg)
         {
         T actual;
         decode(actual);

         if(actual != expected)
            throw Decoding_Error(error_msg);

         return (*this);
         }

      /*
      * Decode an OPTIONAL string type
      */
      template<typename Alloc>
      BER_Decoder& decode_optional_string(std::vector<uint8_t, Alloc>& out,
                                          ASN1_Tag real_type,
                                          uint16_t type_no,
                                          ASN1_Tag class_tag = CONTEXT_SPECIFIC)
         {
         BER_Object obj = get_next_object();

         ASN1_Tag type_tag = static_cast<ASN1_Tag>(type_no);

         if(obj.type_tag == type_tag && obj.class_tag == class_tag)
            {
            if((class_tag & CONSTRUCTED) && (class_tag & CONTEXT_SPECIFIC))
               BER_Decoder(obj.value).decode(out, real_type).verify_end();
            else
               {
               push_back(obj);
               decode(out, real_type, type_tag, class_tag);
               }
            }
         else
            {
            out.clear();
            push_back(obj);
            }

         return (*this);
         }

      BER_Decoder& operator=(const BER_Decoder&) = delete;

      explicit BER_Decoder(DataSource&);

      BER_Decoder(const uint8_t[], size_t);

      explicit BER_Decoder(const secure_vector<uint8_t>&);

      explicit BER_Decoder(const std::vector<uint8_t>& vec);

      BER_Decoder(const BER_Decoder&);
      ~BER_Decoder();
   private:
      BER_Decoder* m_parent;
      DataSource* m_source;
      BER_Object m_pushed;
      mutable bool m_owns;
   };

/*
* Decode an OPTIONAL or DEFAULT element
*/
template<typename T>
BER_Decoder& BER_Decoder::decode_optional(T& out,
                                          ASN1_Tag type_tag,
                                          ASN1_Tag class_tag,
                                          const T& default_value)
   {
   BER_Object obj = get_next_object();

   if(obj.type_tag == type_tag && obj.class_tag == class_tag)
      {
      if((class_tag & CONSTRUCTED) && (class_tag & CONTEXT_SPECIFIC))
         BER_Decoder(obj.value).decode(out).verify_end();
      else
         {
         push_back(obj);
         decode(out, type_tag, class_tag);
         }
      }
   else
      {
      out = default_value;
      push_back(obj);
      }

   return (*this);
   }

/*
* Decode an OPTIONAL or DEFAULT element
*/
template<typename T>
BER_Decoder& BER_Decoder::decode_optional_implicit(
   T& out,
   ASN1_Tag type_tag,
   ASN1_Tag class_tag,
   ASN1_Tag real_type,
   ASN1_Tag real_class,
   const T& default_value)
   {
   BER_Object obj = get_next_object();

   if(obj.type_tag == type_tag && obj.class_tag == class_tag)
      {
      obj.type_tag = real_type;
      obj.class_tag = real_class;
      push_back(obj);
      decode(out, real_type, real_class);
      }
   else
      {
      out = default_value;
      push_back(obj);
      }

   return (*this);
   }
/*
* Decode a list of homogenously typed values
*/
template<typename T>
BER_Decoder& BER_Decoder::decode_list(std::vector<T>& vec,
                                      ASN1_Tag type_tag,
                                      ASN1_Tag class_tag)
   {
   BER_Decoder list = start_cons(type_tag, class_tag);

   while(list.more_items())
      {
      T value;
      list.decode(value);
      vec.push_back(value);
      }

   list.end_cons();

   return (*this);
   }

}


namespace Botan {

#if (BOTAN_MP_WORD_BITS == 8)
  typedef uint8_t word;
#elif (BOTAN_MP_WORD_BITS == 16)
  typedef uint16_t word;
#elif (BOTAN_MP_WORD_BITS == 32)
  typedef uint32_t word;
#elif (BOTAN_MP_WORD_BITS == 64)
  typedef uint64_t word;
#else
  #error BOTAN_MP_WORD_BITS must be 8, 16, 32, or 64
#endif

const word MP_WORD_MASK = ~static_cast<word>(0);
const word MP_WORD_TOP_BIT = static_cast<word>(1) << (8*sizeof(word) - 1);
const word MP_WORD_MAX = MP_WORD_MASK;

}


namespace Botan {

/**
* Arbitrary precision integer
*/
class BOTAN_DLL BigInt
   {
   public:
     /**
     * Base enumerator for encoding and decoding
     */
     enum Base { Decimal = 10, Hexadecimal = 16, Binary = 256 };

     /**
     * Sign symbol definitions for positive and negative numbers
     */
     enum Sign { Negative = 0, Positive = 1 };

     /**
     * DivideByZero Exception
     */
     struct BOTAN_DLL DivideByZero : public Exception
        { DivideByZero() : Exception("BigInt divide by zero") {} };

     /**
     * Create empty BigInt
     */
     BigInt() {}

     /**
     * Create BigInt from 64 bit integer
     * @param n initial value of this BigInt
     */
     BigInt(uint64_t n);

     /**
     * Copy Constructor
     * @param other the BigInt to copy
     */
     BigInt(const BigInt& other);

     /**
     * Create BigInt from a string. If the string starts with 0x the
     * rest of the string will be interpreted as hexadecimal digits.
     * Otherwise, it will be interpreted as a decimal number.
     *
     * @param str the string to parse for an integer value
     */
     BigInt(const std::string& str);

     /**
     * Create a BigInt from an integer in a byte array
     * @param buf the byte array holding the value
     * @param length size of buf
     * @param base is the number base of the integer in buf
     */
     BigInt(const uint8_t buf[], size_t length, Base base = Binary);

     /**
     * \brief Create a random BigInt of the specified size
     *
     * @param rng random number generator
     * @param bits size in bits
     * @param set_high_bit if true, the highest bit is always set
     *
     * @see randomize
     */
     BigInt(RandomNumberGenerator& rng, size_t bits, bool set_high_bit = true);

     /**
     * Create BigInt of specified size, all zeros
     * @param sign the sign
     * @param n size of the internal register in words
     */
     BigInt(Sign sign, size_t n);

     /**
     * Move constructor
     */
     BigInt(BigInt&& other)
        {
        this->swap(other);
        }

     /**
     * Move assignment
     */
     BigInt& operator=(BigInt&& other)
        {
        if(this != &other)
           this->swap(other);

        return (*this);
        }

     /**
     * Copy assignment
     */
     BigInt& operator=(const BigInt&) = default;

     /**
     * Swap this value with another
     * @param other BigInt to swap values with
     */
     void swap(BigInt& other)
        {
        m_reg.swap(other.m_reg);
        std::swap(m_signedness, other.m_signedness);
        }

     void swap_reg(secure_vector<word>& reg)
        {
        m_reg.swap(reg);
        }

     /**
     * += operator
     * @param y the BigInt to add to this
     */
     BigInt& operator+=(const BigInt& y);

     /**
     * -= operator
     * @param y the BigInt to subtract from this
     */
     BigInt& operator-=(const BigInt& y);

     /**
     * *= operator
     * @param y the BigInt to multiply with this
     */
     BigInt& operator*=(const BigInt& y);

     /**
     * /= operator
     * @param y the BigInt to divide this by
     */
     BigInt& operator/=(const BigInt& y);

     /**
     * Modulo operator
     * @param y the modulus to reduce this by
     */
     BigInt& operator%=(const BigInt& y);

     /**
     * Modulo operator
     * @param y the modulus (word) to reduce this by
     */
     word    operator%=(word y);

     /**
     * Left shift operator
     * @param shift the number of bits to shift this left by
     */
     BigInt& operator<<=(size_t shift);

     /**
     * Right shift operator
     * @param shift the number of bits to shift this right by
     */
     BigInt& operator>>=(size_t shift);

     /**
     * Increment operator
     */
     BigInt& operator++() { return (*this += 1); }

     /**
     * Decrement operator
     */
     BigInt& operator--() { return (*this -= 1); }

     /**
     * Postfix increment operator
     */
     BigInt  operator++(int) { BigInt x = (*this); ++(*this); return x; }

     /**
     * Postfix decrement operator
     */
     BigInt  operator--(int) { BigInt x = (*this); --(*this); return x; }

     /**
     * Unary negation operator
     * @return negative this
     */
     BigInt operator-() const;

     /**
     * ! operator
     * @return true iff this is zero, otherwise false
     */
     bool operator !() const { return (!is_nonzero()); }

     /**
     * Zeroize the BigInt. The size of the underlying register is not
     * modified.
     */
     void clear() { zeroise(m_reg); }

     /**
     * Compare this to another BigInt
     * @param n the BigInt value to compare with
     * @param check_signs include sign in comparison?
     * @result if (this<n) return -1, if (this>n) return 1, if both
     * values are identical return 0 [like Perl's <=> operator]
     */
     int32_t cmp(const BigInt& n, bool check_signs = true) const;

     /**
     * Test if the integer has an even value
     * @result true if the integer is even, false otherwise
     */
     bool is_even() const { return (get_bit(0) == 0); }

     /**
     * Test if the integer has an odd value
     * @result true if the integer is odd, false otherwise
     */
     bool is_odd()  const { return (get_bit(0) == 1); }

     /**
     * Test if the integer is not zero
     * @result true if the integer is non-zero, false otherwise
     */
     bool is_nonzero() const { return (!is_zero()); }

     /**
     * Test if the integer is zero
     * @result true if the integer is zero, false otherwise
     */
     bool is_zero() const
        {
        const size_t sw = sig_words();

        for(size_t i = 0; i != sw; ++i)
           if(m_reg[i])
              return false;
        return true;
        }

     /**
     * Set bit at specified position
     * @param n bit position to set
     */
     void set_bit(size_t n);

     /**
     * Clear bit at specified position
     * @param n bit position to clear
     */
     void clear_bit(size_t n);

     /**
     * Clear all but the lowest n bits
     * @param n amount of bits to keep
     */
     void mask_bits(size_t n)
        {
        if(n == 0) { clear(); return; }

        const size_t top_word = n / BOTAN_MP_WORD_BITS;
        const word mask = (static_cast<word>(1) << (n % BOTAN_MP_WORD_BITS)) - 1;

        if(top_word < size())
           {
           const size_t len = size() - (top_word + 1);
           if (len > 0)
              {
              clear_mem(&m_reg[top_word+1], len);
              }
           m_reg[top_word] &= mask;
           }
        }

     /**
     * Return bit value at specified position
     * @param n the bit offset to test
     * @result true, if the bit at position n is set, false otherwise
     */
     bool get_bit(size_t n) const
        {
        return ((word_at(n / BOTAN_MP_WORD_BITS) >> (n % BOTAN_MP_WORD_BITS)) & 1);
        }

     /**
     * Return (a maximum of) 32 bits of the complete value
     * @param offset the offset to start extracting
     * @param length amount of bits to extract (starting at offset)
     * @result the integer extracted from the register starting at
     * offset with specified length
     */
     uint32_t get_substring(size_t offset, size_t length) const;

     /**
     * Convert this value into a uint32_t, if it is in the range
     * [0 ... 2**32-1], or otherwise throw an exception.
     * @result the value as a uint32_t if conversion is possible
     */
     uint32_t to_u32bit() const;

     /**
     * @param n the offset to get a byte from
     * @result byte at offset n
     */
     uint8_t byte_at(size_t n) const
        {
        return get_byte(sizeof(word) - (n % sizeof(word)) - 1,
                        word_at(n / sizeof(word)));
        }

     /**
     * Return the word at a specified position of the internal register
     * @param n position in the register
     * @return value at position n
     */
     word word_at(size_t n) const
        { return ((n < size()) ? m_reg[n] : 0); }

     void set_word_at(size_t i, word w)
        {
        grow_to(i + 1);
        m_reg[i] = w;
        }

     /**
     * Tests if the sign of the integer is negative
     * @result true, iff the integer has a negative sign
     */
     bool is_negative() const { return (sign() == Negative); }

     /**
     * Tests if the sign of the integer is positive
     * @result true, iff the integer has a positive sign
     */
     bool is_positive() const { return (sign() == Positive); }

     /**
     * Return the sign of the integer
     * @result the sign of the integer
     */
     Sign sign() const { return (m_signedness); }

     /**
     * @result the opposite sign of the represented integer value
     */
     Sign reverse_sign() const;

     /**
     * Flip the sign of this BigInt
     */
     void flip_sign();

     /**
     * Set sign of the integer
     * @param sign new Sign to set
     */
     void set_sign(Sign sign);

     /**
     * @result absolute (positive) value of this
     */
     BigInt abs() const;

     /**
     * Give size of internal register
     * @result size of internal register in words
     */
     size_t size() const { return m_reg.size(); }

     /**
     * Return how many words we need to hold this value
     * @result significant words of the represented integer value
     */
     size_t sig_words() const
        {
        const word* x = m_reg.data();
        size_t sig = m_reg.size();

        while(sig && (x[sig-1] == 0))
           sig--;
        return sig;
        }

     /**
     * Give byte length of the integer
     * @result byte length of the represented integer value
     */
     size_t bytes() const;

     /**
     * Get the bit length of the integer
     * @result bit length of the represented integer value
     */
     size_t bits() const;

     /**
     * Return a mutable pointer to the register
     * @result a pointer to the start of the internal register
     */
     word* mutable_data() { return m_reg.data(); }

     /**
     * Return a const pointer to the register
     * @result a pointer to the start of the internal register
     */
     const word* data() const { return m_reg.data(); }

     secure_vector<word>& get_word_vector() { return m_reg; }
     const secure_vector<word>& get_word_vector() const { return m_reg; }

     /**
     * Increase internal register buffer to at least n words
     * @param n new size of register
     */
     void grow_to(size_t n);

     /**
     * Fill BigInt with a random number with size of bitsize
     *
     * If \p set_high_bit is true, the highest bit will be set, which causes
     * the entropy to be \a bits-1. Otherwise the highest bit is randomly chosen
     * by the rng, causing the entropy to be \a bits.
     *
     * @param rng the random number generator to use
     * @param bitsize number of bits the created random value should have
     * @param set_high_bit if true, the highest bit is always set
     */
     void randomize(RandomNumberGenerator& rng, size_t bitsize, bool set_high_bit = true);

     /**
     * Store BigInt-value in a given byte array
     * @param buf destination byte array for the integer value
     */
     void binary_encode(uint8_t buf[]) const;

     /**
     * Read integer value from a byte array with given size
     * @param buf byte array buffer containing the integer
     * @param length size of buf
     */
     void binary_decode(const uint8_t buf[], size_t length);

     /**
     * Read integer value from a byte array (secure_vector<uint8_t>)
     * @param buf the array to load from
     */
     void binary_decode(const secure_vector<uint8_t>& buf)
        {
        binary_decode(buf.data(), buf.size());
        }

     /**
     * @param base the base to measure the size for
     * @return size of this integer in base base
     */
     size_t encoded_size(Base base = Binary) const;

     /**
     * @param rng a random number generator
     * @param min the minimum value
     * @param max the maximum value
     * @return random integer in [min,max)
     */
     static BigInt random_integer(RandomNumberGenerator& rng,
                                  const BigInt& min,
                                  const BigInt& max);

     /**
     * Create a power of two
     * @param n the power of two to create
     * @return bigint representing 2^n
     */
     static BigInt power_of_2(size_t n)
        {
        BigInt b;
        b.set_bit(n);
        return b;
        }

     /**
     * Encode the integer value from a BigInt to a std::vector of bytes
     * @param n the BigInt to use as integer source
     * @param base number-base of resulting byte array representation
     * @result secure_vector of bytes containing the integer with given base
     */
     static std::vector<uint8_t> encode(const BigInt& n, Base base = Binary);

     /**
     * Encode the integer value from a BigInt to a secure_vector of bytes
     * @param n the BigInt to use as integer source
     * @param base number-base of resulting byte array representation
     * @result secure_vector of bytes containing the integer with given base
     */
     static secure_vector<uint8_t> encode_locked(const BigInt& n,
                                              Base base = Binary);

     /**
     * Encode the integer value from a BigInt to a byte array
     * @param buf destination byte array for the encoded integer
     * value with given base
     * @param n the BigInt to use as integer source
     * @param base number-base of resulting byte array representation
     */
     static void encode(uint8_t buf[], const BigInt& n, Base base = Binary);

     /**
     * Create a BigInt from an integer in a byte array
     * @param buf the binary value to load
     * @param length size of buf
     * @param base number-base of the integer in buf
     * @result BigInt representing the integer in the byte array
     */
     static BigInt decode(const uint8_t buf[], size_t length,
                          Base base = Binary);

     /**
     * Create a BigInt from an integer in a byte array
     * @param buf the binary value to load
     * @param base number-base of the integer in buf
     * @result BigInt representing the integer in the byte array
     */
     static BigInt decode(const secure_vector<uint8_t>& buf,
                          Base base = Binary)
        {
        return BigInt::decode(buf.data(), buf.size(), base);
        }

     /**
     * Create a BigInt from an integer in a byte array
     * @param buf the binary value to load
     * @param base number-base of the integer in buf
     * @result BigInt representing the integer in the byte array
     */
     static BigInt decode(const std::vector<uint8_t>& buf,
                          Base base = Binary)
        {
        return BigInt::decode(buf.data(), buf.size(), base);
        }

     /**
     * Encode a BigInt to a byte array according to IEEE 1363
     * @param n the BigInt to encode
     * @param bytes the length of the resulting secure_vector<uint8_t>
     * @result a secure_vector<uint8_t> containing the encoded BigInt
     */
     static secure_vector<uint8_t> encode_1363(const BigInt& n, size_t bytes);

     static void encode_1363(uint8_t out[], size_t bytes, const BigInt& n);

     /**
     * Encode two BigInt to a byte array according to IEEE 1363
     * @param n1 the first BigInt to encode
     * @param n2 the second BigInt to encode
     * @param bytes the length of the encoding of each single BigInt
     * @result a secure_vector<uint8_t> containing the concatenation of the two encoded BigInt
     */
     static secure_vector<uint8_t> encode_fixed_length_int_pair(const BigInt& n1, const BigInt& n2, size_t bytes);

   private:
      secure_vector<word> m_reg;
      Sign m_signedness = Positive;
   };

/*
* Arithmetic Operators
*/
BigInt BOTAN_DLL operator+(const BigInt& x, const BigInt& y);
BigInt BOTAN_DLL operator-(const BigInt& x, const BigInt& y);
BigInt BOTAN_DLL operator*(const BigInt& x, const BigInt& y);
BigInt BOTAN_DLL operator/(const BigInt& x, const BigInt& d);
BigInt BOTAN_DLL operator%(const BigInt& x, const BigInt& m);
word   BOTAN_DLL operator%(const BigInt& x, word m);
BigInt BOTAN_DLL operator<<(const BigInt& x, size_t n);
BigInt BOTAN_DLL operator>>(const BigInt& x, size_t n);

/*
* Comparison Operators
*/
inline bool operator==(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) == 0); }
inline bool operator!=(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) != 0); }
inline bool operator<=(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) <= 0); }
inline bool operator>=(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) >= 0); }
inline bool operator<(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) < 0); }
inline bool operator>(const BigInt& a, const BigInt& b)
   { return (a.cmp(b) > 0); }

/*
* I/O Operators
*/
BOTAN_DLL std::ostream& operator<<(std::ostream&, const BigInt&);
BOTAN_DLL std::istream& operator>>(std::istream&, BigInt&);

}

namespace std {

template<>
inline void swap<Botan::BigInt>(Botan::BigInt& x, Botan::BigInt& y)
   {
   x.swap(y);
   }

}


namespace Botan {

enum blake2b_constant {
  BLAKE2B_BLOCKBYTES = 128,
  BLAKE2B_OUTBYTES   = 64,
  BLAKE2B_IVU64COUNT = 8
};

/**
* BLAKE2B
*/
class BOTAN_DLL Blake2b final : public HashFunction
   {
   public:
      /**
      * @param output_bits the output size of Blake2b in bits
      */
      explicit Blake2b(size_t output_bits = 512);

      size_t hash_block_size() const override { return BLAKE2B_BLOCKBYTES; }
      size_t output_length() const override { return m_output_bits / 8; }

      HashFunction* clone() const override;
      std::string name() const override;
      void clear() override;

   private:
      void add_data(const uint8_t input[], size_t length) override;
      void final_result(uint8_t out[]) override;

      inline void state_init();
      inline void increment_counter(const uint64_t inc);
      void compress(bool lastblock = false);

      size_t m_output_bits;

      secure_vector<uint8_t> m_buffer;
      size_t m_buflen;

      secure_vector<uint64_t> m_H;
      uint64_t m_T[2];
      uint64_t m_F[2];
   };

}


namespace Botan {

/**
* Modular Exponentiator Interface
*/
class BOTAN_DLL Modular_Exponentiator
   {
   public:
      virtual void set_base(const BigInt&) = 0;
      virtual void set_exponent(const BigInt&) = 0;
      virtual BigInt execute() const = 0;
      virtual Modular_Exponentiator* copy() const = 0;
      virtual ~Modular_Exponentiator() {}
   };

/**
* Modular Exponentiator Proxy
*/
class BOTAN_DLL Power_Mod
   {
   public:

      enum Usage_Hints {
         NO_HINTS        = 0x0000,

         BASE_IS_FIXED   = 0x0001,
         BASE_IS_SMALL   = 0x0002,
         BASE_IS_LARGE   = 0x0004,
         BASE_IS_2       = 0x0008,

         EXP_IS_FIXED    = 0x0100,
         EXP_IS_SMALL    = 0x0200,
         EXP_IS_LARGE    = 0x0400
      };

      /*
      * Try to choose a good window size
      */
      static size_t window_bits(size_t exp_bits, size_t base_bits,
                                Power_Mod::Usage_Hints hints);

      /**
      * @param modulus the modulus
      * @param hints Passed to set_modulus if modulus > 0
      * @param disable_montgomery_arith Disables use of Montgomery
      * representation. Likely only useful for testing.
      */
      void set_modulus(const BigInt& modulus,
                       Usage_Hints = NO_HINTS,
                       bool disable_montgomery_arith = false) const;

      /**
      * Set the base
      */
      void set_base(const BigInt& base) const;

      /**
      * Set the exponent
      */
      void set_exponent(const BigInt& exponent) const;

      /**
      * All three of the above functions must have already been called.
      * @return result of g^x%p
      */
      BigInt execute() const;

      Power_Mod& operator=(const Power_Mod&);

      /**
      * @param modulus Optionally call set_modulus
      * @param hints Passed to set_modulus if modulus > 0
      * @param disable_montgomery_arith Disables use of Montgomery
      * representation. Likely only useful for testing.
      */
      Power_Mod(const BigInt& modulus = 0,
                Usage_Hints hints = NO_HINTS,
                bool disable_montgomery_arith = false);
      Power_Mod(const Power_Mod&);
      virtual ~Power_Mod();
   private:
      mutable Modular_Exponentiator* m_core;
   };

/**
* Fixed Exponent Modular Exponentiator Proxy
*/
class BOTAN_DLL Fixed_Exponent_Power_Mod : public Power_Mod
   {
   public:
      BigInt operator()(const BigInt& b) const
         { set_base(b); return execute(); }

      Fixed_Exponent_Power_Mod() {}

      Fixed_Exponent_Power_Mod(const BigInt& exponent,
                               const BigInt& modulus,
                               Usage_Hints hints = NO_HINTS);
   };

/**
* Fixed Base Modular Exponentiator Proxy
*/
class BOTAN_DLL Fixed_Base_Power_Mod : public Power_Mod
   {
   public:
      BigInt operator()(const BigInt& e) const
         { set_exponent(e); return execute(); }

      Fixed_Base_Power_Mod() {}

      Fixed_Base_Power_Mod(const BigInt& base,
                           const BigInt& modulus,
                           Usage_Hints hints = NO_HINTS);
   };

}


namespace Botan {

/**
* Fused multiply-add
* @param a an integer
* @param b an integer
* @param c an integer
* @return (a*b)+c
*/
BigInt BOTAN_DLL mul_add(const BigInt& a,
                         const BigInt& b,
                         const BigInt& c);

/**
* Fused subtract-multiply
* @param a an integer
* @param b an integer
* @param c an integer
* @return (a-b)*c
*/
BigInt BOTAN_DLL sub_mul(const BigInt& a,
                         const BigInt& b,
                         const BigInt& c);

/**
* Fused multiply-subtract
* @param a an integer
* @param b an integer
* @param c an integer
* @return (a*b)-c
*/
BigInt BOTAN_DLL mul_sub(const BigInt& a,
                         const BigInt& b,
                         const BigInt& c);

/**
* Return the absolute value
* @param n an integer
* @return absolute value of n
*/
inline BigInt abs(const BigInt& n) { return n.abs(); }

/**
* Compute the greatest common divisor
* @param x a positive integer
* @param y a positive integer
* @return gcd(x,y)
*/
BigInt BOTAN_DLL gcd(const BigInt& x, const BigInt& y);

/**
* Least common multiple
* @param x a positive integer
* @param y a positive integer
* @return z, smallest integer such that z % x == 0 and z % y == 0
*/
BigInt BOTAN_DLL lcm(const BigInt& x, const BigInt& y);

/**
* @param x an integer
* @return (x*x)
*/
BigInt BOTAN_DLL square(const BigInt& x);

/**
* Modular inversion
* @param x a positive integer
* @param modulus a positive integer
* @return y st (x*y) % modulus == 1 or 0 if no such value
* Not const time
*/
BigInt BOTAN_DLL inverse_mod(const BigInt& x,
                             const BigInt& modulus);

/**
* Const time modular inversion
* Requires the modulus be odd
*/
BigInt BOTAN_DLL ct_inverse_mod_odd_modulus(const BigInt& n, const BigInt& mod);

/**
* Return a^-1 * 2^k mod b
* Returns k, between n and 2n
* Not const time
*/
size_t BOTAN_DLL almost_montgomery_inverse(BigInt& result,
                                           const BigInt& a,
                                           const BigInt& b);

/**
* Call almost_montgomery_inverse and correct the result to a^-1 mod b
*/
BigInt BOTAN_DLL normalized_montgomery_inverse(const BigInt& a, const BigInt& b);


/**
* Compute the Jacobi symbol. If n is prime, this is equivalent
* to the Legendre symbol.
* @see http://mathworld.wolfram.com/JacobiSymbol.html
*
* @param a is a non-negative integer
* @param n is an odd integer > 1
* @return (n / m)
*/
int32_t BOTAN_DLL jacobi(const BigInt& a,
                        const BigInt& n);

/**
* Modular exponentation
* @param b an integer base
* @param x a positive exponent
* @param m a positive modulus
* @return (b^x) % m
*/
BigInt BOTAN_DLL power_mod(const BigInt& b,
                           const BigInt& x,
                           const BigInt& m);

/**
* Compute the square root of x modulo a prime using the
* Shanks-Tonnelli algorithm
*
* @param x the input
* @param p the prime
* @return y such that (y*y)%p == x, or -1 if no such integer
*/
BigInt BOTAN_DLL ressol(const BigInt& x, const BigInt& p);

/*
* Compute -input^-1 mod 2^MP_WORD_BITS. Returns zero if input
* is even. If input is odd, input and 2^n are relatively prime
* and an inverse exists.
*/
word BOTAN_DLL monty_inverse(word input);

/**
* @param x a positive integer
* @return count of the zero bits in x, or, equivalently, the largest
*         value of n such that 2^n divides x evenly. Returns zero if
*         n is less than or equal to zero.
*/
size_t BOTAN_DLL low_zero_bits(const BigInt& x);

/**
* Check for primality
* @param n a positive integer to test for primality
* @param rng a random number generator
* @param prob chance of false positive is bounded by 1/2**prob
* @param is_random true if n was randomly chosen by us
* @return true if all primality tests passed, otherwise false
*/
bool BOTAN_DLL is_prime(const BigInt& n,
                        RandomNumberGenerator& rng,
                        size_t prob = 56,
                        bool is_random = false);

inline bool quick_check_prime(const BigInt& n, RandomNumberGenerator& rng)
   { return is_prime(n, rng, 32); }

inline bool check_prime(const BigInt& n, RandomNumberGenerator& rng)
   { return is_prime(n, rng, 56); }

inline bool verify_prime(const BigInt& n, RandomNumberGenerator& rng)
   { return is_prime(n, rng, 80); }


/**
* Randomly generate a prime
* @param rng a random number generator
* @param bits how large the resulting prime should be in bits
* @param coprime a positive integer the result should be coprime to
* @param equiv a non-negative number that the result should be
               equivalent to modulo equiv_mod
* @param equiv_mod the modulus equiv should be checked against
* @return random prime with the specified criteria
*/
BigInt BOTAN_DLL random_prime(RandomNumberGenerator& rng,
                              size_t bits, const BigInt& coprime = 1,
                              size_t equiv = 1, size_t equiv_mod = 2);

/**
* Return a 'safe' prime, of the form p=2*q+1 with q prime
* @param rng a random number generator
* @param bits is how long the resulting prime should be
* @return prime randomly chosen from safe primes of length bits
*/
BigInt BOTAN_DLL random_safe_prime(RandomNumberGenerator& rng,
                                   size_t bits);

/**
* Generate DSA parameters using the FIPS 186 kosherizer
* @param rng a random number generator
* @param p_out where the prime p will be stored
* @param q_out where the prime q will be stored
* @param pbits how long p will be in bits
* @param qbits how long q will be in bits
* @return random seed used to generate this parameter set
*/
std::vector<uint8_t> BOTAN_DLL
generate_dsa_primes(RandomNumberGenerator& rng,
                    BigInt& p_out, BigInt& q_out,
                    size_t pbits, size_t qbits);

/**
* Generate DSA parameters using the FIPS 186 kosherizer
* @param rng a random number generator
* @param p_out where the prime p will be stored
* @param q_out where the prime q will be stored
* @param pbits how long p will be in bits
* @param qbits how long q will be in bits
* @param seed the seed used to generate the parameters
* @param offset optional offset from seed to start searching at
* @return true if seed generated a valid DSA parameter set, otherwise
          false. p_out and q_out are only valid if true was returned.
*/
bool BOTAN_DLL
generate_dsa_primes(RandomNumberGenerator& rng,
                    BigInt& p_out, BigInt& q_out,
                    size_t pbits, size_t qbits,
                    const std::vector<uint8_t>& seed,
                    size_t offset = 0);

/**
* The size of the PRIMES[] array
*/
const size_t PRIME_TABLE_SIZE = 6541;

/**
* A const array of all primes less than 65535
*/
extern const uint16_t BOTAN_DLL PRIMES[];

}


namespace Botan {

/**
* Modular Reducer (using Barrett's technique)
*/
class BOTAN_DLL Modular_Reducer
   {
   public:
      const BigInt& get_modulus() const { return m_modulus; }

      BigInt reduce(const BigInt& x) const;

      /**
      * Multiply mod p
      * @param x
      * @param y
      * @return (x * y) % p
      */
      BigInt multiply(const BigInt& x, const BigInt& y) const
         { return reduce(x * y); }

      /**
      * Square mod p
      * @param x
      * @return (x * x) % p
      */
      BigInt square(const BigInt& x) const
         { return reduce(Botan::square(x)); }

      /**
      * Cube mod p
      * @param x
      * @return (x * x * x) % p
      */
      BigInt cube(const BigInt& x) const
         { return multiply(x, this->square(x)); }

      bool initialized() const { return (m_mod_words != 0); }

      Modular_Reducer() { m_mod_words = 0; }
      explicit Modular_Reducer(const BigInt& mod);
   private:
      BigInt m_modulus, m_modulus_2, m_mu;
      size_t m_mod_words;
   };

}


namespace Botan {

class RandomNumberGenerator;

/**
* Blinding Function Object.
*/
class BOTAN_DLL Blinder
   {
   public:
      /**
      * Blind a value.
      * The blinding nonce k is freshly generated after
      * BOTAN_BLINDING_REINIT_INTERVAL calls to blind().
      * BOTAN_BLINDING_REINIT_INTERVAL = 0 means a fresh
      * nonce is only generated once. On every other call,
      * an updated nonce is used for blinding: k' = k*k mod n.
      * @param x value to blind
      * @return blinded value
      */
      BigInt blind(const BigInt& x) const;

      /**
      * Unblind a value.
      * @param x value to unblind
      * @return unblinded value
      */
      BigInt unblind(const BigInt& x) const;

      /**
      * @param modulus the modulus
      * @param rng the RNG to use for generating the nonce
      * @param fwd_func a function that calculates the modular
      * exponentiation of the public exponent and the given value (the nonce)
      * @param inv_func a function that calculates the modular inverse
      * of the given value (the nonce)
      */
      Blinder(const BigInt& modulus,
              RandomNumberGenerator& rng,
              std::function<BigInt (const BigInt&)> fwd_func,
              std::function<BigInt (const BigInt&)> inv_func);

      Blinder(const Blinder&) = delete;

      Blinder& operator=(const Blinder&) = delete;

   private:
      BigInt blinding_nonce() const;

      Modular_Reducer m_reducer;
      RandomNumberGenerator& m_rng;
      std::function<BigInt (const BigInt&)> m_fwd_fn;
      std::function<BigInt (const BigInt&)> m_inv_fn;
      size_t m_modulus_bits = 0;

      mutable BigInt m_e, m_d;
      mutable size_t m_counter = 0;
   };

}


namespace Botan {

/**
* Blowfish
*/
class BOTAN_DLL Blowfish final : public Block_Cipher_Fixed_Params<8, 1, 56>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      /**
      * Modified EKSBlowfish key schedule, used for bcrypt password hashing
      */
      void eks_key_schedule(const uint8_t key[], size_t key_length,
                            const uint8_t salt[16], size_t workfactor);

      void clear() override;
      std::string name() const override { return "Blowfish"; }
      BlockCipher* clone() const override { return new Blowfish; }
   private:
      void key_schedule(const uint8_t key[], size_t length) override;

      void key_expansion(const uint8_t key[],
                         size_t key_length,
                         const uint8_t salt[16]);

      void generate_sbox(secure_vector<uint32_t>& box,
                         uint32_t& L, uint32_t& R,
                         const uint8_t salt[16],
                         size_t salt_off) const;

      secure_vector<uint32_t> m_S, m_P;
   };

}

namespace Botan {

/**
* @mainpage Botan Crypto Library API Reference
*
* <dl>
* <dt>Abstract Base Classes<dd>
*        BlockCipher, HashFunction, KDF, MessageAuthenticationCode, RandomNumberGenerator,
*        PK_Key_Agreement, PK_Signer, PK_Verifier, StreamCipher, SymmetricAlgorithm
* <dt>Authenticated Encryption Modes<dd>
*        @ref CCM_Mode "CCM", @ref ChaCha20Poly1305_Mode "ChaCha20Poly1305", @ref EAX_Mode "EAX",
*        @ref GCM_Mode "GCM", @ref OCB_Mode "OCB", @ref SIV_Mode "SIV"
* <dt>Block Ciphers<dd>
*        @ref aes.h "AES", @ref Blowfish, @ref camellia.h "Camellia", @ref Cascade_Cipher "Cascade",
*        @ref CAST_128 "CAST-128", @ref CAST_128 "CAST-256", DES, @ref DESX "DES-X", @ref TripleDES "3DES",
*        @ref GOST_28147_89 "GOST 28147-89", IDEA, KASUMI, Lion, MISTY1, Noekeon, SEED, Serpent,
*        @ref Threefish_512 "Threefish", Twofish, XTEA
* <dt>Stream Ciphers<dd>
*        ChaCha, @ref CTR_BE "CTR", OFB, RC4, Salsa20
* <dt>Hash Functions<dd>
*        Blake2b, @ref GOST_34_11 "GOST 34.11", @ref Keccak_1600 "Keccak", MD4, MD5, @ref RIPEMD_160 "RIPEMD-160",
*        @ref SHA_160 "SHA-1", @ref SHA_224 "SHA-224", @ref SHA_256 "SHA-256", @ref SHA_384 "SHA-384",
*        @ref SHA_512 "SHA-512", @ref Skein_512 "Skein-512", Tiger, Whirlpool
* <dt>Non-Cryptographic Checksums<dd>
*        Adler32, CRC24, CRC32
* <dt>Message Authentication Codes<dd>
*        @ref CBC_MAC "CBC-MAC", CMAC, HMAC, Poly1305, SipHash, ANSI_X919_MAC
* <dt>Random Number Generators<dd>
*        AutoSeeded_RNG, HMAC_DRBG, RDRAND_RNG, System_RNG
* <dt>Key Derivation<dd>
*        HKDF, @ref KDF1 "KDF1 (IEEE 1363)", @ref KDF1_18033 "KDF1 (ISO 18033-2)", @ref KDF2 "KDF2 (IEEE 1363)",
*        @ref sp800_108.h "SP800-108", @ref SP800_56C "SP800-56C", @ref PKCS5_PBKDF1 "PBKDF1 (PKCS#5),
*        @ref PKCS5_PBKDF2 "PBKDF2 (PKCS#5)"
* <dt>Password Hashing<dd>
*        @ref bcrypt.h "bcrypt", @ref passhash9.h "passhash9"
* <dt>Public Key Cryptosystems<dd>
*        @ref dlies.h "DLIES", @ref ecies.h "ECIES", @ref elgamal.h "ElGamal", @ref mceies.h "MCEIES",
*        @ref rsa.h "RSA"
* <dt>Public Key Signature Schemes<dd>
*        @ref dsa.h "DSA", @ref ecdsa.h "ECDSA", @ref ecgdsa.h "ECGDSA", @ref eckcdsa.h "ECKCDSA",
*        @ref gost_3410.h "GOST 34.10-2001", @ref mceliece.h "McEliece"
* <dt>Key Agreement<dd>
*        @ref dh.h "DH", @ref ecdh.h "ECDH"
* <dt>Compression<dd>
*        @ref bzip2.h "bzip2", @ref lzma.h "lzma", @ref zlib.h "zlib"
* <dt>TLS<dd>
*        TLS::Client, TLS::Server, TLS::Policy, TLS::Protocol_Version, TLS::Callbacks, TLS::Ciphersuite,
*        TLS::Session, TLS::Session_Manager, Credentials_Manager
* <dt>X.509<dd>
*        X509_Certificate, X509_CRL, X509_CA, Certificate_Extension, PKCS10_Request, X509_Cert_Options,
*        Certificate_Store, Certificate_Store_In_SQL, Certificate_Store_In_SQLite
* </dl>
*/

}


namespace Botan {

/**
* Base class for all stream ciphers
*/
class BOTAN_DLL StreamCipher : public SymmetricAlgorithm
   {
   public:
      virtual ~StreamCipher() {}

      /**
      * Create an instance based on a name
      * If provider is empty then best available is chosen.
      * @param algo_spec algorithm name
      * @param provider provider implementation to use
      * @return a null pointer if the algo/provider combination cannot be found
      */
      static std::unique_ptr<StreamCipher>
         create(const std::string& algo_spec,
                const std::string& provider = "");

      /**
      * Create an instance based on a name
      * If provider is empty then best available is chosen.
      * @param algo_spec algorithm name
      * @param provider provider implementation to use
      * Throws a Lookup_Error if the algo/provider combination cannot be found
      */
      static std::unique_ptr<StreamCipher>
         create_or_throw(const std::string& algo_spec,
                         const std::string& provider = "");

      /**
      * @return list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      /**
      * Encrypt or decrypt a message
      * @param in the plaintext
      * @param out the byte array to hold the output, i.e. the ciphertext
      * @param len the length of both in and out in bytes
      */
      virtual void cipher(const uint8_t in[], uint8_t out[], size_t len) = 0;

      /**
      * Encrypt or decrypt a message
      * The message is encrypted/decrypted in place.
      * @param buf the plaintext / ciphertext
      * @param len the length of buf in bytes
      */
      void cipher1(uint8_t buf[], size_t len)
         { cipher(buf, buf, len); }

      /**
      * Encrypt a message
      * The message is encrypted/decrypted in place.
      * @param inout the plaintext / ciphertext
      */
      template<typename Alloc>
         void encipher(std::vector<uint8_t, Alloc>& inout)
         { cipher(inout.data(), inout.data(), inout.size()); }

      /**
      * Encrypt a message
      * The message is encrypted in place.
      * @param inout the plaintext / ciphertext
      */
      template<typename Alloc>
         void encrypt(std::vector<uint8_t, Alloc>& inout)
         { cipher(inout.data(), inout.data(), inout.size()); }

      /**
      * Decrypt a message in place
      * The message is decrypted in place.
      * @param inout the plaintext / ciphertext
      */
      template<typename Alloc>
         void decrypt(std::vector<uint8_t, Alloc>& inout)
         { cipher(inout.data(), inout.data(), inout.size()); }

      /**
      * Resync the cipher using the IV
      * @param iv the initialization vector
      * @param iv_len the length of the IV in bytes
      */
      virtual void set_iv(const uint8_t iv[], size_t iv_len) = 0;

      /**
      * @param iv_len the length of the IV in bytes
      * @return if the length is valid for this algorithm
      */
      virtual bool valid_iv_length(size_t iv_len) const { return (iv_len == 0); }

      /**
      * @return a new object representing the same algorithm as *this
      */
      virtual StreamCipher* clone() const = 0;

      /**
      * Set the offset and the state used later to generate the keystream
      * @param offset the offset where we begin to generate the keystream
      */
      virtual void seek(uint64_t offset) = 0;

      /**
      * @return provider information about this implementation. Default is "base",
      * might also return "sse2", "avx2", "openssl", or some other arbitrary string.
      */
      virtual std::string provider() const { return "base"; }
   };

}


namespace Botan {

/**
* This class represents Message Authentication Code (MAC) objects.
*/
class BOTAN_DLL MessageAuthenticationCode : public Buffered_Computation,
                                            public SymmetricAlgorithm
   {
   public:
      /**
      * Create an instance based on a name
      * If provider is empty then best available is chosen.
      * @param algo_spec algorithm name
      * @param provider provider implementation to use
      * @return a null pointer if the algo/provider combination cannot be found
      */
      static std::unique_ptr<MessageAuthenticationCode>
         create(const std::string& algo_spec,
                const std::string& provider = "");

      /*
      * Create an instance based on a name
      * If provider is empty then best available is chosen.
      * @param algo_spec algorithm name
      * @param provider provider implementation to use
      * Throws a Lookup_Error if algo/provider combination cannot be found
      */
      static std::unique_ptr<MessageAuthenticationCode>
         create_or_throw(const std::string& algo_spec,
                         const std::string& provider = "");

      /**
      * @return list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      virtual ~MessageAuthenticationCode() {}

      /**
      * Prepare for processing a message under the specified nonce
      *
      * Most MACs neither require nor support a nonce; for these algorithms
      * calling `start_msg` is optional and calling it with anything other than
      * an empty string is an error. One MAC which *requires* a per-message
      * nonce be specified is GMAC.
      *
      * @param nonce the message nonce bytes
      * @param nonce_len the size of len in bytes
      * Default implementation simply rejects all non-empty nonces
      * since most hash/MAC algorithms do not support randomization
      */
      virtual void start_msg(const uint8_t nonce[], size_t nonce_len)
         {
         BOTAN_UNUSED(nonce);
         if(nonce_len > 0)
            throw Invalid_IV_Length(name(), nonce_len);
         }

      /**
      * Begin processing a message with a nonce
      *
      * @param nonce the per message nonce
      */
      template<typename Alloc>
      void start(const std::vector<uint8_t, Alloc>& nonce)
         {
         start_msg(nonce.data(), nonce.size());
         }

      /**
      * Begin processing a message.
      * @param nonce the per message nonce
      * @param nonce_len length of nonce
      */
      void start(const uint8_t nonce[], size_t nonce_len)
         {
         start_msg(nonce, nonce_len);
         }

      /**
      * Begin processing a message.
      */
      void start()
         {
         return start_msg(nullptr, 0);
         }

      /**
      * Verify a MAC.
      * @param in the MAC to verify as a byte array
      * @param length the length of param in
      * @return true if the MAC is valid, false otherwise
      */
      virtual bool verify_mac(const uint8_t in[], size_t length);

      /**
      * Verify a MAC.
      * @param in the MAC to verify as a byte array
      * @return true if the MAC is valid, false otherwise
      */
      virtual bool verify_mac(const std::vector<uint8_t>& in)
         {
         return verify_mac(in.data(), in.size());
         }

      /**
      * Verify a MAC.
      * @param in the MAC to verify as a byte array
      * @return true if the MAC is valid, false otherwise
      */
      virtual bool verify_mac(const secure_vector<uint8_t>& in)
         {
         return verify_mac(in.data(), in.size());
         }

      /**
      * Get a new object representing the same algorithm as *this
      */
      virtual MessageAuthenticationCode* clone() const = 0;

      /**
      * @return provider information about this implementation. Default is "base",
      * might also return "sse2", "avx2", "openssl", or some other arbitrary string.
      */
      virtual std::string provider() const { return "base"; }

   };

typedef MessageAuthenticationCode MAC;

}


namespace Botan {

/*
* As of 1.11.26 this header is deprecated. Instead use the calls T::create and
* T::providers (as demonstrated in the implementation below).
*/

/*
* Get an algorithm object
*  NOTE: these functions create and return new objects, letting the
* caller assume ownership of them
*/

/**
* Block cipher factory method.
*
* @param algo_spec the name of the desired block cipher
* @param provider the provider to use
* @return pointer to the block cipher object
*/
BOTAN_DEPRECATED("Use BlockCipher::create")
inline BlockCipher* get_block_cipher(const std::string& algo_spec,
                                     const std::string& provider = "")
   {
   return BlockCipher::create(algo_spec, provider).release();
   }

BOTAN_DEPRECATED("Use BlockCipher::create_or_throw")
inline std::unique_ptr<BlockCipher> make_block_cipher(const std::string& algo_spec,
                                                      const std::string& provider = "")
   {
   return BlockCipher::create_or_throw(algo_spec, provider);
   }

BOTAN_DEPRECATED("Use BlockCipher::providers")
inline std::vector<std::string> get_block_cipher_providers(const std::string& algo_spec)
   {
   return BlockCipher::providers(algo_spec);
   }

/**
* Stream cipher factory method.
*
* @param algo_spec the name of the desired stream cipher
* @param provider the provider to use
* @return pointer to the stream cipher object
*/
BOTAN_DEPRECATED("Use StreamCipher::create")
inline StreamCipher* get_stream_cipher(const std::string& algo_spec,
                                       const std::string& provider = "")
   {
   return StreamCipher::create(algo_spec, provider).release();
   }

BOTAN_DEPRECATED("Use StreamCipher::create_or_throw")
inline std::unique_ptr<StreamCipher> make_stream_cipher(const std::string& algo_spec,
                                                        const std::string& provider = "")
   {
   return StreamCipher::create_or_throw(algo_spec, provider);
   }

BOTAN_DEPRECATED("Use StreamCipher::providers")
inline std::vector<std::string> get_stream_cipher_providers(const std::string& algo_spec)
   {
   return StreamCipher::providers(algo_spec);
   }

/**
* Hash function factory method.
*
* @param algo_spec the name of the desired hash function
* @param provider the provider to use
* @return pointer to the hash function object
*/
BOTAN_DEPRECATED("Use HashFunction::create")
inline HashFunction* get_hash_function(const std::string& algo_spec,
                                       const std::string& provider = "")
   {
   return HashFunction::create(algo_spec, provider).release();
   }

BOTAN_DEPRECATED("Use HashFunction::create_or_throw")
inline std::unique_ptr<HashFunction> make_hash_function(const std::string& algo_spec,
                                                        const std::string& provider = "")
   {
   return HashFunction::create_or_throw(algo_spec, provider);
   }

BOTAN_DEPRECATED("Use HashFunction::create")
inline HashFunction* get_hash(const std::string& algo_spec,
                              const std::string& provider = "")
   {
   return HashFunction::create(algo_spec, provider).release();
   }

BOTAN_DEPRECATED("Use HashFunction::providers")
inline std::vector<std::string> get_hash_function_providers(const std::string& algo_spec)
   {
   return HashFunction::providers(algo_spec);
   }

/**
* MAC factory method.
*
* @param algo_spec the name of the desired MAC
* @param provider the provider to use
* @return pointer to the MAC object
*/
BOTAN_DEPRECATED("MessageAuthenticationCode::create")
inline MessageAuthenticationCode* get_mac(const std::string& algo_spec,
                                             const std::string& provider = "")
   {
   return MessageAuthenticationCode::create(algo_spec, provider).release();
   }

BOTAN_DEPRECATED("MessageAuthenticationCode::create_or_throw")
inline std::unique_ptr<MessageAuthenticationCode> make_message_auth(const std::string& algo_spec,
                                                                       const std::string& provider = "")
   {
   return MessageAuthenticationCode::create(algo_spec, provider);
   }

BOTAN_DEPRECATED("MessageAuthenticationCode::providers")
inline std::vector<std::string> get_mac_providers(const std::string& algo_spec)
   {
   return MessageAuthenticationCode::providers(algo_spec);
   }

}


namespace Botan {

/*
* Get information describing the version
*/

/**
* Get a human-readable string identifying the version of Botan.
* No particular format should be assumed.
* @return version string
*/
BOTAN_DLL std::string version_string();

BOTAN_DLL const char* version_cstr();

/**
* Return the date this version of botan was released, in an integer of
* the form YYYYMMDD. For instance a version released on May 21, 2013
* would return the integer 20130521. If the currently running version
* is not an official release, this function will return 0 instead.
*
* @return release date, or zero if unreleased
*/
BOTAN_DLL uint32_t version_datestamp();

/**
* Get the major version number.
* @return major version number
*/
BOTAN_DLL uint32_t version_major();

/**
* Get the minor version number.
* @return minor version number
*/
BOTAN_DLL uint32_t version_minor();

/**
* Get the patch number.
* @return patch number
*/
BOTAN_DLL uint32_t version_patch();

/**
* Usable for checking that the DLL version loaded at runtime exactly
* matches the compile-time version. Call using BOTAN_VERSION_* macro
* values. Returns the empty string if an exact match, otherwise an
* appropriate message. Added with 1.11.26.
*/
BOTAN_DLL std::string
runtime_version_check(uint32_t major,
                      uint32_t minor,
                      uint32_t patch);

/*
* Macros for compile-time version checks
*/
#define BOTAN_VERSION_CODE_FOR(a,b,c) ((a << 16) | (b << 8) | (c))

/**
* Compare using BOTAN_VERSION_CODE_FOR, as in
*  # if BOTAN_VERSION_CODE < BOTAN_VERSION_CODE_FOR(1,8,0)
*  #    error "Botan version too old"
*  # endif
*/
#define BOTAN_VERSION_CODE BOTAN_VERSION_CODE_FOR(BOTAN_VERSION_MAJOR, \
                                                  BOTAN_VERSION_MINOR, \
                                                  BOTAN_VERSION_PATCH)

}



#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
#endif


namespace Botan {

/**
* Filter mixin that breaks input into blocks, useful for
* cipher modes
*/
class BOTAN_DLL Buffered_Filter
   {
   public:
      /**
      * Write bytes into the buffered filter, which will them emit them
      * in calls to buffered_block in the subclass
      * @param in the input bytes
      * @param length of in in bytes
      */
      void write(const uint8_t in[], size_t length);

      template<typename Alloc>
         void write(const std::vector<uint8_t, Alloc>& in, size_t length)
         {
         write(in.data(), length);
         }

      /**
      * Finish a message, emitting to buffered_block and buffered_final
      * Will throw an exception if less than final_minimum bytes were
      * written into the filter.
      */
      void end_msg();

      /**
      * Initialize a Buffered_Filter
      * @param block_size the function buffered_block will be called
      *        with inputs which are a multiple of this size
      * @param final_minimum the function buffered_final will be called
      *        with at least this many bytes.
      */
      Buffered_Filter(size_t block_size, size_t final_minimum);

      virtual ~Buffered_Filter() {}
   protected:
      /**
      * The block processor, implemented by subclasses
      * @param input some input bytes
      * @param length the size of input, guaranteed to be a multiple
      *        of block_size
      */
      virtual void buffered_block(const uint8_t input[], size_t length) = 0;

      /**
      * The final block, implemented by subclasses
      * @param input some input bytes
      * @param length the size of input, guaranteed to be at least
      *        final_minimum bytes
      */
      virtual void buffered_final(const uint8_t input[], size_t length) = 0;

      /**
      * @return block size of inputs
      */
      size_t buffered_block_size() const { return m_main_block_mod; }

      /**
      * @return current position in the buffer
      */
      size_t current_position() const { return m_buffer_pos; }

      /**
      * Reset the buffer position
      */
      void buffer_reset() { m_buffer_pos = 0; }
   private:
      size_t m_main_block_mod, m_final_minimum;

      secure_vector<uint8_t> m_buffer;
      size_t m_buffer_pos;
   };

}


namespace Botan {

/**
* Struct representing a particular date and time
*/
struct BOTAN_DLL calendar_point
   {
   /** The year */
   uint32_t year;

   /** The month, 1 through 12 for Jan to Dec */
   uint32_t month;

   /** The day of the month, 1 through 31 (or 28 or 30 based on month */
   uint32_t day;

   /** Hour in 24-hour form, 0 to 23 */
   uint32_t hour;

   /** Minutes in the hour, 0 to 60 */
   uint32_t minutes;

   /** Seconds in the minute, 0 to 60, but might be slightly
       larger to deal with leap seconds on some systems
   */
   uint32_t seconds;

   /**
   * Initialize a calendar_point
   * @param y the year
   * @param mon the month
   * @param d the day
   * @param h the hour
   * @param min the minute
   * @param sec the second
   */
   calendar_point(uint32_t y, uint32_t mon, uint32_t d, uint32_t h, uint32_t min, uint32_t sec) :
      year(y), month(mon), day(d), hour(h), minutes(min), seconds(sec) {}

   /**
   * Returns an STL timepoint object
   */
   std::chrono::system_clock::time_point to_std_timepoint() const;

   /**
   * Returns a human readable string of the struct's components.
   * Formatting might change over time. Currently it is RFC339 'iso-date-time'.
   */
   std::string to_string() const;
   };

/**
* Convert a time_point to a calendar_point
* @param time_point a time point from the system clock
* @return calendar_point object representing this time point
*/
BOTAN_DLL calendar_point calendar_value(
   const std::chrono::system_clock::time_point& time_point);

}


namespace Botan {

/**
* Camellia-128
*/
class BOTAN_DLL Camellia_128 final : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "Camellia-128"; }
      BlockCipher* clone() const override { return new Camellia_128; }
   private:
      void key_schedule(const uint8_t key[], size_t length) override;

      secure_vector<uint64_t> m_SK;
   };

/**
* Camellia-192
*/
class BOTAN_DLL Camellia_192 final : public Block_Cipher_Fixed_Params<16, 24>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "Camellia-192"; }
      BlockCipher* clone() const override { return new Camellia_192; }
   private:
      void key_schedule(const uint8_t key[], size_t length) override;

      secure_vector<uint64_t> m_SK;
   };

/**
* Camellia-256
*/
class BOTAN_DLL Camellia_256 final : public Block_Cipher_Fixed_Params<16, 32>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "Camellia-256"; }
      BlockCipher* clone() const override { return new Camellia_256; }
   private:
      void key_schedule(const uint8_t key[], size_t length) override;

      secure_vector<uint64_t> m_SK;
   };

}


namespace Botan {

/**
* Block Cipher Cascade
*/
class BOTAN_DLL Cascade_Cipher final : public BlockCipher
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      size_t block_size() const override { return m_block; }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(m_cipher1->maximum_keylength() +
                                         m_cipher2->maximum_keylength());
         }

      void clear() override;
      std::string name() const override;
      BlockCipher* clone() const override;

      /**
      * Create a cascade of two block ciphers
      * @param cipher1 the first cipher
      * @param cipher2 the second cipher
      */
      Cascade_Cipher(BlockCipher* cipher1, BlockCipher* cipher2);

      Cascade_Cipher(const Cascade_Cipher&) = delete;
      Cascade_Cipher& operator=(const Cascade_Cipher&) = delete;
   private:
      void key_schedule(const uint8_t[], size_t) override;

      size_t m_block;
      std::unique_ptr<BlockCipher> m_cipher1, m_cipher2;
   };


}


namespace Botan {

/**
* CAST-128
*/
class BOTAN_DLL CAST_128 final : public Block_Cipher_Fixed_Params<8, 11, 16>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "CAST-128"; }
      BlockCipher* clone() const override { return new CAST_128; }

   private:
      void key_schedule(const uint8_t[], size_t) override;

      static void cast_ks(secure_vector<uint32_t>& ks,
                          secure_vector<uint32_t>& user_key);

      secure_vector<uint32_t> m_MK;
      secure_vector<uint8_t> m_RK;
   };

}


namespace Botan {

/**
* CAST-256
*/
class BOTAN_DLL CAST_256 final : public Block_Cipher_Fixed_Params<16, 4, 32, 4>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "CAST-256"; }
      BlockCipher* clone() const override { return new CAST_256; }
   private:
      void key_schedule(const uint8_t[], size_t) override;

      secure_vector<uint32_t> m_MK;
      secure_vector<uint8_t> m_RK;
   };

}


namespace Botan {

/**
* Block Cipher Mode Padding Method
* This class is pretty limited, it cannot deal well with
* randomized padding methods, or any padding method that
* wants to add more than one block. For instance, it should
* be possible to define cipher text stealing mode as simply
* a padding mode for CBC, which happens to consume the last
* two block (and requires use of the block cipher).
*/
class BOTAN_DLL BlockCipherModePaddingMethod
   {
   public:
      /**
      * Add padding bytes to buffer.
      * @param buffer data to pad
      * @param final_block_bytes size of the final block in bytes
      * @param block_size size of each block in bytes
      */
      virtual void add_padding(secure_vector<uint8_t>& buffer,
                               size_t final_block_bytes,
                               size_t block_size) const = 0;

      /**
      * Remove padding bytes from block
      * @param block the last block
      * @param size the size of the block in bytes
      * @return number of padding bytes
      */
      virtual size_t unpad(const uint8_t block[],
                           size_t size) const = 0;

      /**
      * @param block_size of the cipher
      * @return valid block size for this padding mode
      */
      virtual bool valid_blocksize(size_t block_size) const = 0;

      /**
      * @return name of the mode
      */
      virtual std::string name() const = 0;

      /**
      * virtual destructor
      */
      virtual ~BlockCipherModePaddingMethod() {}
   };

/**
* PKCS#7 Padding
*/
class BOTAN_DLL PKCS7_Padding final : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<uint8_t>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const uint8_t[], size_t) const override;

      bool valid_blocksize(size_t bs) const override { return (bs > 0 && bs < 256); }

      std::string name() const override { return "PKCS7"; }
   };

/**
* ANSI X9.23 Padding
*/
class BOTAN_DLL ANSI_X923_Padding final : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<uint8_t>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const uint8_t[], size_t) const override;

      bool valid_blocksize(size_t bs) const override { return (bs > 0 && bs < 256); }

      std::string name() const override { return "X9.23"; }
   };

/**
* One And Zeros Padding (ISO/IEC 9797-1, padding method 2)
*/
class BOTAN_DLL OneAndZeros_Padding final : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<uint8_t>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const uint8_t[], size_t) const override;

      bool valid_blocksize(size_t bs) const override { return (bs > 0); }

      std::string name() const override { return "OneAndZeros"; }
   };

/**
* ESP Padding (RFC 4304)
*/
class BOTAN_DLL ESP_Padding final : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<uint8_t>& buffer,
                       size_t final_block_bytes,
                       size_t block_size) const override;

      size_t unpad(const uint8_t[], size_t) const override;

      bool valid_blocksize(size_t bs) const override { return (bs > 0); }

      std::string name() const override { return "ESP"; }
   };

/**
* Null Padding
*/
class BOTAN_DLL Null_Padding final : public BlockCipherModePaddingMethod
   {
   public:
      void add_padding(secure_vector<uint8_t>&, size_t, size_t) const override {}

      size_t unpad(const uint8_t[], size_t size) const override { return size; }

      bool valid_blocksize(size_t) const override { return true; }

      std::string name() const override { return "NoPadding"; }
   };

/**
* Get a block cipher padding mode by name (eg "NoPadding" or "PKCS7")
* @param algo_spec block cipher padding mode name
*/
BOTAN_DLL BlockCipherModePaddingMethod* get_bc_pad(const std::string& algo_spec);

}


namespace Botan {

/**
* CBC Mode
*/
class BOTAN_DLL CBC_Mode : public Cipher_Mode
   {
   public:
      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      size_t default_nonce_length() const override;

      bool valid_nonce_length(size_t n) const override;

      void clear() override;

      void reset() override;

   protected:
      CBC_Mode(BlockCipher* cipher, BlockCipherModePaddingMethod* padding);

      const BlockCipher& cipher() const { return *m_cipher; }

      const BlockCipherModePaddingMethod& padding() const
         {
         BOTAN_ASSERT_NONNULL(m_padding);
         return *m_padding;
         }

      secure_vector<uint8_t>& state() { return m_state; }

      uint8_t* state_ptr() { return m_state.data(); }

   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;

      void key_schedule(const uint8_t key[], size_t length) override;

      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<BlockCipherModePaddingMethod> m_padding;
      secure_vector<uint8_t> m_state;
   };

/**
* CBC Encryption
*/
class BOTAN_DLL CBC_Encryption : public CBC_Mode
   {
   public:
      /**
      * @param cipher block cipher to use
      * @param padding padding method to use
      */
      CBC_Encryption(BlockCipher* cipher, BlockCipherModePaddingMethod* padding) :
         CBC_Mode(cipher, padding) {}

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;

      size_t minimum_final_size() const override;
   };

/**
* CBC Encryption with ciphertext stealing (CBC-CS3 variant)
*/
class BOTAN_DLL CTS_Encryption final : public CBC_Encryption
   {
   public:
      /**
      * @param cipher block cipher to use
      */
      explicit CTS_Encryption(BlockCipher* cipher) : CBC_Encryption(cipher, nullptr) {}

      size_t output_length(size_t input_length) const override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

      size_t minimum_final_size() const override;

      bool valid_nonce_length(size_t n) const override;
   };

/**
* CBC Decryption
*/
class BOTAN_DLL CBC_Decryption : public CBC_Mode
   {
   public:
      /**
      * @param cipher block cipher to use
      * @param padding padding method to use
      */
      CBC_Decryption(BlockCipher* cipher, BlockCipherModePaddingMethod* padding) :
         CBC_Mode(cipher, padding), m_tempbuf(update_granularity()) {}

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;

      size_t minimum_final_size() const override;

      void reset() override;

   private:
      secure_vector<uint8_t> m_tempbuf;
   };

/**
* CBC Decryption with ciphertext stealing (CBC-CS3 variant)
*/
class BOTAN_DLL CTS_Decryption final : public CBC_Decryption
   {
   public:
      /**
      * @param cipher block cipher to use
      */
      explicit CTS_Decryption(BlockCipher* cipher) : CBC_Decryption(cipher, nullptr) {}

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

      size_t minimum_final_size() const override;

      bool valid_nonce_length(size_t n) const override;
   };

}


namespace Botan {

/**
* CBC-MAC
*/
class BOTAN_DLL CBC_MAC final : public MessageAuthenticationCode
   {
   public:
      std::string name() const override;
      MessageAuthenticationCode* clone() const override;
      size_t output_length() const override { return m_cipher->block_size(); }
      void clear() override;

      Key_Length_Specification key_spec() const override
         {
         return m_cipher->key_spec();
         }

      /**
      * @param cipher the block cipher to use
      */
      explicit CBC_MAC(BlockCipher* cipher);
   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      void key_schedule(const uint8_t[], size_t) override;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<uint8_t> m_state;
      size_t m_position = 0;
   };

}


namespace Botan {

/**
* Base class for CCM encryption and decryption
* @see RFC 3610
*/
class BOTAN_DLL CCM_Mode : public AEAD_Mode
   {
   public:
      size_t process(uint8_t buf[], size_t sz) override;

      void set_associated_data(const uint8_t ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      bool valid_nonce_length(size_t) const override;

      size_t default_nonce_length() const override;

      void clear() override;

      void reset() override;

      size_t tag_size() const override { return m_tag_size; }

   protected:
      CCM_Mode(BlockCipher* cipher, size_t tag_size, size_t L);

      size_t L() const { return m_L; }

      const BlockCipher& cipher() const { return *m_cipher; }

      void encode_length(size_t len, uint8_t out[]);

      void inc(secure_vector<uint8_t>& C);

      const secure_vector<uint8_t>& ad_buf() const { return m_ad_buf; }

      secure_vector<uint8_t>& msg_buf() { return m_msg_buf; }

      secure_vector<uint8_t> format_b0(size_t msg_size);
      secure_vector<uint8_t> format_c0();
   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;

      void key_schedule(const uint8_t key[], size_t length) override;

      const size_t m_tag_size;
      const size_t m_L;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<uint8_t> m_nonce, m_msg_buf, m_ad_buf;
   };

/**
* CCM Encryption
*/
class BOTAN_DLL CCM_Encryption final : public CCM_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      * @param tag_size is how big the auth tag will be (even values
      *                 between 4 and 16 are accepted)
      * @param L length of L parameter. The total message length
      *           must be less than 2**L bytes, and the nonce is 15-L bytes.
      */
      CCM_Encryption(BlockCipher* cipher, size_t tag_size = 16, size_t L = 3) :
         CCM_Mode(cipher, tag_size, L) {}

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }
   };

/**
* CCM Decryption
*/
class BOTAN_DLL CCM_Decryption final : public CCM_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      * @param tag_size is how big the auth tag will be (even values
      *                 between 4 and 16 are accepted)
      * @param L length of L parameter. The total message length
      *           must be less than 2**L bytes, and the nonce is 15-L bytes.
      */
      CCM_Decryption(BlockCipher* cipher, size_t tag_size = 16, size_t L = 3) :
         CCM_Mode(cipher, tag_size, L) {}

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length >= tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }
   };

}


namespace Botan {

/*
* WARNING: This API is preliminary and will change
* Currently pubkey.h does not support a 2-phase KEM scheme of
* the sort NEWHOPE exports.
*/

// TODO: change to just a secure_vector
class newhope_poly
   {
   public:
      uint16_t coeffs[1024];
      ~newhope_poly() { secure_scrub_memory(coeffs, sizeof(coeffs)); }
   };

enum Newhope_Params {
   NEWHOPE_SENDABYTES = 1824,
   NEWHOPE_SENDBBYTES = 2048,

   NEWHOPE_OFFER_BYTES  = 1824,
   NEWHOPE_ACCEPT_BYTES = 2048,
   NEWHOPE_SHARED_KEY_BYTES = 32,

   CECPQ1_OFFER_BYTES   = NEWHOPE_OFFER_BYTES + 32,
   CECPQ1_ACCEPT_BYTES  = NEWHOPE_ACCEPT_BYTES + 32,
   CECPQ1_SHARED_KEY_BYTES = NEWHOPE_SHARED_KEY_BYTES + 32
};

/**
* This chooses the XOF + hash for NewHope
* The official NewHope specification and reference implementation use
* SHA-3 and SHAKE-128. BoringSSL instead uses SHA-256 and AES-128 in
* CTR mode. CECPQ1 (x25519+NewHope) always uses BoringSSL's mode
*/
enum class Newhope_Mode {
   SHA3,
   BoringSSL
};

// offer
void BOTAN_DLL newhope_keygen(uint8_t *send,
                              newhope_poly *sk,
                              RandomNumberGenerator& rng,
                              Newhope_Mode = Newhope_Mode::SHA3);

// accept
void BOTAN_DLL newhope_sharedb(uint8_t *sharedkey,
                               uint8_t *send,
                               const uint8_t *received,
                               RandomNumberGenerator& rng,
                               Newhope_Mode mode = Newhope_Mode::SHA3);

// finish
void BOTAN_DLL newhope_shareda(uint8_t *sharedkey,
                               const newhope_poly *ska,
                               const uint8_t *received,
                               Newhope_Mode mode = Newhope_Mode::SHA3);

}


namespace Botan {

class CECPQ1_key
   {
   public:
      secure_vector<uint8_t> m_x25519;
      newhope_poly m_newhope;
   };

void BOTAN_DLL CECPQ1_offer(uint8_t* offer_message,
                            CECPQ1_key* offer_key_output,
                            RandomNumberGenerator& rng);

void BOTAN_DLL CECPQ1_accept(uint8_t* shared_key,
                             uint8_t* accept_message,
                             const uint8_t* offer_message,
                             RandomNumberGenerator& rng);

void BOTAN_DLL CECPQ1_finish(uint8_t* shared_key,
                             const CECPQ1_key& offer_key,
                             const uint8_t* accept_message);

}


namespace Botan {

/**
* Certificate validation status code
*/
enum class Certificate_Status_Code {
   OK = 0,
   VERIFIED = 0,

   // Revocation status
   OCSP_RESPONSE_GOOD = 1,
   OCSP_SIGNATURE_OK = 2,
   VALID_CRL_CHECKED = 3,
   OCSP_NO_HTTP = 4,

   // Errors
   FIRST_ERROR_STATUS = 1000,

   SIGNATURE_METHOD_TOO_WEAK = 1000,
   UNTRUSTED_HASH = 1001,
   NO_REVOCATION_DATA = 1002,

   // Time problems
   CERT_NOT_YET_VALID = 2000,
   CERT_HAS_EXPIRED = 2001,
   OCSP_NOT_YET_VALID = 2002,
   OCSP_HAS_EXPIRED = 2003,
   CRL_NOT_YET_VALID = 2004,
   CRL_HAS_EXPIRED = 2005,

   // Chain generation problems
   CERT_ISSUER_NOT_FOUND = 3000,
   CANNOT_ESTABLISH_TRUST = 3001,
   CERT_CHAIN_LOOP = 3002,
   CHAIN_LACKS_TRUST_ROOT = 3003,
   CHAIN_NAME_MISMATCH = 3004,

   // Validation errors
   POLICY_ERROR = 4000,
   INVALID_USAGE = 4001,
   CERT_CHAIN_TOO_LONG = 4002,
   CA_CERT_NOT_FOR_CERT_ISSUER = 4003,
   NAME_CONSTRAINT_ERROR = 4004,

   // Revocation errors
   CA_CERT_NOT_FOR_CRL_ISSUER = 4005,
   OCSP_CERT_NOT_LISTED = 4006,
   OCSP_BAD_STATUS = 4007,

   // Other problems
   CERT_NAME_NOMATCH = 4008,
   UNKNOWN_CRITICAL_EXTENSION = 4009,
   OCSP_SIGNATURE_ERROR = 4501,
   OCSP_ISSUER_NOT_FOUND = 4502,
   OCSP_RESPONSE_MISSING_KEYUSAGE = 4503,
   OCSP_RESPONSE_INVALID = 4504,

   // Hard failures
   CERT_IS_REVOKED = 5000,
   CRL_BAD_SIGNATURE = 5001,
   SIGNATURE_ERROR = 5002,
   CERT_PUBKEY_INVALID = 5003,
};

/**
* Convert a status code to a human readable diagnostic message
* @param code the certifcate status
* @return string literal constant, or nullptr if code unknown
*/
BOTAN_DLL const char* to_string(Certificate_Status_Code code);

}

namespace Botan {

namespace PK_Ops {

class Encryption;
class Decryption;
class Verification;
class Signature;
class Key_Agreement;
class KEM_Encryption;
class KEM_Decryption;

}

}


namespace Botan {

class RandomNumberGenerator;

/**
* Public Key Base Class.
*/
class BOTAN_DLL Public_Key
   {
   public:
      virtual ~Public_Key() {}

      /**
      * Get the name of the underlying public key scheme.
      * @return name of the public key scheme
      */
      virtual std::string algo_name() const = 0;

      /**
      * Return the estimated strength of the underlying key against
      * the best currently known attack. Note that this ignores anything
      * but pure attacks against the key itself and do not take into
      * account padding schemes, usage mistakes, etc which might reduce
      * the strength. However it does suffice to provide an upper bound.
      *
      * @return estimated strength in bits
      */
      virtual size_t estimated_strength() const = 0;

      /**
      * Return an integer value best approximating the length of the
      * primary security parameter. For example for RSA this will be
      * the size of the modulus, for ECDSA the size of the ECC group,
      * and for McEliece the size of the code will be returned.
      */
      virtual size_t key_length() const = 0;

      /**
      * Get the OID of the underlying public key scheme.
      * @return OID of the public key scheme
      */
      virtual OID get_oid() const;

      /**
      * Test the key values for consistency.
      * @param rng rng to use
      * @param strong whether to perform strong and lengthy version
      * of the test
      * @return true if the test is passed
      */
      virtual bool check_key(RandomNumberGenerator& rng,
                             bool strong) const = 0;


      /**
      * @return X.509 AlgorithmIdentifier for this key
      */
      virtual AlgorithmIdentifier algorithm_identifier() const = 0;

      /**
      * @return BER encoded public key bits
      */
      virtual std::vector<uint8_t> public_key_bits() const = 0;

      /**
      * @return X.509 subject key encoding for this key object
      */
      std::vector<uint8_t> subject_public_key() const;

      // Internal or non-public declarations follow

      /**
      * Returns more than 1 if the output of this algorithm
      * (ciphertext, signature) should be treated as more than one
      * value. This is used for algorithms like DSA and ECDSA, where
      * the (r,s) output pair can be encoded as either a plain binary
      * list or a TLV tagged DER encoding depending on the protocol.
      *
      * This function is public but applications should have few
      * reasons to ever call this.
      *
      * @return number of message parts
      */
      virtual size_t message_parts() const { return 1; }

      /**
      * Returns how large each of the message parts refered to
      * by message_parts() is
      *
      * This function is public but applications should have few
      * reasons to ever call this.
      *
      * @return size of the message parts in bits
      */
      virtual size_t message_part_size() const { return 0; }

      /**
      * This is an internal library function exposed on key types.
      * In almost all cases applications should use wrappers in pubkey.h
      *
      * Return an encryption operation for this key/params or throw
      *
      * @param rng a random number generator. The PK_Op may maintain a
      * reference to the RNG and use it many times. The rng must outlive
      * any operations which reference it.
      * @param params additional parameters
      * @param provider the provider to use
      */
      virtual std::unique_ptr<PK_Ops::Encryption>
         create_encryption_op(RandomNumberGenerator& rng,
                              const std::string& params,
                              const std::string& provider) const;

      /**
      * This is an internal library function exposed on key types.
      * In almost all cases applications should use wrappers in pubkey.h
      *
      * Return a KEM encryption operation for this key/params or throw
      *
      * @param rng a random number generator. The PK_Op may maintain a
      * reference to the RNG and use it many times. The rng must outlive
      * any operations which reference it.
      * @param params additional parameters
      * @param provider the provider to use
      */
      virtual std::unique_ptr<PK_Ops::KEM_Encryption>
         create_kem_encryption_op(RandomNumberGenerator& rng,
                                  const std::string& params,
                                  const std::string& provider) const;

      /**
      * This is an internal library function exposed on key types.
      * In almost all cases applications should use wrappers in pubkey.h
      *
      * Return a verification operation for this key/params or throw
      * @param params additional parameters
      * @param provider the provider to use
      */
      virtual std::unique_ptr<PK_Ops::Verification>
         create_verification_op(const std::string& params,
                                const std::string& provider) const;
   };

/**
* Private Key Base Class
*/
class BOTAN_DLL Private_Key : public virtual Public_Key
   {
   public:
      /**
      * @return BER encoded private key bits
      */
      virtual secure_vector<uint8_t> private_key_bits() const = 0;

      /**
      * @return PKCS #8 private key encoding for this key object
      */
      secure_vector<uint8_t> private_key_info() const;

      /**
      * @return PKCS #8 AlgorithmIdentifier for this key
      * Might be different from the X.509 identifier, but normally is not
      */
      virtual AlgorithmIdentifier pkcs8_algorithm_identifier() const
         { return algorithm_identifier(); }

      // Internal or non-public declarations follow

      /**
       * @return Hash of the PKCS #8 encoding for this key object
       */
      std::string fingerprint(const std::string& alg = "SHA") const;

      /**
      * This is an internal library function exposed on key types.
      * In almost all cases applications should use wrappers in pubkey.h
      *
      * Return an decryption operation for this key/params or throw
      *
      * @param rng a random number generator. The PK_Op may maintain a
      * reference to the RNG and use it many times. The rng must outlive
      * any operations which reference it.
      * @param params additional parameters
      * @param provider the provider to use
      *
      */
      virtual std::unique_ptr<PK_Ops::Decryption>
         create_decryption_op(RandomNumberGenerator& rng,
                              const std::string& params,
                              const std::string& provider) const;

      /**
      * This is an internal library function exposed on key types.
      * In almost all cases applications should use wrappers in pubkey.h
      *
      * Return a KEM decryption operation for this key/params or throw
      *
      * @param rng a random number generator. The PK_Op may maintain a
      * reference to the RNG and use it many times. The rng must outlive
      * any operations which reference it.
      * @param params additional parameters
      * @param provider the provider to use
      */
      virtual std::unique_ptr<PK_Ops::KEM_Decryption>
         create_kem_decryption_op(RandomNumberGenerator& rng,
                                  const std::string& params,
                                  const std::string& provider) const;

      /**
      * This is an internal library function exposed on key types.
      * In almost all cases applications should use wrappers in pubkey.h
      *
      * Return a signature operation for this key/params or throw
      *
      * @param rng a random number generator. The PK_Op may maintain a
      * reference to the RNG and use it many times. The rng must outlive
      * any operations which reference it.
      * @param params additional parameters
      * @param provider the provider to use
      */
      virtual std::unique_ptr<PK_Ops::Signature>
         create_signature_op(RandomNumberGenerator& rng,
                             const std::string& params,
                             const std::string& provider) const;

      /**
      * This is an internal library function exposed on key types.
      * In almost all cases applications should use wrappers in pubkey.h
      *
      * Return a key agreement operation for this key/params or throw
      *
      * @param rng a random number generator. The PK_Op may maintain a
      * reference to the RNG and use it many times. The rng must outlive
      * any operations which reference it.
      * @param params additional parameters
      * @param provider the provider to use
      */
      virtual std::unique_ptr<PK_Ops::Key_Agreement>
         create_key_agreement_op(RandomNumberGenerator& rng,
                                 const std::string& params,
                                 const std::string& provider) const;
   };

/**
* PK Secret Value Derivation Key
*/
class BOTAN_DLL PK_Key_Agreement_Key : public virtual Private_Key
   {
   public:
      /*
      * @return public component of this key
      */
      virtual std::vector<uint8_t> public_value() const = 0;

      virtual ~PK_Key_Agreement_Key() {}
   };

/*
* Old compat typedefs
* TODO: remove these?
*/
typedef PK_Key_Agreement_Key PK_KA_Key;
typedef Public_Key X509_PublicKey;
typedef Private_Key PKCS8_PrivateKey;

}


namespace Botan {

/**
* The two types of X509 encoding supported by Botan.
*/
enum X509_Encoding { RAW_BER, PEM };

/**
* This namespace contains functions for handling X.509 public keys
*/
namespace X509 {

/**
* BER encode a key
* @param key the public key to encode
* @return BER encoding of this key
*/
BOTAN_DLL std::vector<uint8_t> BER_encode(const Public_Key& key);

/**
* PEM encode a public key into a string.
* @param key the key to encode
* @return PEM encoded key
*/
BOTAN_DLL std::string PEM_encode(const Public_Key& key);

/**
* Create a public key from a data source.
* @param source the source providing the DER or PEM encoded key
* @return new public key object
*/
BOTAN_DLL Public_Key* load_key(DataSource& source);

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)
/**
* Create a public key from a file
* @param filename pathname to the file to load
* @return new public key object
*/
BOTAN_DLL Public_Key* load_key(const std::string& filename);
#endif

/**
* Create a public key from a memory region.
* @param enc the memory region containing the DER or PEM encoded key
* @return new public key object
*/
BOTAN_DLL Public_Key* load_key(const std::vector<uint8_t>& enc);

/**
* Copy a key.
* @param key the public key to copy
* @return new public key object
*/
BOTAN_DLL Public_Key* copy_key(const Public_Key& key);

}

}


namespace Botan {

/**
* This class represents abstract X.509 signed objects as
* in the X.500 SIGNED macro
*/
class BOTAN_DLL X509_Object : public ASN1_Object
   {
   public:
      /**
      * The underlying data that is to be or was signed
      * @return data that is or was signed
      */
      std::vector<uint8_t> tbs_data() const;

      /**
      * @return signature on tbs_data()
      */
      std::vector<uint8_t> signature() const;

      /**
      * @return signature algorithm that was used to generate signature
      */
      AlgorithmIdentifier signature_algorithm() const;

      /**
      * @return hash algorithm that was used to generate signature
      */
      std::string hash_used_for_signature() const;

      /**
      * Create a signed X509 object.
      * @param signer the signer used to sign the object
      * @param rng the random number generator to use
      * @param alg_id the algorithm identifier of the signature scheme
      * @param tbs the tbs bits to be signed
      * @return signed X509 object
      */
      static std::vector<uint8_t> make_signed(class PK_Signer* signer,
                                           RandomNumberGenerator& rng,
                                           const AlgorithmIdentifier& alg_id,
                                           const secure_vector<uint8_t>& tbs);

      /**
      * Check the signature on this data
      * @param key the public key purportedly used to sign this data
      * @return true if the signature is valid, otherwise false
      */
      bool check_signature(const Public_Key& key) const;

      /**
      * Check the signature on this data
      * @param key the public key purportedly used to sign this data
      *        the pointer will be deleted after use
      * @return true if the signature is valid, otherwise false
      */
      bool check_signature(const Public_Key* key) const;

      /**
      * DER encode an X509_Object
      * See @ref ASN1_Object::encode_into()
      */
      void encode_into(class DER_Encoder& to) const override;

      /**
      * Decode a BER encoded X509_Object
      * See @ref ASN1_Object::decode_from()
      */
      void decode_from(class BER_Decoder& from) override;

      /**
      * @return BER encoding of this
      */
      std::vector<uint8_t> BER_encode() const;

      /**
      * @return PEM encoding of this
      */
      std::string PEM_encode() const;

      virtual ~X509_Object() {}
   protected:
      X509_Object(DataSource& src, const std::string& pem_labels);
      X509_Object(const std::vector<uint8_t>& vec, const std::string& labels);

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)
      X509_Object(const std::string& file, const std::string& pem_labels);
#endif

      void do_decode();
      X509_Object() {}
      AlgorithmIdentifier m_sig_algo;
      std::vector<uint8_t> m_tbs_bits, m_sig;
   private:
      virtual void force_decode() = 0;
      void init(DataSource&, const std::string&);

      std::vector<std::string> m_PEM_labels_allowed;
      std::string m_PEM_label_pref;
   };

}


namespace Botan {

/**
* Distinguished Name
*/
class BOTAN_DLL X509_DN final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      std::multimap<OID, std::string> get_attributes() const;
      std::vector<std::string> get_attribute(const std::string&) const;

      std::multimap<std::string, std::string> contents() const;

      void add_attribute(const std::string&, const std::string&);
      void add_attribute(const OID&, const std::string&);

      static std::string deref_info_field(const std::string&);

      std::vector<uint8_t> get_bits() const;

      bool empty() const { return m_dn_info.empty(); }

      X509_DN();
      explicit X509_DN(const std::multimap<OID, std::string>&);
      explicit X509_DN(const std::multimap<std::string, std::string>&);
   private:
      std::multimap<OID, ASN1_String> m_dn_info;
      std::vector<uint8_t> m_dn_bits;
   };

bool BOTAN_DLL operator==(const X509_DN&, const X509_DN&);
bool BOTAN_DLL operator!=(const X509_DN&, const X509_DN&);
bool BOTAN_DLL operator<(const X509_DN&, const X509_DN&);

BOTAN_DLL std::ostream& operator<<(std::ostream& out, const X509_DN& dn);
BOTAN_DLL std::istream& operator>>(std::istream& in, X509_DN& dn);

}


namespace Botan {

/**
* Data Store
*
* This class is used internally by the library, and exposed for ABI
* reasons. There is no reason for applications to use this type directly.
* It will be removed in a future major release.
*/
class BOTAN_DLL Data_Store
   {
   public:
      /**
      * A search function
      */
      bool operator==(const Data_Store&) const;

      std::multimap<std::string, std::string> search_for(
         std::function<bool (std::string, std::string)> predicate) const;

      std::vector<std::string> get(const std::string&) const;

      std::string get1(const std::string& key) const;

      std::string get1(const std::string& key,
                       const std::string& default_value) const;

      std::vector<uint8_t> get1_memvec(const std::string&) const;
      uint32_t get1_uint32(const std::string&, uint32_t = 0) const;

      bool has_value(const std::string&) const;

      void add(const std::multimap<std::string, std::string>&);
      void add(const std::string&, const std::string&);
      void add(const std::string&, uint32_t);
      void add(const std::string&, const secure_vector<uint8_t>&);
      void add(const std::string&, const std::vector<uint8_t>&);
   private:
      std::multimap<std::string, std::string> m_contents;
   };

}


namespace Botan {

class X509_Certificate;

/**
* @brief X.509 GeneralName Type
*
* Handles parsing GeneralName types in their BER and canonical string
* encoding. Allows matching GeneralNames against each other using
* the rules laid out in the RFC 5280, sec. 4.2.1.10 (Name Contraints).
*/
class BOTAN_DLL GeneralName : public ASN1_Object
   {
   public:
      enum MatchResult : int
            {
            All,
            Some,
            None,
            NotFound,
            UnknownType,
            };

      /**
      * Creates an empty GeneralName.
      */
      GeneralName() : m_type(), m_name() {}

      /**
      * Creates a new GeneralName for its string format.
      * @param str type and name, colon-separated, e.g., "DNS:google.com"
      */
      GeneralName(const std::string& str);

      void encode_into(class DER_Encoder&) const override;

      void decode_from(class BER_Decoder&) override;

      /**
      * @return Type of the name. Can be DN, DNS, IP, RFC822 or URI.
      */
      const std::string& type() const { return m_type; }

      /**
      * @return The name as string. Format depends on type.
      */
      const std::string& name() const { return m_name; }

      /**
      * Checks whether a given certificate (partially) matches this name.
      * @param cert certificate to be matched
      * @return the match result
      */
      MatchResult matches(const X509_Certificate& cert) const;

   private:
      std::string m_type;
      std::string m_name;

      bool matches_dns(const std::string&) const;
      bool matches_dn(const std::string&) const;
      bool matches_ip(const std::string&) const;
   };

std::ostream& operator<<(std::ostream& os, const GeneralName& gn);

/**
* @brief A single Name Constraint
*
* The Name Constraint extension adds a minimum and maximum path
* length to a GeneralName to form a constraint. The length limits
* are currently unused.
*/
class BOTAN_DLL GeneralSubtree : public ASN1_Object
   {
   public:
      /**
      * Creates an empty name constraint.
      */
      GeneralSubtree() : m_base(), m_minimum(0), m_maximum(std::numeric_limits<std::size_t>::max())
      {}

      /***
      * Creates a new name constraint.
      * @param base name
      * @param min minimum path length
      * @param max maximum path length
      */
      GeneralSubtree(GeneralName base, size_t min, size_t max)
      : m_base(base), m_minimum(min), m_maximum(max)
      {}

      /**
      * Creates a new name constraint for its string format.
      * @param str name constraint
      */
      GeneralSubtree(const std::string& str);

      void encode_into(class DER_Encoder&) const override;

      void decode_from(class BER_Decoder&) override;

      /**
      * @return name
      */
      GeneralName base() const { return m_base; }

      /**
      * @return minimum path length
      */
      size_t minimum() const { return m_minimum; }

      /**
      * @return maximum path length
      */
      size_t maximum() const { return m_maximum; }

   private:
      GeneralName m_base;
      size_t m_minimum;
      size_t m_maximum;
   };

std::ostream& operator<<(std::ostream& os, const GeneralSubtree& gs);

/**
* @brief Name Constraints
*
* Wraps the Name Constraints associated with a certificate.
*/
class BOTAN_DLL NameConstraints
   {
   public:
      /**
      * Creates an empty name NameConstraints.
      */
      NameConstraints() : m_permitted_subtrees(), m_excluded_subtrees() {}

      /**
      * Creates NameConstraints from a list of permitted and excluded subtrees.
      * @param permitted_subtrees names for which the certificate is permitted
      * @param excluded_subtrees names for which the certificate is not permitted
      */
      NameConstraints(std::vector<GeneralSubtree>&& permitted_subtrees,
                    std::vector<GeneralSubtree>&& excluded_subtrees)
      : m_permitted_subtrees(permitted_subtrees), m_excluded_subtrees(excluded_subtrees)
      {}

      /**
      * @return permitted names
      */
      const std::vector<GeneralSubtree>& permitted() const { return m_permitted_subtrees; }

      /**
      * @return excluded names
      */
      const std::vector<GeneralSubtree>& excluded() const { return m_excluded_subtrees; }

   private:
      std::vector<GeneralSubtree> m_permitted_subtrees;
      std::vector<GeneralSubtree> m_excluded_subtrees;
};

}


namespace Botan {

/**
* X.509v3 Key Constraints.
* If updating update copy in ffi.h
*/
enum Key_Constraints {
   NO_CONSTRAINTS     = 0,
   DIGITAL_SIGNATURE  = 1 << 15,
   NON_REPUDIATION    = 1 << 14,
   KEY_ENCIPHERMENT   = 1 << 13,
   DATA_ENCIPHERMENT  = 1 << 12,
   KEY_AGREEMENT      = 1 << 11,
   KEY_CERT_SIGN      = 1 << 10,
   CRL_SIGN           = 1 << 9,
   ENCIPHER_ONLY      = 1 << 8,
   DECIPHER_ONLY      = 1 << 7
};

class Public_Key;

/**
* Check that key constraints are permitted for a specific public key.
* @param pub_key the public key on which the constraints shall be enforced on
* @param constraints the constraints that shall be enforced on the key
* @throw Exception if the given constraints are not permitted for this key
*/
BOTAN_DLL void verify_cert_constraints_valid_for_key_type(const Public_Key& pub_key,
                                                                Key_Constraints constraints);

std::string BOTAN_DLL key_constraints_to_string(Key_Constraints);

}


namespace Botan {

class X509_Certificate;

/**
* X.509v2 CRL Reason Code.
*/
enum CRL_Code {
   UNSPECIFIED            = 0,
   KEY_COMPROMISE         = 1,
   CA_COMPROMISE          = 2,
   AFFILIATION_CHANGED    = 3,
   SUPERSEDED             = 4,
   CESSATION_OF_OPERATION = 5,
   CERTIFICATE_HOLD       = 6,
   REMOVE_FROM_CRL        = 8,
   PRIVLEDGE_WITHDRAWN    = 9,
   AA_COMPROMISE          = 10,

   DELETE_CRL_ENTRY       = 0xFF00,
   OCSP_GOOD              = 0xFF01,
   OCSP_UNKNOWN           = 0xFF02
};

/**
* This class represents CRL entries
*/
class BOTAN_DLL CRL_Entry final : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;

      /**
      * Get the serial number of the certificate associated with this entry.
      * @return certificate's serial number
      */
      std::vector<uint8_t> serial_number() const { return m_serial; }

      /**
      * Get the revocation date of the certificate associated with this entry
      * @return certificate's revocation date
      */
      X509_Time expire_time() const { return m_time; }

      /**
      * Get the entries reason code
      * @return reason code
      */
      CRL_Code reason_code() const { return m_reason; }

      /**
      * Construct an empty CRL entry.
      * @param throw_on_unknown_critical_extension should we throw an exception
      * if an unknown CRL extension marked as critical is encountered
      */
      explicit CRL_Entry(bool throw_on_unknown_critical_extension = false);

      /**
      * Construct an CRL entry.
      * @param cert the certificate to revoke
      * @param reason the reason code to set in the entry
      */
      CRL_Entry(const X509_Certificate& cert,
                CRL_Code reason = UNSPECIFIED);

   private:
      bool m_throw_on_unknown_critical;
      std::vector<uint8_t> m_serial;
      X509_Time m_time;
      CRL_Code m_reason;
   };

/**
* Test two CRL entries for equality in all fields.
*/
BOTAN_DLL bool operator==(const CRL_Entry&, const CRL_Entry&);

/**
* Test two CRL entries for inequality in at least one field.
*/
BOTAN_DLL bool operator!=(const CRL_Entry&, const CRL_Entry&);

}


namespace Botan {

class X509_Certificate;

/**
* X.509 Certificate Extension
*/
class BOTAN_DLL Certificate_Extension
   {
   public:
      /**
      * @return OID representing this extension
      */
      virtual OID oid_of() const;

      /**
      * Make a copy of this extension
      * @return copy of this
      */
      virtual Certificate_Extension* copy() const = 0;

      /*
      * Add the contents of this extension into the information
      * for the subject and/or issuer, as necessary.
      * @param subject the subject info
      * @param issuer the issuer info
      */
      virtual void contents_to(Data_Store& subject,
                               Data_Store& issuer) const = 0;

      /*
      * @return specific OID name
      */
      virtual std::string oid_name() const = 0;

      /*
      * Callback visited during path validation.
      *
      * An extension can implement this callback to inspect
      * the path during path validation.
      *
      * If an error occurs during validation of this extension,
      * an appropriate status code shall be added to cert_status.
      *
      * @param subject Subject certificate that contains this extension
      * @param issuer Issuer certificate
      * @param status Certificate validation status codes for subject certificate
      * @param cert_path Certificate path which is currently validated
      * @param pos Position of subject certificate in cert_path
      */
      virtual void validate(const X509_Certificate& subject, const X509_Certificate& issuer,
            const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path,
            std::vector<std::set<Certificate_Status_Code>>& cert_status,
            size_t pos);

      virtual ~Certificate_Extension() {}
   protected:
      friend class Extensions;
      virtual bool should_encode() const { return true; }
      virtual std::vector<uint8_t> encode_inner() const = 0;
      virtual void decode_inner(const std::vector<uint8_t>&) = 0;
   };

/**
* X.509 Certificate Extension List
*/
class BOTAN_DLL Extensions : public ASN1_Object
   {
   public:
      void encode_into(class DER_Encoder&) const override;
      void decode_from(class BER_Decoder&) override;
      void contents_to(Data_Store&, Data_Store&) const;

      /**
      * Adds a new extension to the list.
      * @param extn the certificate extension
      * @param critical whether this extension should be marked as critical
      * @throw Invalid_Argument if the extension is already present in the list
      */
      void add(Certificate_Extension* extn, bool critical = false);

      /**
      * Adds an extension to the list or replaces it.
      * @param extn the certificate extension
      * @param critical whether this extension should be marked as critical
      */
      void replace(Certificate_Extension* extn, bool critical = false);

      /**
      * Searches for an extension by OID and returns the result.
      * Only the known extensions types declared in this header
      * are searched for by this function.
      * @return Pointer to extension with oid, nullptr if not found.
      */
      std::unique_ptr<Certificate_Extension> get(const OID& oid) const;

      /**
      * Searches for an extension by OID and returns the result.
      * Only the unknown extensions, that is, extensions
      * types that are not declared in this header, are searched
      * for by this function.
      * @return Pointer to extension with oid, nullptr if not found.
      */
      template<typename T>
      std::unique_ptr<T> get_raw(const OID& oid)
      {
      try
         {
         if(m_extensions_raw.count(oid) > 0)
            {
            std::unique_ptr<T> ext(new T);
            ext->decode_inner(m_extensions_raw[oid].first);
            return std::move(ext);
            }
         }
      catch(std::exception& e)
         {
         throw Decoding_Error("Exception while decoding extension " +
                              oid.as_string() + ": " + e.what());
         }
      return nullptr;
      }

      /**
      * Returns the list of extensions together with the corresponding
      * criticality flag. Only contains the known extensions
      * types declared in this header.
      */
      std::vector<std::pair<std::unique_ptr<Certificate_Extension>, bool>> extensions() const;

      /**
      * Returns the list of extensions as raw, encoded bytes
      * together with the corresponding criticality flag.
      * Contains all extensions, known as well as unknown extensions.
      */
      std::map<OID, std::pair<std::vector<uint8_t>, bool>> extensions_raw() const;

      Extensions& operator=(const Extensions&);

      Extensions(const Extensions&);

      /**
      * @param st whether to throw an exception when encountering an unknown
      * extension type during decoding
      */
      explicit Extensions(bool st = true) : m_throw_on_unknown_critical(st) {}

   private:
      static Certificate_Extension* create_extension(const OID&, bool);

      std::vector<std::pair<std::unique_ptr<Certificate_Extension>, bool>> m_extensions;
      bool m_throw_on_unknown_critical;
      std::map<OID, std::pair<std::vector<uint8_t>, bool>> m_extensions_raw;
   };

namespace Cert_Extension {

static const size_t NO_CERT_PATH_LIMIT = 0xFFFFFFF0;

/**
* Basic Constraints Extension
*/
class BOTAN_DLL Basic_Constraints final : public Certificate_Extension
   {
   public:
      Basic_Constraints* copy() const override
         { return new Basic_Constraints(m_is_ca, m_path_limit); }

      Basic_Constraints(bool ca = false, size_t limit = 0) :
         m_is_ca(ca), m_path_limit(limit) {}

      bool get_is_ca() const { return m_is_ca; }
      size_t get_path_limit() const;

   private:
      std::string oid_name() const override
         { return "X509v3.BasicConstraints"; }

      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      bool m_is_ca;
      size_t m_path_limit;
   };

/**
* Key Usage Constraints Extension
*/
class BOTAN_DLL Key_Usage final : public Certificate_Extension
   {
   public:
      Key_Usage* copy() const override { return new Key_Usage(m_constraints); }

      explicit Key_Usage(Key_Constraints c = NO_CONSTRAINTS) : m_constraints(c) {}

      Key_Constraints get_constraints() const { return m_constraints; }

   private:
      std::string oid_name() const override { return "X509v3.KeyUsage"; }

      bool should_encode() const override
         { return (m_constraints != NO_CONSTRAINTS); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      Key_Constraints m_constraints;
   };

/**
* Subject Key Identifier Extension
*/
class BOTAN_DLL Subject_Key_ID final : public Certificate_Extension
   {
   public:
      Subject_Key_ID* copy() const override
         { return new Subject_Key_ID(m_key_id); }

      Subject_Key_ID() {}
      explicit Subject_Key_ID(const std::vector<uint8_t>&);

      std::vector<uint8_t> get_key_id() const { return m_key_id; }
   private:
      std::string oid_name() const override
         { return "X509v3.SubjectKeyIdentifier"; }

      bool should_encode() const override { return (m_key_id.size() > 0); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<uint8_t> m_key_id;
   };

/**
* Authority Key Identifier Extension
*/
class BOTAN_DLL Authority_Key_ID final : public Certificate_Extension
   {
   public:
      Authority_Key_ID* copy() const override
         { return new Authority_Key_ID(m_key_id); }

      Authority_Key_ID() {}
      explicit Authority_Key_ID(const std::vector<uint8_t>& k) : m_key_id(k) {}

      std::vector<uint8_t> get_key_id() const { return m_key_id; }

   private:
      std::string oid_name() const override
         { return "X509v3.AuthorityKeyIdentifier"; }

      bool should_encode() const override { return (m_key_id.size() > 0); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<uint8_t> m_key_id;
   };

/**
* Alternative Name Extension Base Class
*/
class BOTAN_DLL Alternative_Name : public Certificate_Extension
   {
   public:
      AlternativeName get_alt_name() const { return m_alt_name; }

   protected:
      Alternative_Name(const AlternativeName&, const std::string& oid_name);

      Alternative_Name(const std::string&, const std::string&);

   private:
      std::string oid_name() const override { return m_oid_name_str; }

      bool should_encode() const override { return m_alt_name.has_items(); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::string m_oid_name_str;
      AlternativeName m_alt_name;
   };

/**
* Subject Alternative Name Extension
*/
class BOTAN_DLL Subject_Alternative_Name : public Alternative_Name
   {
   public:
      Subject_Alternative_Name* copy() const override
         { return new Subject_Alternative_Name(get_alt_name()); }

      explicit Subject_Alternative_Name(const AlternativeName& = AlternativeName());
   };

/**
* Issuer Alternative Name Extension
*/
class BOTAN_DLL Issuer_Alternative_Name : public Alternative_Name
   {
   public:
      Issuer_Alternative_Name* copy() const override
         { return new Issuer_Alternative_Name(get_alt_name()); }

      explicit Issuer_Alternative_Name(const AlternativeName& = AlternativeName());
   };

/**
* Extended Key Usage Extension
*/
class BOTAN_DLL Extended_Key_Usage final : public Certificate_Extension
   {
   public:
      Extended_Key_Usage* copy() const override
         { return new Extended_Key_Usage(m_oids); }

      Extended_Key_Usage() {}
      explicit Extended_Key_Usage(const std::vector<OID>& o) : m_oids(o) {}

      std::vector<OID> get_oids() const { return m_oids; }

   private:
      std::string oid_name() const override
         { return "X509v3.ExtendedKeyUsage"; }

      bool should_encode() const override { return (m_oids.size() > 0); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<OID> m_oids;
   };

/**
* Name Constraints
*/
class BOTAN_DLL Name_Constraints : public Certificate_Extension
   {
   public:
      Name_Constraints* copy() const override
         { return new Name_Constraints(m_name_constraints); }

      Name_Constraints() {}
      Name_Constraints(const NameConstraints &nc) : m_name_constraints(nc) {}

      void validate(const X509_Certificate& subject, const X509_Certificate& issuer,
            const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path,
            std::vector<std::set<Certificate_Status_Code>>& cert_status,
            size_t pos) override;

   private:
      std::string oid_name() const override
         { return "X509v3.NameConstraints"; }

      bool should_encode() const override { return true; }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      NameConstraints m_name_constraints;
   };

/**
* Certificate Policies Extension
*/
class BOTAN_DLL Certificate_Policies final : public Certificate_Extension
   {
   public:
      Certificate_Policies* copy() const override
         { return new Certificate_Policies(m_oids); }

      Certificate_Policies() {}
      explicit Certificate_Policies(const std::vector<OID>& o) : m_oids(o) {}

      std::vector<OID> get_oids() const { return m_oids; }

   private:
      std::string oid_name() const override
         { return "X509v3.CertificatePolicies"; }

      bool should_encode() const override { return (m_oids.size() > 0); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<OID> m_oids;
   };

class BOTAN_DLL Authority_Information_Access final : public Certificate_Extension
   {
   public:
      Authority_Information_Access* copy() const override
         { return new Authority_Information_Access(m_ocsp_responder); }

      Authority_Information_Access() {}

      explicit Authority_Information_Access(const std::string& ocsp) :
         m_ocsp_responder(ocsp) {}

   private:
      std::string oid_name() const override
         { return "PKIX.AuthorityInformationAccess"; }

      bool should_encode() const override { return (!m_ocsp_responder.empty()); }

      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;

      void contents_to(Data_Store&, Data_Store&) const override;

      std::string m_ocsp_responder;
   };

/**
* CRL Number Extension
*/
class BOTAN_DLL CRL_Number final : public Certificate_Extension
   {
   public:
      CRL_Number* copy() const override;

      CRL_Number() : m_has_value(false), m_crl_number(0) {}
      CRL_Number(size_t n) : m_has_value(true), m_crl_number(n) {}

      size_t get_crl_number() const;

   private:
      std::string oid_name() const override { return "X509v3.CRLNumber"; }

      bool should_encode() const override { return m_has_value; }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      bool m_has_value;
      size_t m_crl_number;
   };

/**
* CRL Entry Reason Code Extension
*/
class BOTAN_DLL CRL_ReasonCode final : public Certificate_Extension
   {
   public:
      CRL_ReasonCode* copy() const override
         { return new CRL_ReasonCode(m_reason); }

      explicit CRL_ReasonCode(CRL_Code r = UNSPECIFIED) : m_reason(r) {}

      CRL_Code get_reason() const { return m_reason; }

   private:
      std::string oid_name() const override { return "X509v3.ReasonCode"; }

      bool should_encode() const override { return (m_reason != UNSPECIFIED); }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      CRL_Code m_reason;
   };

/**
* CRL Distribution Points Extension
*/
class BOTAN_DLL CRL_Distribution_Points final : public Certificate_Extension
   {
   public:
      class BOTAN_DLL Distribution_Point final : public ASN1_Object
         {
         public:
            void encode_into(class DER_Encoder&) const override;
            void decode_from(class BER_Decoder&) override;

            const AlternativeName& point() const { return m_point; }
         private:
            AlternativeName m_point;
         };

      CRL_Distribution_Points* copy() const override
         { return new CRL_Distribution_Points(m_distribution_points); }

      CRL_Distribution_Points() {}

      explicit CRL_Distribution_Points(const std::vector<Distribution_Point>& points) :
         m_distribution_points(points) {}

      std::vector<Distribution_Point> distribution_points() const
         { return m_distribution_points; }

   private:
      std::string oid_name() const override
         { return "X509v3.CRLDistributionPoints"; }

      bool should_encode() const override
         { return !m_distribution_points.empty(); }

      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      std::vector<Distribution_Point> m_distribution_points;
   };

/**
* An unknown X.509 extension marked as critical
* Will always add a failure to the path validation result.
*/
class BOTAN_DLL Unknown_Critical_Extension final : public Certificate_Extension
   {
   public:
      explicit Unknown_Critical_Extension(OID oid) : m_oid(oid) {}

      Unknown_Critical_Extension* copy() const override
         { return new Unknown_Critical_Extension(m_oid); }

      OID oid_of() const override
         { return m_oid; };

      void validate(const X509_Certificate&, const X509_Certificate&,
      		const std::vector<std::shared_ptr<const X509_Certificate>>&,
      		std::vector<std::set<Certificate_Status_Code>>& cert_status,
      		size_t pos) override
         {
         cert_status.at(pos).insert(Certificate_Status_Code::UNKNOWN_CRITICAL_EXTENSION);
         }

   private:
      std::string oid_name() const override
         { return "Unknown OID name"; }

      bool should_encode() const override { return false; }
      std::vector<uint8_t> encode_inner() const override;
      void decode_inner(const std::vector<uint8_t>&) override;
      void contents_to(Data_Store&, Data_Store&) const override;

      OID m_oid;
   };

}

}


namespace Botan {

enum class Usage_Type
   {
   UNSPECIFIED, // no restrictions
   TLS_SERVER_AUTH,
   TLS_CLIENT_AUTH,
   CERTIFICATE_AUTHORITY,
   OCSP_RESPONDER
   };

/**
* This class represents X.509 Certificate
*/
class BOTAN_DLL X509_Certificate : public X509_Object
   {
   public:
      /**
      * Get the public key associated with this certificate.
      * @return subject public key of this certificate
      */
      Public_Key* subject_public_key() const;

      /**
      * Get the public key associated with this certificate.
      * @return subject public key of this certificate
      */
      std::vector<uint8_t> subject_public_key_bits() const;

      /**
      * Get the bit string of the public key associated with this certificate
      * @return subject public key of this certificate
      */
      std::vector<uint8_t> subject_public_key_bitstring() const;

      /**
      * Get the SHA-1 bit string of the public key associated with this certificate.
      * This is used for OCSP among other protocols
      * @return hash of subject public key of this certificate
      */
      std::vector<uint8_t> subject_public_key_bitstring_sha1() const;

      /**
      * Get the certificate's issuer distinguished name (DN).
      * @return issuer DN of this certificate
      */
      X509_DN issuer_dn() const;

      /**
      * Get the certificate's subject distinguished name (DN).
      * @return subject DN of this certificate
      */
      X509_DN subject_dn() const;

      /**
      * Get a value for a specific subject_info parameter name.
      * @param name the name of the parameter to look up. Possible names are
      * "X509.Certificate.version", "X509.Certificate.serial",
      * "X509.Certificate.start", "X509.Certificate.end",
      * "X509.Certificate.v2.key_id", "X509.Certificate.public_key",
      * "X509v3.BasicConstraints.path_constraint",
      * "X509v3.BasicConstraints.is_ca", "X509v3.NameConstraints",
      * "X509v3.ExtendedKeyUsage", "X509v3.CertificatePolicies",
      * "X509v3.SubjectKeyIdentifier" or "X509.Certificate.serial".
      * @return value(s) of the specified parameter
      */
      std::vector<std::string> subject_info(const std::string& name) const;

      /**
      * Get a value for a specific subject_info parameter name.
      * @param name the name of the parameter to look up. Possible names are
      * "X509.Certificate.v2.key_id" or "X509v3.AuthorityKeyIdentifier".
      * @return value(s) of the specified parameter
      */
      std::vector<std::string> issuer_info(const std::string& name) const;

      /**
      * Raw subject DN
      */
      std::vector<uint8_t> raw_issuer_dn() const;

      /**
      * Raw issuer DN
      */
      std::vector<uint8_t> raw_subject_dn() const;

      /**
      * Get the notBefore of the certificate.
      * @return notBefore of the certificate
      */
      std::string start_time() const;

      /**
      * Get the notAfter of the certificate.
      * @return notAfter of the certificate
      */
      std::string end_time() const;

      /**
      * Get the X509 version of this certificate object.
      * @return X509 version
      */
      uint32_t x509_version() const;

      /**
      * Get the serial number of this certificate.
      * @return certificates serial number
      */
      std::vector<uint8_t> serial_number() const;

      /**
      * Get the DER encoded AuthorityKeyIdentifier of this certificate.
      * @return DER encoded AuthorityKeyIdentifier
      */
      std::vector<uint8_t> authority_key_id() const;

      /**
      * Get the DER encoded SubjectKeyIdentifier of this certificate.
      * @return DER encoded SubjectKeyIdentifier
      */
      std::vector<uint8_t> subject_key_id() const;

      /**
      * Check whether this certificate is self signed.
      * @return true if this certificate is self signed
      */
      bool is_self_signed() const { return m_self_signed; }

      /**
      * Check whether this certificate is a CA certificate.
      * @return true if this certificate is a CA certificate
      */
      bool is_CA_cert() const;

      /**
      * Returns true if the specified @param usage is set in the key usage extension 
      * or if no key usage constraints are set at all.
      * To check if a certain key constraint is set in the certificate 
      * use @see X509_Certificate#has_constraints.
      */
      bool allowed_usage(Key_Constraints usage) const;

      /**
      * Returns true if the specified @param usage is set in the extended key usage extension
      * or if no extended key usage constraints are set at all.
      * To check if a certain extended key constraint is set in the certificate
      * use @see X509_Certificate#has_ex_constraint.
      */
      bool allowed_extended_usage(const std::string& usage) const;

      /**
      * Returns true if the required key and extended key constraints are set in the certificate
      * for the specified @param usage or if no key constraints are set in both the key usage
      * and extended key usage extension.
      */
      bool allowed_usage(Usage_Type usage) const;

      /// Returns true if the specified @param constraints are included in the key usage extension.
      bool has_constraints(Key_Constraints constraints) const;
      
      /**
      * Returns true if and only if @param ex_constraint (referring to an extended key
      * constraint, eg "PKIX.ServerAuth") is included in the extended
      * key extension.
      */
      bool has_ex_constraint(const std::string& ex_constraint) const;

      /**
      * Get the path limit as defined in the BasicConstraints extension of
      * this certificate.
      * @return path limit
      */
      uint32_t path_limit() const;

      /**
      * Check whenever a given X509 Extension is marked critical in this
      * certificate.
      */
      bool is_critical(const std::string& ex_name) const;

      /**
      * Get the key constraints as defined in the KeyUsage extension of this
      * certificate.
      * @return key constraints
      */
      Key_Constraints constraints() const;

      /**
      * Get the key constraints as defined in the ExtendedKeyUsage
      * extension of this certificate.
      * @return key constraints
      */
      std::vector<std::string> ex_constraints() const;

      /**
      * Get the name constraints as defined in the NameConstraints
      * extension of this certificate.
      * @return name constraints
      */
      NameConstraints name_constraints() const;

      /**
      * Get the policies as defined in the CertificatePolicies extension
      * of this certificate.
      * @return certificate policies
      */
      std::vector<std::string> policies() const;

      /**
      * Get all extensions of this certificate.
      * @return certificate extensions
      */
      Extensions v3_extensions() const;

      /**
      * Return the listed address of an OCSP responder, or empty if not set
      */
      std::string ocsp_responder() const;

      /**
      * Return the CRL distribution point, or empty if not set
      */
      std::string crl_distribution_point() const;

      /**
      * @return a string describing the certificate
      */
      std::string to_string() const;

      /**
      * @return a fingerprint of the certificate
      * @param hash_name hash function used to calculate the fingerprint
      */
      std::string fingerprint(const std::string& hash_name = "SHA-1") const;

      /**
      * Check if a certain DNS name matches up with the information in
      * the cert
      * @param name DNS name to match
      */
      bool matches_dns_name(const std::string& name) const;

      /**
      * Check to certificates for equality.
      * @return true both certificates are (binary) equal
      */
      bool operator==(const X509_Certificate& other) const;

      /**
      * Impose an arbitrary (but consistent) ordering
      * @return true if this is less than other by some unspecified criteria
      */
      bool operator<(const X509_Certificate& other) const;

      /**
      * Create a certificate from a data source providing the DER or
      * PEM encoded certificate.
      * @param source the data source
      */
      explicit X509_Certificate(DataSource& source);

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)
      /**
      * Create a certificate from a file containing the DER or PEM
      * encoded certificate.
      * @param filename the name of the certificate file
      */
      explicit X509_Certificate(const std::string& filename);
#endif

      /**
      * Create a certificate from a buffer
      * @param in the buffer containing the DER-encoded certificate
      */
      explicit X509_Certificate(const std::vector<uint8_t>& in);

      X509_Certificate(const X509_Certificate& other) = default;

      X509_Certificate& operator=(const X509_Certificate& other) = default;

   private:
      void force_decode() override;
      friend class X509_CA;
      friend class BER_Decoder;

      X509_Certificate() {}

      Data_Store m_subject, m_issuer;
      bool m_self_signed;
      Extensions m_v3_extensions;
   };

/**
* Check two certificates for inequality
* @param cert1 The first certificate
* @param cert2 The second certificate
* @return true if the arguments represent different certificates,
* false if they are binary identical
*/
BOTAN_DLL bool operator!=(const X509_Certificate& cert1, const X509_Certificate& cert2);

/*
* Data Store Extraction Operations
*/

/*
* Create and populate a X509_DN
* @param info data store containing DN information
* @return DN containing attributes from data store
*/
BOTAN_DLL X509_DN create_dn(const Data_Store& info);

/*
* Create and populate an AlternativeName
* @param info data store containing AlternativeName information
* @return AlternativeName containing attributes from data store
*/
BOTAN_DLL AlternativeName create_alt_name(const Data_Store& info);

}


namespace Botan {

class X509_Certificate;

/**
* This class represents X.509 Certificate Revocation Lists (CRLs).
*/
class BOTAN_DLL X509_CRL final : public X509_Object
   {
   public:
      /**
      * This class represents CRL related errors.
      */
      struct BOTAN_DLL X509_CRL_Error : public Exception
         {
         explicit X509_CRL_Error(const std::string& error) :
            Exception("X509_CRL: " + error) {}
         };

      /**
      * Check if this particular certificate is listed in the CRL
      */
      bool is_revoked(const X509_Certificate& cert) const;

      /**
      * Get the entries of this CRL in the form of a vector.
      * @return vector containing the entries of this CRL.
      */
      std::vector<CRL_Entry> get_revoked() const;

      /**
      * Get the issuer DN of this CRL.
      * @return CRLs issuer DN
      */
      X509_DN issuer_dn() const;

      /**
      * Get the AuthorityKeyIdentifier of this CRL.
      * @return this CRLs AuthorityKeyIdentifier
      */
      std::vector<uint8_t> authority_key_id() const;

      /**
      * Get the serial number of this CRL.
      * @return CRLs serial number
      */
      uint32_t crl_number() const;

      /**
      * Get the CRL's thisUpdate value.
      * @return CRLs thisUpdate
      */
      X509_Time this_update() const;

      /**
      * Get the CRL's nextUpdate value.
      * @return CRLs nextdUpdate
      */
      X509_Time next_update() const;

      /**
      * Construct a CRL from a data source.
      * @param source the data source providing the DER or PEM encoded CRL.
      * @param throw_on_unknown_critical should we throw an exception
      * if an unknown CRL extension marked as critical is encountered.
      */
      X509_CRL(DataSource& source, bool throw_on_unknown_critical = false);

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)
      /**
      * Construct a CRL from a file containing the DER or PEM encoded CRL.
      * @param filename the name of the CRL file
      * @param throw_on_unknown_critical should we throw an exception
      * if an unknown CRL extension marked as critical is encountered.
      */
      X509_CRL(const std::string& filename,
               bool throw_on_unknown_critical = false);
#endif

      /**
      * Construct a CRL from a binary vector
      * @param vec the binary (DER) representation of the CRL
      * @param throw_on_unknown_critical should we throw an exception
      * if an unknown CRL extension marked as critical is encountered.
      */
      X509_CRL(const std::vector<uint8_t>& vec,
               bool throw_on_unknown_critical = false);

      /**
      * Construct a CRL
      * @param issuer issuer of this CRL
      * @param thisUpdate valid from
      * @param nextUpdate valid until
      * @param revoked entries to be included in the CRL
      */
      X509_CRL(const X509_DN& issuer, const X509_Time& thisUpdate,
               const X509_Time& nextUpdate, const std::vector<CRL_Entry>& revoked);

   private:
      void force_decode() override;

      bool m_throw_on_unknown_critical;
      std::vector<CRL_Entry> m_revoked;
      Data_Store m_info;
   };

}


namespace Botan {

/**
* Certificate Store Interface
*/
class BOTAN_DLL Certificate_Store
   {
   public:
      virtual ~Certificate_Store() {}

      /**
      * Find a certificate by Subject DN and (optionally) key identifier
      * @param subject_dn the subject's distinguished name
      * @param key_id an optional key id
      * @return a matching certificate or nullptr otherwise
      */
      virtual std::shared_ptr<const X509_Certificate>
         find_cert(const X509_DN& subject_dn, const std::vector<uint8_t>& key_id) const = 0;

      /**
      * Find a certificate by searching for one with a matching SHA-1 hash of
      * public key. Used for OCSP.
      * @param key_hash SHA-1 hash of the subject's public key
      * @return a matching certificate or nullptr otherwise
      */
      virtual std::shared_ptr<const X509_Certificate>
         find_cert_by_pubkey_sha1(const std::vector<uint8_t>& key_hash) const = 0;

      /**
      * Finds a CRL for the given certificate
      * @param subject the subject certificate
      * @return the CRL for subject or nullptr otherwise
      */
      virtual std::shared_ptr<const X509_CRL> find_crl_for(const X509_Certificate& subject) const;

      /**
      * @return whether the certificate is known
      * @param cert certififcate to be searched
      */
      bool certificate_known(const X509_Certificate& cert) const
         {
         return find_cert(cert.subject_dn(), cert.subject_key_id()) != nullptr;
         }

      // remove this (used by TLS::Server)
      virtual std::vector<X509_DN> all_subjects() const = 0;
   };

/**
* In Memory Certificate Store
*/
class BOTAN_DLL Certificate_Store_In_Memory : public Certificate_Store
   {
   public:
      /**
      * Attempt to parse all files in dir (including subdirectories)
      * as certificates. Ignores errors.
      */
      explicit Certificate_Store_In_Memory(const std::string& dir);

      /**
      * Adds given certificate to the store.
      */
      explicit Certificate_Store_In_Memory(const X509_Certificate& cert);

      /**
      * Create an empty store.
      */
      Certificate_Store_In_Memory() {}

      /**
      * Add a certificate to the store.
      * @param cert certificate to be added
      */
      void add_certificate(const X509_Certificate& cert);

      /**
      * Add a certificate already in a shared_ptr to the store.
      * @param cert certificate to be added
      */
      void add_certificate(std::shared_ptr<const X509_Certificate> cert);

      /**
      * Add a certificate revocation list (CRL) to the store.
      * @param crl CRL to be added
      */
      void add_crl(const X509_CRL& crl);

      /**
      * Add a certificate revocation list (CRL) to the store as a shared_ptr
      * @param crl CRL to be added
      */
      void add_crl(std::shared_ptr<const X509_CRL> crl);

      /**
      * @return DNs for all certificates managed by the store
      */
      std::vector<X509_DN> all_subjects() const override;

      /*
      * Find a certificate by Subject DN and (optionally) key identifier
      */
      std::shared_ptr<const X509_Certificate> find_cert(
         const X509_DN& subject_dn,
         const std::vector<uint8_t>& key_id) const override;

      std::shared_ptr<const X509_Certificate>
         find_cert_by_pubkey_sha1(const std::vector<uint8_t>& key_hash) const override;

      /**
      * Finds a CRL for the given certificate
      */
      std::shared_ptr<const X509_CRL> find_crl_for(const X509_Certificate& subject) const override;
   private:
      // TODO: Add indexing on the DN and key id to avoid linear search
      std::vector<std::shared_ptr<const X509_Certificate>> m_certs;
      std::vector<std::shared_ptr<const X509_CRL>> m_crls;
   };

}


namespace Botan {

class BOTAN_DLL SQL_Database
   {
   public:

      class BOTAN_DLL SQL_DB_Error : public Exception
         {
         public:
            explicit SQL_DB_Error(const std::string& what) : Exception("SQL database", what) {}
         };

      class BOTAN_DLL Statement
         {
         public:
            /* Bind statement parameters */
            virtual void bind(int column, const std::string& str) = 0;

            virtual void bind(int column, size_t i) = 0;

            virtual void bind(int column, std::chrono::system_clock::time_point time) = 0;

            virtual void bind(int column, const std::vector<uint8_t>& blob) = 0;

            virtual void bind(int column, const uint8_t* data, size_t len) = 0;

            /* Get output */
            virtual std::pair<const uint8_t*, size_t> get_blob(int column) = 0;

            virtual size_t get_size_t(int column) = 0;

            /* Run to completion */
            virtual size_t spin() = 0;

            /* Maybe update */
            virtual bool step() = 0;

            virtual ~Statement() {}
         };

      /*
      * Create a new statement for execution.
      * Use ?1, ?2, ?3, etc for parameters to set later with bind
      */
      virtual std::shared_ptr<Statement> new_statement(const std::string& base_sql) const = 0;

      virtual size_t row_count(const std::string& table_name) = 0;

      virtual void create_table(const std::string& table_schema) = 0;

      virtual ~SQL_Database() {}
};

}


namespace Botan {

class RandomNumberGenerator;

/**
 * Certificate and private key store backed by an SQL database.
 */
class BOTAN_DLL Certificate_Store_In_SQL : public Certificate_Store
   {
   public:
      /**
      * Create/open a certificate store.
      * @param db underlying database storage
      * @param passwd password to encrypt private keys in the database
      * @param rng used for encrypting keys
      * @param table_prefix optional prefix for db table names
      */
      explicit Certificate_Store_In_SQL(const std::shared_ptr<SQL_Database> db,
                                        const std::string& passwd,
                                        RandomNumberGenerator& rng,
                                        const std::string& table_prefix = "");

      /**
      * Returns the first certificate with matching subject DN and optional key ID.
      */
      virtual std::shared_ptr<const X509_Certificate>
         find_cert(const X509_DN& subject_dn, const std::vector<uint8_t>& key_id) const override;

      std::shared_ptr<const X509_Certificate>
         find_cert_by_pubkey_sha1(const std::vector<uint8_t>& key_hash) const override;

      /**
      * Returns all subject DNs known to the store instance.
      */
      virtual std::vector<X509_DN> all_subjects() const override;

      /**
      * Inserts "cert" into the store, returns false if the certificate is
      * already known and true if insertion was successful.
      */
      bool insert_cert(const X509_Certificate& cert);

      /**
      * Removes "cert" from the store. Returns false if the certificate could not
      * be found and true if removal was successful.
      */
      bool remove_cert(const X509_Certificate& cert);

      /// Returns the private key for "cert" or an empty shared_ptr if none was found.
      std::shared_ptr<const Private_Key> find_key(const X509_Certificate&) const;

      /// Returns all certificates for private key "key".
      std::vector<std::shared_ptr<const X509_Certificate>>
         find_certs_for_key(const Private_Key& key) const;

      /**
      * Inserts "key" for "cert" into the store, returns false if the key is
      * already known and true if insertion was successful.
      */
      bool insert_key(const X509_Certificate& cert, const Private_Key& key);

      /// Removes "key" from the store.
      void remove_key(const Private_Key& key);

      /// Marks "cert" as revoked starting from "time".
      void revoke_cert(const X509_Certificate&, CRL_Code, const X509_Time& time = X509_Time());

      /// Reverses the revokation for "cert".
      void affirm_cert(const X509_Certificate&);

      /**
      * Generates Certificate Revocation Lists for all certificates marked as revoked.
      * A CRL is returned for each unique issuer DN.
      */
      std::vector<X509_CRL> generate_crls() const;

      /**
      * Generates a CRL for all certificates issued by the given issuer.
      */
      virtual std::shared_ptr<const X509_CRL>
         find_crl_for(const X509_Certificate& issuer) const override;

   private:
      RandomNumberGenerator& m_rng;
      std::shared_ptr<SQL_Database> m_database;
      std::string m_prefix;
      std::string m_password;
      mutex_type m_mutex;
   };

}

namespace Botan {

/**
* CFB Mode
*/
class BOTAN_DLL CFB_Mode : public Cipher_Mode
   {
   public:
      std::string name() const override;

      size_t update_granularity() const override;

      size_t minimum_final_size() const override;

      Key_Length_Specification key_spec() const override;

      size_t output_length(size_t input_length) const override;

      size_t default_nonce_length() const override;

      bool valid_nonce_length(size_t n) const override;

      void clear() override;

      void reset() override;
   protected:
      CFB_Mode(BlockCipher* cipher, size_t feedback_bits);

      const BlockCipher& cipher() const { return *m_cipher; }

      size_t feedback() const { return m_feedback_bytes; }

      secure_vector<uint8_t>& shift_register() { return m_shift_register; }

      secure_vector<uint8_t>& keystream_buf() { return m_keystream_buf; }

   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;
      void key_schedule(const uint8_t key[], size_t length) override;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<uint8_t> m_shift_register;
      secure_vector<uint8_t> m_keystream_buf;
      size_t m_feedback_bytes;
   };

/**
* CFB Encryption
*/
class BOTAN_DLL CFB_Encryption final : public CFB_Mode
   {
   public:
      /**
      * If feedback_bits is zero, cipher->block_size() bytes will be used.
      * @param cipher block cipher to use
      * @param feedback_bits number of bits fed back into the shift register,
      * must be a multiple of 8
      */
      CFB_Encryption(BlockCipher* cipher, size_t feedback_bits) :
         CFB_Mode(cipher, feedback_bits) {}

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   };

/**
* CFB Decryption
*/
class BOTAN_DLL CFB_Decryption final : public CFB_Mode
   {
   public:
      /**
      * If feedback_bits is zero, cipher->block_size() bytes will be used.
      * @param cipher block cipher to use
      * @param feedback_bits number of bits fed back into the shift register,
      * must be a multiple of 8
      */
      CFB_Decryption(BlockCipher* cipher, size_t feedback_bits) :
         CFB_Mode(cipher, feedback_bits) {}

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   };

}


namespace Botan {

/**
* DJB's ChaCha (http://cr.yp.to/chacha.html)
*/
class BOTAN_DLL ChaCha final : public StreamCipher
   {
   public:
      StreamCipher* clone() const override { return new ChaCha(m_rounds); }

      /**
      * @param rounds number of rounds
      * @note Currently only 8, 12 or 20 rounds are supported, all others
      * will throw an exception
      */
      ChaCha(size_t rounds = 20);

      std::string provider() const override;

      void cipher(const uint8_t in[], uint8_t out[], size_t length) override;

      void set_iv(const uint8_t iv[], size_t iv_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return (iv_len == 8 || iv_len == 12); }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(16, 32, 16);
         }

      void clear() override;

      std::string name() const override;

      void seek(uint64_t offset) override;

   private:
      void key_schedule(const uint8_t key[], size_t key_len) override;

      void chacha_x4(uint8_t output[64*4], uint32_t state[16], size_t rounds);

#if defined(BOTAN_HAS_CHACHA_SSE2)
      void chacha_sse2_x4(uint8_t output[64*4], uint32_t state[16], size_t rounds);
#endif

      size_t m_rounds;
      secure_vector<uint32_t> m_state;
      secure_vector<uint8_t> m_buffer;
      size_t m_position = 0;
   };

}


namespace Botan {

/**
* Base class
* See draft-irtf-cfrg-chacha20-poly1305-03 for specification
* If a nonce of 64 bits is used the older version described in
* draft-agl-tls-chacha20poly1305-04 is used instead.
*/
class BOTAN_DLL ChaCha20Poly1305_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const uint8_t ad[], size_t ad_len) override;

      std::string name() const override { return "ChaCha20Poly1305"; }

      size_t update_granularity() const override { return 64; }

      Key_Length_Specification key_spec() const override
         { return Key_Length_Specification(32); }

      bool valid_nonce_length(size_t n) const override;

      size_t tag_size() const override { return 16; }

      void clear() override;

      void reset() override;

   protected:
      std::unique_ptr<StreamCipher> m_chacha;
      std::unique_ptr<MessageAuthenticationCode> m_poly1305;

      ChaCha20Poly1305_Mode();

      secure_vector<uint8_t> m_ad;
      size_t m_nonce_len = 0;
      size_t m_ctext_len = 0;

      bool cfrg_version() const { return m_nonce_len == 12; }
      void update_len(size_t len);
   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;

      void key_schedule(const uint8_t key[], size_t length) override;
   };

/**
* ChaCha20Poly1305 Encryption
*/
class BOTAN_DLL ChaCha20Poly1305_Encryption final : public ChaCha20Poly1305_Mode
   {
   public:
      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   };

/**
* ChaCha20Poly1305 Decryption
*/
class BOTAN_DLL ChaCha20Poly1305_Decryption final : public ChaCha20Poly1305_Mode
   {
   public:
      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length >= tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   };

}


namespace Botan {

/**
* The different charsets (nominally) supported by Botan.
*/
enum Character_Set {
   LOCAL_CHARSET,
   UCS2_CHARSET,
   UTF8_CHARSET,
   LATIN1_CHARSET
};

namespace Charset {

/*
* Character Set Handling
*/
std::string BOTAN_DLL transcode(const std::string& str,
                                Character_Set to,
                                Character_Set from);

bool BOTAN_DLL is_digit(char c);
bool BOTAN_DLL is_space(char c);
bool BOTAN_DLL caseless_cmp(char x, char y);

uint8_t BOTAN_DLL char2digit(char c);
char BOTAN_DLL digit2char(uint8_t b);

}

}


namespace Botan {

/**
* This class represents keyed filters, i.e. filters that have to be
* fed with a key in order to function.
*/
class BOTAN_DLL Keyed_Filter : public Filter
   {
   public:
      /**
      * Set the key of this filter
      * @param key the key to use
      */
      virtual void set_key(const SymmetricKey& key) = 0;

      /**
      * Set the initialization vector of this filter. Note: you should
      * call set_iv() only after you have called set_key()
      * @param iv the initialization vector to use
      */
      virtual void set_iv(const InitializationVector& iv);

      /**
      * Check whether a key length is valid for this filter
      * @param length the key length to be checked for validity
      * @return true if the key length is valid, false otherwise
      */
      bool valid_keylength(size_t length) const
         {
         return key_spec().valid_keylength(length);
         }

      /**
      * @return object describing limits on key size
      */
      virtual Key_Length_Specification key_spec() const = 0;

      /**
      * Check whether an IV length is valid for this filter
      * @param length the IV length to be checked for validity
      * @return true if the IV length is valid, false otherwise
      */
      virtual bool valid_iv_length(size_t length) const
         { return (length == 0); }
   };



/*
* Get a cipher object
*/

/**
* Factory method for general symmetric cipher filters.
* @param algo_spec the name of the desired cipher
* @param key the key to be used for encryption/decryption performed by
* the filter
* @param iv the initialization vector to be used
* @param direction determines whether the filter will be an encrypting
* or decrypting filter
* @return pointer to newly allocated encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   const SymmetricKey& key,
                                   const InitializationVector& iv,
                                   Cipher_Dir direction);

/**
* Factory method for general symmetric cipher filters.
* @param algo_spec the name of the desired cipher
* @param key the key to be used for encryption/decryption performed by
* the filter
* @param direction determines whether the filter will be an encrypting
* or decrypting filter
* @return pointer to the encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   const SymmetricKey& key,
                                   Cipher_Dir direction);

/**
* Factory method for general symmetric cipher filters. No key will be
* set in the filter.
*
* @param algo_spec the name of the desired cipher
* @param direction determines whether the filter will be an encrypting or
* decrypting filter
* @return pointer to the encryption or decryption filter
*/
BOTAN_DLL Keyed_Filter* get_cipher(const std::string& algo_spec,
                                   Cipher_Dir direction);

}


namespace Botan {

/**
* Filter interface for cipher modes
*/
class BOTAN_DLL Cipher_Mode_Filter : public Keyed_Filter,
                                     private Buffered_Filter
   {
   public:
      explicit Cipher_Mode_Filter(Cipher_Mode* t);

      void set_iv(const InitializationVector& iv) override;

      void set_key(const SymmetricKey& key) override;

      Key_Length_Specification key_spec() const override;

      bool valid_iv_length(size_t length) const override;

      std::string name() const override;

   protected:
      const Cipher_Mode& get_mode() const { return *m_mode; }

      Cipher_Mode& get_mode() { return *m_mode; }

   private:
      void write(const uint8_t input[], size_t input_length) override;
      void start_msg() override;
      void end_msg() override;

      void buffered_block(const uint8_t input[], size_t input_length) override;
      void buffered_final(const uint8_t input[], size_t input_length) override;

      class Nonce_State
         {
         public:
            explicit Nonce_State(bool allow_null_nonce) : m_fresh_nonce(allow_null_nonce) {}

            void update(const InitializationVector& iv);
            std::vector<uint8_t> get();
         private:
            bool m_fresh_nonce;
            std::vector<uint8_t> m_nonce;
         };

      Nonce_State m_nonce;
      std::unique_ptr<Cipher_Mode> m_mode;
      secure_vector<uint8_t> m_buffer;
   };

// deprecated aliases, will be removed before 2.0
typedef Cipher_Mode_Filter Transform_Filter;
typedef Transform_Filter Transformation_Filter;

}


namespace Botan {

/**
* CMAC, also known as OMAC1
*/
class BOTAN_DLL CMAC final : public MessageAuthenticationCode
   {
   public:
      std::string name() const override;
      size_t output_length() const override { return m_cipher->block_size(); }
      MessageAuthenticationCode* clone() const override;

      void clear() override;

      Key_Length_Specification key_spec() const override
         {
         return m_cipher->key_spec();
         }

      /**
      * CMAC's polynomial doubling operation
      * @param in the input
      */
      static secure_vector<uint8_t> poly_double(const secure_vector<uint8_t>& in);

      /**
      * @param cipher the block cipher to use
      */
      explicit CMAC(BlockCipher* cipher);

      CMAC(const CMAC&) = delete;
      CMAC& operator=(const CMAC&) = delete;
   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      void key_schedule(const uint8_t[], size_t) override;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<uint8_t> m_buffer, m_state, m_B, m_P;
      size_t m_position;
   };

}


namespace Botan {

/**
* Combines two hash functions using a Feistel scheme. Described in
* "On the Security of Hash Function Combiners", Anja Lehmann
*/
class BOTAN_DLL Comb4P final : public HashFunction
   {
   public:
      /**
      * @param h1 the first hash
      * @param h2 the second hash
      */
      Comb4P(HashFunction* h1, HashFunction* h2);

      size_t hash_block_size() const override;

      size_t output_length() const override
         {
         return m_hash1->output_length() + m_hash2->output_length();
         }

      HashFunction* clone() const override
         {
         return new Comb4P(m_hash1->clone(), m_hash2->clone());
         }

      std::string name() const override
         {
         return "Comb4P(" + m_hash1->name() + "," + m_hash2->name() + ")";
         }

      void clear() override;
   private:
      void add_data(const uint8_t input[], size_t length) override;
      void final_result(uint8_t out[]) override;

      std::unique_ptr<HashFunction> m_hash1, m_hash2;
   };

}


namespace Botan {

#if defined(BOTAN_HAS_COMPRESSION)

class Compression_Algorithm;
class Decompression_Algorithm;

/**
* Filter interface for compression
*/
class BOTAN_DLL Compression_Filter : public Filter
   {
   public:
      void start_msg() override;
      void write(const uint8_t input[], size_t input_length) override;
      void end_msg() override;

      void flush();

      std::string name() const override;

      Compression_Filter(const std::string& type,
                         size_t compression_level,
                         size_t buffer_size = 4096);

      ~Compression_Filter();
   private:
      std::unique_ptr<Compression_Algorithm> m_comp;
      size_t m_buffersize, m_level;
      secure_vector<uint8_t> m_buffer;
   };

/**
* Filter interface for decompression
*/
class BOTAN_DLL Decompression_Filter : public Filter
   {
   public:
      void start_msg() override;
      void write(const uint8_t input[], size_t input_length) override;
      void end_msg() override;

      std::string name() const override;

      Decompression_Filter(const std::string& type,
                           size_t buffer_size = 4096);

      ~Decompression_Filter();
   private:
      std::unique_ptr<Decompression_Algorithm> m_comp;
      std::size_t m_buffersize;
      secure_vector<uint8_t> m_buffer;
   };

#endif

}


namespace Botan {

/**
* A class handling runtime CPU feature detection
*
* Currently this class supports only x86 (via CPUID) and PowerPC (AltiVec detection)
*/
class BOTAN_DLL CPUID
   {
   public:
      /**
      * Probe the CPU and see what extensions are supported
      */
      static void initialize();

      static bool has_simd_32();

      static void print(std::ostream& o);

      /**
      * Return a best guess of the cache line size
      */
      static size_t cache_line_size()
         {
         if(!g_initialized)
            {
            initialize();
            }
         return g_cache_line_size;
         }

      static bool is_little_endian()
         {
         if(!g_initialized)
            {
            initialize();
            }
         return g_little_endian;
         }

      enum CPUID_bits {
#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)
         // This matches the layout of cpuid(1)
         CPUID_RDTSC_BIT = 4,
         CPUID_SSE2_BIT = 26,
         CPUID_CLMUL_BIT = 33,
         CPUID_SSSE3_BIT = 41,
         CPUID_SSE41_BIT = 51,
         CPUID_SSE42_BIT = 52,
         CPUID_AESNI_BIT = 57,
         CPUID_RDRAND_BIT = 62,

         CPUID_AVX2_BIT = 64+5,
         CPUID_BMI2_BIT = 64+8,
         CPUID_AVX512F_BIT = 64+16,
         CPUID_RDSEED_BIT = 64+18,
         CPUID_ADX_BIT = 64+19,
         CPUID_SHA_BIT = 64+29,
#endif

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
         CPUID_ALTIVEC_BIT = 0
#endif

         // TODO: ARMv8 feature detection
      };

#if defined(BOTAN_TARGET_CPU_IS_PPC_FAMILY)
      /**
      * Check if the processor supports AltiVec/VMX
      */
      static bool has_altivec()
         { return has_cpuid_bit(CPUID_ALTIVEC_BIT); }
#endif

#if defined(BOTAN_TARGET_CPU_IS_X86_FAMILY)

      /**
      * Check if the processor supports RDTSC
      */
      static bool has_rdtsc()
         { return has_cpuid_bit(CPUID_RDTSC_BIT); }

      /**
      * Check if the processor supports SSE2
      */
      static bool has_sse2()
         { return has_cpuid_bit(CPUID_SSE2_BIT); }

      /**
      * Check if the processor supports SSSE3
      */
      static bool has_ssse3()
         { return has_cpuid_bit(CPUID_SSSE3_BIT); }

      /**
      * Check if the processor supports SSE4.1
      */
      static bool has_sse41()
         { return has_cpuid_bit(CPUID_SSE41_BIT); }

      /**
      * Check if the processor supports SSE4.2
      */
      static bool has_sse42()
         { return has_cpuid_bit(CPUID_SSE42_BIT); }

      /**
      * Check if the processor supports AVX2
      */
      static bool has_avx2()
         { return has_cpuid_bit(CPUID_AVX2_BIT); }

      /**
      * Check if the processor supports AVX-512F
      */
      static bool has_avx512f()
         { return has_cpuid_bit(CPUID_AVX512F_BIT); }

      /**
      * Check if the processor supports BMI2
      */
      static bool has_bmi2()
         { return has_cpuid_bit(CPUID_BMI2_BIT); }

      /**
      * Check if the processor supports AES-NI
      */
      static bool has_aes_ni()
         { return has_cpuid_bit(CPUID_AESNI_BIT); }

      /**
      * Check if the processor supports CLMUL
      */
      static bool has_clmul()
         { return has_cpuid_bit(CPUID_CLMUL_BIT); }

      /**
      * Check if the processor supports Intel SHA extension
      */
      static bool has_intel_sha()
         { return has_cpuid_bit(CPUID_SHA_BIT); }

      /**
      * Check if the processor supports ADX extension
      */
      static bool has_adx()
         { return has_cpuid_bit(CPUID_ADX_BIT); }

      /**
      * Check if the processor supports RDRAND
      */
      static bool has_rdrand()
         { return has_cpuid_bit(CPUID_RDRAND_BIT); }

      /**
      * Check if the processor supports RDSEED
      */
      static bool has_rdseed()
         { return has_cpuid_bit(CPUID_RDSEED_BIT); }
#endif

      /*
      * Clear a CPUID bit
      * Call CPUID::initialize to reset
      */
      static void clear_cpuid_bit(CPUID_bits bit)
         {
         const uint64_t mask = ~(static_cast<uint64_t>(1) << (bit % 64));
         g_processor_flags[bit/64] &= mask;
         }

      static bool has_cpuid_bit(CPUID_bits elem)
         {
         if(!g_initialized)
            initialize();
         const size_t bit = static_cast<size_t>(elem);
         return ((g_processor_flags[bit/64] >> (bit % 64)) & 1);
         }

   private:
      static bool g_initialized;
      static bool g_little_endian;
      static size_t g_cache_line_size;
      static uint64_t g_processor_flags[2];
   };

}


namespace Botan {

/**
* 24-bit cyclic redundancy check
*/
class BOTAN_DLL CRC24 final : public HashFunction
   {
   public:
      std::string name() const override { return "CRC24"; }
      size_t output_length() const override { return 3; }
      HashFunction* clone() const override { return new CRC24; }

      void clear() override { m_crc = 0xB704CE; }

      CRC24() { clear(); }
      ~CRC24() { clear(); }
   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      uint32_t m_crc;
   };

}


namespace Botan {

/**
* 32-bit cyclic redundancy check
*/
class BOTAN_DLL CRC32 final : public HashFunction
   {
   public:
      std::string name() const override { return "CRC32"; }
      size_t output_length() const override { return 4; }
      HashFunction* clone() const override { return new CRC32; }

      void clear() override { m_crc = 0xFFFFFFFF; }

      CRC32() { clear(); }
      ~CRC32() { clear(); }
   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      uint32_t m_crc;
   };

}


namespace Botan {

class BigInt;

/**
* Interface for a credentials manager.
*
* A type is a fairly static value that represents the general nature
* of the transaction occurring. Currently used values are "tls-client"
* and "tls-server". Context represents a hostname, email address,
* username, or other identifier.
*/
class BOTAN_DLL Credentials_Manager
   {
   public:
      virtual ~Credentials_Manager() {}

      /**
      * Return a list of the certificates of CAs that we trust in this
      * type/context.
      *
      * @param type specifies the type of operation occurring
      *
      * @param context specifies a context relative to type. For instance
      *        for type "tls-client", context specifies the servers name.
      */
      virtual std::vector<Certificate_Store*> trusted_certificate_authorities(
         const std::string& type,
         const std::string& context);

      /**
      * Return a cert chain we can use, ordered from leaf to root,
      * or else an empty vector.
      *
      * It is assumed that the caller can get the private key of the
      * leaf with private_key_for
      *
      * @param cert_key_types specifies the key types desired ("RSA",
      *                       "DSA", "ECDSA", etc), or empty if there
      *                       is no preference by the caller.
      *
      * @param type specifies the type of operation occurring
      *
      * @param context specifies a context relative to type.
      */
      virtual std::vector<X509_Certificate> cert_chain(
         const std::vector<std::string>& cert_key_types,
         const std::string& type,
         const std::string& context);

      /**
      * Return a cert chain we can use, ordered from leaf to root,
      * or else an empty vector.
      *
      * It is assumed that the caller can get the private key of the
      * leaf with private_key_for
      *
      * @param cert_key_type specifies the type of key requested
      *                      ("RSA", "DSA", "ECDSA", etc)
      *
      * @param type specifies the type of operation occurring
      *
      * @param context specifies a context relative to type.
      */
      std::vector<X509_Certificate> cert_chain_single_type(
         const std::string& cert_key_type,
         const std::string& type,
         const std::string& context);

      /**
      * @return private key associated with this certificate if we should
      *         use it with this context. cert was returned by cert_chain
      * @note this object should retain ownership of the returned key;
      *       it should not be deleted by the caller.
      */
      virtual Private_Key* private_key_for(const X509_Certificate& cert,
                                           const std::string& type,
                                           const std::string& context);

      /**
      * @param type specifies the type of operation occurring
      * @param context specifies a context relative to type.
      * @return true if we should attempt SRP authentication
      */
      virtual bool attempt_srp(const std::string& type,
                               const std::string& context);

      /**
      * @param type specifies the type of operation occurring
      * @param context specifies a context relative to type.
      * @return identifier for client-side SRP auth, if available
                for this type/context. Should return empty string
                if password auth not desired/available.
      */
      virtual std::string srp_identifier(const std::string& type,
                                         const std::string& context);

      /**
      * @param type specifies the type of operation occurring
      * @param context specifies a context relative to type.
      * @param identifier specifies what identifier we want the
      *        password for. This will be a value previously returned
      *        by srp_identifier.
      * @return password for client-side SRP auth, if available
                for this identifier/type/context.
      */
      virtual std::string srp_password(const std::string& type,
                                       const std::string& context,
                                       const std::string& identifier);

      /**
      * Retrieve SRP verifier parameters
      */
      virtual bool srp_verifier(const std::string& type,
                                const std::string& context,
                                const std::string& identifier,
                                std::string& group_name,
                                BigInt& verifier,
                                std::vector<uint8_t>& salt,
                                bool generate_fake_on_unknown);

      /**
      * @param type specifies the type of operation occurring
      * @param context specifies a context relative to type.
      * @return the PSK identity hint for this type/context
      */
      virtual std::string psk_identity_hint(const std::string& type,
                                            const std::string& context);

      /**
      * @param type specifies the type of operation occurring
      * @param context specifies a context relative to type.
      * @param identity_hint was passed by the server (but may be empty)
      * @return the PSK identity we want to use
      */
      virtual std::string psk_identity(const std::string& type,
                                       const std::string& context,
                                       const std::string& identity_hint);

      /**
      * @param type specifies the type of operation occurring
      * @param context specifies a context relative to type.
      * @param identity is a PSK identity previously returned by
               psk_identity for the same type and context.
      * @return the PSK used for identity, or throw an exception if no
      * key exists
      */
      virtual SymmetricKey psk(const std::string& type,
                               const std::string& context,
                               const std::string& identity);
   };

}


namespace Botan {

/**
* This namespace holds various high-level crypto functions
*/
namespace CryptoBox {

/**
* Encrypt a message using a passphrase
* @param input the input data
* @param input_len the length of input in bytes
* @param passphrase the passphrase used to encrypt the message
* @param rng a ref to a random number generator, such as AutoSeeded_RNG
*/
BOTAN_DLL std::string encrypt(const uint8_t input[], size_t input_len,
                              const std::string& passphrase,
                              RandomNumberGenerator& rng);


/**
* Decrypt a message encrypted with CryptoBox::encrypt
* @param input the input data
* @param input_len the length of input in bytes
* @param passphrase the passphrase used to encrypt the message
*/
BOTAN_DLL std::string decrypt(const uint8_t input[], size_t input_len,
                              const std::string& passphrase);

/**
* Decrypt a message encrypted with CryptoBox::encrypt
* @param input the input data
* @param passphrase the passphrase used to encrypt the message
*/
BOTAN_DLL std::string decrypt(const std::string& input,
                              const std::string& passphrase);

}

}


namespace Botan {

/**
* CTR-BE (Counter mode, big-endian)
*/
class BOTAN_DLL CTR_BE final : public StreamCipher
   {
   public:
      void cipher(const uint8_t in[], uint8_t out[], size_t length) override;

      void set_iv(const uint8_t iv[], size_t iv_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return (iv_len <= m_cipher->block_size()); }

      Key_Length_Specification key_spec() const override
         {
         return m_cipher->key_spec();
         }

      std::string name() const override;

      CTR_BE* clone() const override
         { return new CTR_BE(m_cipher->clone()); }

      void clear() override;

      /**
      * @param cipher the block cipher to use
      */
      explicit CTR_BE(BlockCipher* cipher);

      CTR_BE(BlockCipher* cipher, size_t ctr_size);

      void seek(uint64_t offset) override;
   private:
      void key_schedule(const uint8_t key[], size_t key_len) override;
      void increment_counter();

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<uint8_t> m_counter, m_pad;
      size_t m_ctr_size;
      size_t m_pad_pos;
   };

}


namespace Botan {

class BOTAN_DLL Curve25519_PublicKey : public virtual Public_Key
   {
   public:
      std::string algo_name() const override { return "Curve25519"; }

      size_t estimated_strength() const override { return 128; }

      size_t key_length() const override { return 255; }

      bool check_key(RandomNumberGenerator& rng, bool strong) const override;

      AlgorithmIdentifier algorithm_identifier() const override;

      std::vector<uint8_t> public_key_bits() const override;

      std::vector<uint8_t> public_value() const { return m_public; }

      /**
      * Create a Curve25519 Public Key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded public key bits
      */
      Curve25519_PublicKey(const AlgorithmIdentifier& alg_id,
                           const std::vector<uint8_t>& key_bits);

      /**
      * Create a Curve25519 Public Key.
      * @param pub 32-byte raw public key
      */
      explicit Curve25519_PublicKey(const std::vector<uint8_t>& pub) : m_public(pub) {}

      /**
      * Create a Curve25519 Public Key.
      * @param pub 32-byte raw public key
      */
      explicit Curve25519_PublicKey(const secure_vector<uint8_t>& pub) :
         m_public(pub.begin(), pub.end()) {}

   protected:
      Curve25519_PublicKey() {}
      std::vector<uint8_t> m_public;
   };

class BOTAN_DLL Curve25519_PrivateKey : public Curve25519_PublicKey,
                                        public virtual Private_Key,
                                        public virtual PK_Key_Agreement_Key
   {
   public:
      /**
      * Construct a private key from the specified parameters.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits PKCS #8 structure
      */
      Curve25519_PrivateKey(const AlgorithmIdentifier& alg_id,
                            const secure_vector<uint8_t>& key_bits);

      /**
      * Generate a private key.
      * @param rng the RNG to use
      */
      explicit Curve25519_PrivateKey(RandomNumberGenerator& rng);

      /**
      * Construct a private key from the specified parameters.
      * @param secret_key DER encoded private key bits
      */
      explicit Curve25519_PrivateKey(const secure_vector<uint8_t>& secret_key);

      std::vector<uint8_t> public_value() const override { return Curve25519_PublicKey::public_value(); }

      secure_vector<uint8_t> agree(const uint8_t w[], size_t w_len) const;

      const secure_vector<uint8_t>& get_x() const { return m_private; }

      secure_vector<uint8_t> private_key_bits() const override;

      bool check_key(RandomNumberGenerator& rng, bool strong) const override;

      std::unique_ptr<PK_Ops::Key_Agreement>
         create_key_agreement_op(RandomNumberGenerator& rng,
                                 const std::string& params,
                                 const std::string& provider) const override;

   private:
      secure_vector<uint8_t> m_private;
   };

/*
* The types above are just wrappers for curve25519_donna, plus defining
* encodings for public and private keys.
*/
void BOTAN_DLL curve25519_donna(uint8_t mypublic[32],
                                const uint8_t secret[32],
                                const uint8_t basepoint[32]);

/**
* Exponentiate by the x25519 base point
* @param mypublic output value
* @param secret random scalar
*/
void BOTAN_DLL curve25519_basepoint(uint8_t mypublic[32],
                                    const uint8_t secret[32]);

}


namespace Botan {

class CurveGFp_Repr
   {
   public:
      virtual ~CurveGFp_Repr() {}

      virtual const BigInt& get_p() const = 0;
      virtual const BigInt& get_a() const = 0;
      virtual const BigInt& get_b() const = 0;

      virtual size_t get_p_words() const = 0;

      /*
      * Returns to_curve_rep(get_a())
      */
      virtual const BigInt& get_a_rep() const = 0;

      /*
      * Returns to_curve_rep(get_b())
      */
      virtual const BigInt& get_b_rep() const = 0;

      virtual void to_curve_rep(BigInt& x, secure_vector<word>& ws) const = 0;

      virtual void from_curve_rep(BigInt& x, secure_vector<word>& ws) const = 0;

      virtual void curve_mul(BigInt& z, const BigInt& x, const BigInt& y,
                             secure_vector<word>& ws) const = 0;

      virtual void curve_sqr(BigInt& z, const BigInt& x,
                             secure_vector<word>& ws) const = 0;
   };

/**
* This class represents an elliptic curve over GF(p)
*/
class BOTAN_DLL CurveGFp
   {
   public:

      /**
      * Create an uninitialized CurveGFp
      */
      CurveGFp() {}

      /**
      * Construct the elliptic curve E: y^2 = x^3 + ax + b over GF(p)
      * @param p prime number of the field
      * @param a first coefficient
      * @param b second coefficient
      */
      CurveGFp(const BigInt& p, const BigInt& a, const BigInt& b) :
         m_repr(choose_repr(p, a, b))
         {
         }

      CurveGFp(const CurveGFp&) = default;

      CurveGFp& operator=(const CurveGFp&) = default;

      /**
      * @return curve coefficient a
      */
      const BigInt& get_a() const { return m_repr->get_a(); }

      /**
      * @return curve coefficient b
      */
      const BigInt& get_b() const { return m_repr->get_b(); }

      /**
      * Get prime modulus of the field of the curve
      * @return prime modulus of the field of the curve
      */
      const BigInt& get_p() const { return m_repr->get_p(); }

      const BigInt& get_a_rep() const { return m_repr->get_a_rep(); }

      const BigInt& get_b_rep() const { return m_repr->get_b_rep(); }

      void to_rep(BigInt& x, secure_vector<word>& ws) const
         {
         m_repr->to_curve_rep(x, ws);
         }

      void from_rep(BigInt& x, secure_vector<word>& ws) const
         {
         m_repr->from_curve_rep(x, ws);
         }

      BigInt from_rep(const BigInt& x, secure_vector<word>& ws) const
         {
         BigInt xt(x);
         m_repr->from_curve_rep(xt, ws);
         return xt;
         }

      // TODO: from_rep taking && ref

      void mul(BigInt& z, const BigInt& x, const BigInt& y, secure_vector<word>& ws) const
         {
         m_repr->curve_mul(z, x, y, ws);
         }

      BigInt mul(const BigInt& x, const BigInt& y, secure_vector<word>& ws) const
         {
         BigInt z;
         m_repr->curve_mul(z, x, y, ws);
         return z;
         }

      void sqr(BigInt& z, const BigInt& x, secure_vector<word>& ws) const
         {
         m_repr->curve_sqr(z, x, ws);
         }

      BigInt sqr(const BigInt& x, secure_vector<word>& ws) const
         {
         BigInt z;
         m_repr->curve_sqr(z, x, ws);
         return z;
         }

      void swap(CurveGFp& other)
         {
         std::swap(m_repr, other.m_repr);
         }

   private:
      static std::shared_ptr<CurveGFp_Repr>
         choose_repr(const BigInt& p, const BigInt& a, const BigInt& b);

      std::shared_ptr<CurveGFp_Repr> m_repr;
   };

/**
* Equality operator
* @param lhs a curve
* @param rhs a curve
* @return true iff lhs is the same as rhs
*/
inline bool operator==(const CurveGFp& lhs, const CurveGFp& rhs)
   {
   return (lhs.get_p() == rhs.get_p()) &&
          (lhs.get_a() == rhs.get_a()) &&
          (lhs.get_b() == rhs.get_b());
   }

inline bool operator!=(const CurveGFp& lhs, const CurveGFp& rhs)
   {
   return !(lhs == rhs);
   }

}

namespace std {

template<> inline
void swap<Botan::CurveGFp>(Botan::CurveGFp& curve1,
                           Botan::CurveGFp& curve2) BOTAN_NOEXCEPT
   {
   curve1.swap(curve2);
   }

} // namespace std


namespace Botan {

/**
* NIST Prime reduction functions.
*
* Reduces the value in place
*
* ws is a workspace function which is used as a temporary,
* and will be resized as needed.
*/
BOTAN_DLL const BigInt& prime_p521();
BOTAN_DLL void redc_p521(BigInt& x, secure_vector<word>& ws);

#if (BOTAN_MP_WORD_BITS == 32) || (BOTAN_MP_WORD_BITS == 64)

#define BOTAN_HAS_NIST_PRIME_REDUCERS_W32

BOTAN_DLL const BigInt& prime_p384();
BOTAN_DLL void redc_p384(BigInt& x, secure_vector<word>& ws);

BOTAN_DLL const BigInt& prime_p256();
BOTAN_DLL void redc_p256(BigInt& x, secure_vector<word>& ws);

BOTAN_DLL const BigInt& prime_p224();
BOTAN_DLL void redc_p224(BigInt& x, secure_vector<word>& ws);

BOTAN_DLL const BigInt& prime_p192();
BOTAN_DLL void redc_p192(BigInt& x, secure_vector<word>& ws);

#endif

}


namespace Botan {

/**
* This class represents abstract data sink objects.
*/
class BOTAN_DLL DataSink : public Filter
   {
   public:
      bool attachable() override { return false; }
      DataSink() {}
      virtual ~DataSink() {}

      DataSink& operator=(const DataSink&) = delete;
      DataSink(const DataSink&) = delete;
   };

/**
* This class represents a data sink which writes its output to a stream.
*/
class BOTAN_DLL DataSink_Stream : public DataSink
   {
   public:
      /**
      * Construct a DataSink_Stream from a stream.
      * @param stream the stream to write to
      * @param name identifier
      */
      DataSink_Stream(std::ostream& stream,
                      const std::string& name = "<std::ostream>");

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)

      /**
      * Construct a DataSink_Stream from a filesystem path name.
      * @param pathname the name of the file to open a stream to
      * @param use_binary indicates whether to treat the file
      * as a binary file or not
      */
      DataSink_Stream(const std::string& pathname,
                      bool use_binary = false);
#endif

      std::string name() const override { return m_identifier; }

      void write(const uint8_t[], size_t) override;

      ~DataSink_Stream();

   private:
      const std::string m_identifier;

      // May be null, if m_sink was an external reference
      std::unique_ptr<std::ostream> m_sink_memory;
      std::ostream& m_sink;
   };

}


namespace Botan {

class BigInt;
class ASN1_Object;

/**
* General DER Encoding Object
*/
class BOTAN_DLL DER_Encoder
   {
   public:
      secure_vector<uint8_t> get_contents();

      std::vector<uint8_t> get_contents_unlocked()
         { return unlock(get_contents()); }

      DER_Encoder& start_cons(ASN1_Tag type_tag,
                              ASN1_Tag class_tag = UNIVERSAL);
      DER_Encoder& end_cons();

      DER_Encoder& start_explicit(uint16_t type_tag);
      DER_Encoder& end_explicit();

      DER_Encoder& raw_bytes(const uint8_t val[], size_t len);
      DER_Encoder& raw_bytes(const secure_vector<uint8_t>& val);
      DER_Encoder& raw_bytes(const std::vector<uint8_t>& val);

      DER_Encoder& encode_null();
      DER_Encoder& encode(bool b);
      DER_Encoder& encode(size_t s);
      DER_Encoder& encode(const BigInt& n);
      DER_Encoder& encode(const secure_vector<uint8_t>& v, ASN1_Tag real_type);
      DER_Encoder& encode(const std::vector<uint8_t>& v, ASN1_Tag real_type);
      DER_Encoder& encode(const uint8_t val[], size_t len, ASN1_Tag real_type);

      DER_Encoder& encode(bool b,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(size_t s,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const BigInt& n,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const std::vector<uint8_t>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const secure_vector<uint8_t>& v,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      DER_Encoder& encode(const uint8_t v[], size_t len,
                          ASN1_Tag real_type,
                          ASN1_Tag type_tag,
                          ASN1_Tag class_tag = CONTEXT_SPECIFIC);

      template<typename T>
      DER_Encoder& encode_optional(const T& value, const T& default_value)
         {
         if(value != default_value)
            encode(value);
         return (*this);
         }

      template<typename T>
      DER_Encoder& encode_list(const std::vector<T>& values)
         {
         for(size_t i = 0; i != values.size(); ++i)
            encode(values[i]);
         return (*this);
         }

      DER_Encoder& encode(const ASN1_Object& obj);
      DER_Encoder& encode_if(bool pred, DER_Encoder& enc);
      DER_Encoder& encode_if(bool pred, const ASN1_Object& obj);

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const uint8_t rep[], size_t length);

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const std::vector<uint8_t>& rep)
         {
         return add_object(type_tag, class_tag, rep.data(), rep.size());
         }

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const secure_vector<uint8_t>& rep)
         {
         return add_object(type_tag, class_tag, rep.data(), rep.size());
         }

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              const std::string& str);

      DER_Encoder& add_object(ASN1_Tag type_tag, ASN1_Tag class_tag,
                              uint8_t val);

   private:
      class DER_Sequence
         {
         public:
            ASN1_Tag tag_of() const;
            secure_vector<uint8_t> get_contents();
            void add_bytes(const uint8_t[], size_t);
            DER_Sequence(ASN1_Tag, ASN1_Tag);
         private:
            ASN1_Tag m_type_tag, m_class_tag;
            secure_vector<uint8_t> m_contents;
            std::vector< secure_vector<uint8_t> > m_set_contents;
         };

      secure_vector<uint8_t> m_contents;
      std::vector<DER_Sequence> m_subsequences;
   };

}


namespace Botan {

/**
* DES
*/
class BOTAN_DLL DES final : public Block_Cipher_Fixed_Params<8, 8>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "DES"; }
      BlockCipher* clone() const override { return new DES; }
   private:
      void key_schedule(const uint8_t[], size_t) override;

      secure_vector<uint32_t> m_round_key;
   };

/**
* Triple DES
*/
class BOTAN_DLL TripleDES final : public Block_Cipher_Fixed_Params<8, 16, 24, 8>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "TripleDES"; }
      BlockCipher* clone() const override { return new TripleDES; }
   private:
      void key_schedule(const uint8_t[], size_t) override;

      secure_vector<uint32_t> m_round_key;
   };

/*
* DES Tables
*/
extern const uint32_t DES_SPBOX1[256];
extern const uint32_t DES_SPBOX2[256];
extern const uint32_t DES_SPBOX3[256];
extern const uint32_t DES_SPBOX4[256];
extern const uint32_t DES_SPBOX5[256];
extern const uint32_t DES_SPBOX6[256];
extern const uint32_t DES_SPBOX7[256];
extern const uint32_t DES_SPBOX8[256];

extern const uint64_t DES_IPTAB1[256];
extern const uint64_t DES_IPTAB2[256];
extern const uint64_t DES_FPTAB1[256];
extern const uint64_t DES_FPTAB2[256];

}


namespace Botan {

/**
* DESX
*/
class BOTAN_DLL DESX final : public Block_Cipher_Fixed_Params<8, 24>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "DESX"; }
      BlockCipher* clone() const override { return new DESX; }
   private:
      void key_schedule(const uint8_t[], size_t) override;
      secure_vector<uint8_t> m_K1, m_K2;
      DES m_des;
   };

}


namespace Botan {

/**
* This class represents discrete logarithm groups. It holds a prime p,
* a prime q = (p-1)/2 and g = x^((p-1)/q) mod p.
*/
class BOTAN_DLL DL_Group
   {
   public:

      /**
      * Get the prime p.
      * @return prime p
      */
      const BigInt& get_p() const;

      /**
      * Get the prime q.
      * @return prime q
      */
      const BigInt& get_q() const;

      /**
      * Get the base g.
      * @return base g
      */
      const BigInt& get_g() const;

      /**
      * The DL group encoding format variants.
      */
      enum Format {
         ANSI_X9_42,
         ANSI_X9_57,
         PKCS_3,

         DSA_PARAMETERS = ANSI_X9_57,
         DH_PARAMETERS = ANSI_X9_42,
         ANSI_X9_42_DH_PARAMETERS = ANSI_X9_42,
         PKCS3_DH_PARAMETERS = PKCS_3
      };

      /**
      * Determine the prime creation for DL groups.
      */
      enum PrimeType { Strong, Prime_Subgroup, DSA_Kosherizer };

      /**
      * Perform validity checks on the group.
      * @param rng the rng to use
      * @param strong whether to perform stronger by lengthier tests
      * @return true if the object is consistent, false otherwise
      */
      bool verify_group(RandomNumberGenerator& rng, bool strong) const;

      /**
      * Encode this group into a string using PEM encoding.
      * @param format the encoding format
      * @return string holding the PEM encoded group
      */
      std::string PEM_encode(Format format) const;

      /**
      * Encode this group into a string using DER encoding.
      * @param format the encoding format
      * @return string holding the DER encoded group
      */
      std::vector<uint8_t> DER_encode(Format format) const;

      /**
      * Decode a DER/BER encoded group into this instance.
      * @param ber a vector containing the DER/BER encoded group
      * @param format the format of the encoded group
      */
      void BER_decode(const std::vector<uint8_t>& ber,
                      Format format);

      /**
      * Decode a PEM encoded group into this instance.
      * @param pem the PEM encoding of the group
      */
      void PEM_decode(const std::string& pem);

      /**
      * Construct a DL group with uninitialized internal value.
      * Use this constructor is you wish to set the groups values
      * from a DER or PEM encoded group.
      */
      DL_Group();

      /**
      * Construct a DL group that is registered in the configuration.
      * @param name the name that is configured in the global configuration
      * for the desired group. If no configuration file is specified,
      * the default values from the file policy.cpp will be used. For instance,
      * use "modp/ietf/3072".
      */
      DL_Group(const std::string& name);

      /**
      * Create a new group randomly.
      * @param rng the random number generator to use
      * @param type specifies how the creation of primes p and q shall
      * be performed. If type=Strong, then p will be determined as a
      * safe prime, and q will be chosen as (p-1)/2. If
      * type=Prime_Subgroup and qbits = 0, then the size of q will be
      * determined according to the estimated difficulty of the DL
      * problem. If type=DSA_Kosherizer, DSA primes will be created.
      * @param pbits the number of bits of p
      * @param qbits the number of bits of q. Leave it as 0 to have
      * the value determined according to pbits.
      */
      DL_Group(RandomNumberGenerator& rng, PrimeType type,
               size_t pbits, size_t qbits = 0);

      /**
      * Create a DSA group with a given seed.
      * @param rng the random number generator to use
      * @param seed the seed to use to create the random primes
      * @param pbits the desired bit size of the prime p
      * @param qbits the desired bit size of the prime q.
      */
      DL_Group(RandomNumberGenerator& rng,
               const std::vector<uint8_t>& seed,
               size_t pbits = 1024, size_t qbits = 0);

      /**
      * Create a DL group. The prime q will be determined according to p.
      * @param p the prime p
      * @param g the base g
      */
      DL_Group(const BigInt& p, const BigInt& g);

      /**
      * Create a DL group.
      * @param p the prime p
      * @param q the prime q
      * @param g the base g
      */
      DL_Group(const BigInt& p, const BigInt& q, const BigInt& g);

      /**
      * Return PEM representation of named DL group
      */
      static std::string PEM_for_named_group(const std::string& name);
   private:
      static BigInt make_dsa_generator(const BigInt&, const BigInt&);

      void init_check() const;
      void initialize(const BigInt&, const BigInt&, const BigInt&);
      bool m_initialized;
      BigInt m_p, m_q, m_g;
   };

}


namespace Botan {

/**
* This class represents discrete logarithm (DL) public keys.
*/
class BOTAN_DLL DL_Scheme_PublicKey : public virtual Public_Key
   {
   public:
      bool check_key(RandomNumberGenerator& rng, bool) const override;

      AlgorithmIdentifier algorithm_identifier() const override;

      std::vector<uint8_t> public_key_bits() const override;

      /**
      * Get the DL domain parameters of this key.
      * @return DL domain parameters of this key
      */
      const DL_Group& get_domain() const { return m_group; }

      /**
      * Get the public value y with y = g^x mod p where x is the secret key.
      */
      const BigInt& get_y() const { return m_y; }

      /**
      * Get the prime p of the underlying DL group.
      * @return prime p
      */
      const BigInt& group_p() const { return m_group.get_p(); }

      /**
      * Get the prime q of the underlying DL group.
      * @return prime q
      */
      const BigInt& group_q() const { return m_group.get_q(); }

      /**
      * Get the generator g of the underlying DL group.
      * @return generator g
      */
      const BigInt& group_g() const { return m_group.get_g(); }

      /**
      * Get the underlying groups encoding format.
      * @return encoding format
      */
      virtual DL_Group::Format group_format() const = 0;

      size_t key_length() const override;
      size_t estimated_strength() const override;

      /**
      * Create a public key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded public key bits
      * @param group_format the underlying groups encoding format
      */
      DL_Scheme_PublicKey(const AlgorithmIdentifier& alg_id,
                          const std::vector<uint8_t>& key_bits,
                          DL_Group::Format group_format);

   protected:
      DL_Scheme_PublicKey() {}

      /**
      * The DL public key
      */
      BigInt m_y;

      /**
      * The DL group
      */
      DL_Group m_group;
   };

/**
* This class represents discrete logarithm (DL) private keys.
*/
class BOTAN_DLL DL_Scheme_PrivateKey : public virtual DL_Scheme_PublicKey,
                                       public virtual Private_Key
   {
   public:
      bool check_key(RandomNumberGenerator& rng, bool) const override;

      /**
      * Get the secret key x.
      * @return secret key
      */
      const BigInt& get_x() const { return m_x; }

      secure_vector<uint8_t> private_key_bits() const override;

      /**
      * Create a private key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded private key bits
      * @param group_format the underlying groups encoding format
      */
      DL_Scheme_PrivateKey(const AlgorithmIdentifier& alg_id,
                           const secure_vector<uint8_t>& key_bits,
                           DL_Group::Format group_format);

   protected:
      DL_Scheme_PrivateKey() {}

      /**
      * The DL private key
      */
      BigInt m_x;
   };

}


namespace Botan {

/**
* This class represents Diffie-Hellman public keys.
*/
class BOTAN_DLL DH_PublicKey : public virtual DL_Scheme_PublicKey
   {
   public:
      std::string algo_name() const override { return "DH"; }

      std::vector<uint8_t> public_value() const;

      DL_Group::Format group_format() const override { return DL_Group::ANSI_X9_42; }

      /**
      * Create a public key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded public key bits
      */
      DH_PublicKey(const AlgorithmIdentifier& alg_id,
                   const std::vector<uint8_t>& key_bits) :
         DL_Scheme_PublicKey(alg_id, key_bits, DL_Group::ANSI_X9_42) {}

      /**
      * Construct a public key with the specified parameters.
      * @param grp the DL group to use in the key
      * @param y the public value y
      */
      DH_PublicKey(const DL_Group& grp, const BigInt& y);
   protected:
      DH_PublicKey() {}
   };

/**
* This class represents Diffie-Hellman private keys.
*/
class BOTAN_DLL DH_PrivateKey : public DH_PublicKey,
                                public PK_Key_Agreement_Key,
                                public virtual DL_Scheme_PrivateKey
   {
   public:
      std::vector<uint8_t> public_value() const override;

      /**
      * Load a private key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits PKCS #8 structure
      */
      DH_PrivateKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<uint8_t>& key_bits);

      /**
      * Create a private key.
      * @param rng random number generator to use
      * @param grp the group to be used in the key
      * @param x the key's secret value (or if zero, generate a new key)
      */
      DH_PrivateKey(RandomNumberGenerator& rng, const DL_Group& grp,
                    const BigInt& x = 0);

      std::unique_ptr<PK_Ops::Key_Agreement>
         create_key_agreement_op(RandomNumberGenerator& rng,
                                 const std::string& params,
                                 const std::string& provider) const override;
   };

}


namespace Botan {

/**
* BigInt Division
* @param x an integer
* @param y a non-zero integer
* @param q will be set to x / y
* @param r will be set to x % y
*/
void BOTAN_DLL divide(const BigInt& x,
                      const BigInt& y,
                      BigInt& q,
                      BigInt& r);

}


namespace Botan {

/**
* Encoding Method for Encryption
*/
class BOTAN_DLL EME
   {
   public:
      virtual ~EME() = default;

      /**
      * Return the maximum input size in bytes we can support
      * @param keybits the size of the key in bits
      * @return upper bound of input in bytes
      */
      virtual size_t maximum_input_size(size_t keybits) const = 0;

      /**
      * Encode an input
      * @param in the plaintext
      * @param in_length length of plaintext in bytes
      * @param key_length length of the key in bits
      * @param rng a random number generator
      * @return encoded plaintext
      */
      secure_vector<uint8_t> encode(const uint8_t in[],
                                 size_t in_length,
                                 size_t key_length,
                                 RandomNumberGenerator& rng) const;

      /**
      * Encode an input
      * @param in the plaintext
      * @param key_length length of the key in bits
      * @param rng a random number generator
      * @return encoded plaintext
      */
      secure_vector<uint8_t> encode(const secure_vector<uint8_t>& in,
                                 size_t key_length,
                                 RandomNumberGenerator& rng) const;

      /**
      * Decode an input
      * @param valid_mask written to specifies if output is valid
      * @param in the encoded plaintext
      * @param in_len length of encoded plaintext in bytes
      * @return bytes of out[] written to along with
      *         validity mask (0xFF if valid, else 0x00)
      */
      virtual secure_vector<uint8_t> unpad(uint8_t& valid_mask,
                                        const uint8_t in[],
                                        size_t in_len) const = 0;

      /**
      * Encode an input
      * @param in the plaintext
      * @param in_length length of plaintext in bytes
      * @param key_length length of the key in bits
      * @param rng a random number generator
      * @return encoded plaintext
      */
      virtual secure_vector<uint8_t> pad(const uint8_t in[],
                                      size_t in_length,
                                      size_t key_length,
                                      RandomNumberGenerator& rng) const = 0;
   };

/**
* Factory method for EME (message-encoding methods for encryption) objects
* @param algo_spec the name of the EME to create
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL EME*  get_eme(const std::string& algo_spec);

}


namespace Botan {

/**
* EMSA, from IEEE 1363s Encoding Method for Signatures, Appendix
*
* Any way of encoding/padding signatures
*/
class BOTAN_DLL EMSA
   {
   public:
      /**
      * Add more data to the signature computation
      * @param input some data
      * @param length length of input in bytes
      */
      virtual void update(const uint8_t input[], size_t length) = 0;

      /**
      * @return raw hash
      */
      virtual secure_vector<uint8_t> raw_data() = 0;

      /**
      * Return the encoding of a message
      * @param msg the result of raw_data()
      * @param output_bits the desired output bit size
      * @param rng a random number generator
      * @return encoded signature
      */
      virtual secure_vector<uint8_t> encoding_of(const secure_vector<uint8_t>& msg,
                                             size_t output_bits,
                                             RandomNumberGenerator& rng) = 0;

      /**
      * Verify the encoding
      * @param coded the received (coded) message representative
      * @param raw the computed (local, uncoded) message representative
      * @param key_bits the size of the key in bits
      * @return true if coded is a valid encoding of raw, otherwise false
      */
      virtual bool verify(const secure_vector<uint8_t>& coded,
                          const secure_vector<uint8_t>& raw,
                          size_t key_bits) = 0;

      virtual ~EMSA();

      /**
      * @return a new object representing the same encoding method as *this
      */
      virtual EMSA* clone() = 0;
   };

/**
* Factory method for EMSA (message-encoding methods for signatures
* with appendix) objects
* @param algo_spec the name of the EMSA to create
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL EMSA* get_emsa(const std::string& algo_spec);

/**
* Returns the hash function used in the given EMSA scheme
* If the hash function is not specified or not understood,
* returns "SHA-512"
* @param algo_spec the name of the EMSA
* @return hash function used in the given EMSA scheme
*/
BOTAN_DLL std::string hash_for_emsa(const std::string& algo_spec);

}


namespace Botan {

/**
* Key Derivation Function
*/
class BOTAN_DLL KDF
   {
   public:
      virtual ~KDF() {}

      /**
      * Create an instance based on a name
      * If provider is empty then best available is chosen.
      * @param algo_spec algorithm name
      * @param provider provider implementation to choose
      * @return a null pointer if the algo/provider combination cannot be found
      */
      static std::unique_ptr<KDF>
         create(const std::string& algo_spec,
                const std::string& provider = "");

      /**
      * Create an instance based on a name, or throw if the
      * algo/provider combination cannot be found. If provider is
      * empty then best available is chosen.
      */
      static std::unique_ptr<KDF>
         create_or_throw(const std::string& algo_spec,
                         const std::string& provider = "");

      /**
      * @return list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      /**
      * @return KDF name
      */
      virtual std::string name() const = 0;

      /**
      * Derive a key
      * @param key buffer holding the derived key, must be of length key_len
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param secret_len size of secret in bytes
      * @param salt a diversifier
      * @param salt_len size of salt in bytes
      * @param label purpose for the derived keying material
      * @param label_len size of label in bytes
      * @return the derived key
      */
      virtual size_t kdf(uint8_t key[], size_t key_len,
                         const uint8_t secret[], size_t secret_len,
                         const uint8_t salt[], size_t salt_len,
                         const uint8_t label[], size_t label_len) const = 0;

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param secret_len size of secret in bytes
      * @param salt a diversifier
      * @param salt_len size of salt in bytes
      * @param label purpose for the derived keying material
      * @param label_len size of label in bytes
      * @return the derived key
      */
      secure_vector<uint8_t> derive_key(size_t key_len,
                                    const uint8_t secret[],
                                    size_t secret_len,
                                    const uint8_t salt[],
                                    size_t salt_len,
                                    const uint8_t label[] = nullptr,
                                    size_t label_len = 0) const
         {
         secure_vector<uint8_t> key(key_len);
         key.resize(kdf(key.data(), key.size(), secret, secret_len, salt, salt_len, label, label_len));
         return key;
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      * @param label purpose for the derived keying material
      * @return the derived key
      */
      secure_vector<uint8_t> derive_key(size_t key_len,
                                    const secure_vector<uint8_t>& secret,
                                    const std::string& salt = "",
                                    const std::string& label = "") const
         {
         return derive_key(key_len, secret.data(), secret.size(),
                           reinterpret_cast<const uint8_t*>(salt.data()),
                           salt.length(),
                           reinterpret_cast<const uint8_t*>(label.data()),
                           label.length());

         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      * @param label purpose for the derived keying material
      * @return the derived key
      */
      template<typename Alloc, typename Alloc2, typename Alloc3>
      secure_vector<uint8_t> derive_key(size_t key_len,
                                     const std::vector<uint8_t, Alloc>& secret,
                                     const std::vector<uint8_t, Alloc2>& salt,
                                     const std::vector<uint8_t, Alloc3>& label) const
         {
         return derive_key(key_len,
                           secret.data(), secret.size(),
                           salt.data(), salt.size(),
                           label.data(), label.size());
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param salt a diversifier
      * @param salt_len size of salt in bytes
      * @param label purpose for the derived keying material
      * @return the derived key
      */
      secure_vector<uint8_t> derive_key(size_t key_len,
                                    const secure_vector<uint8_t>& secret,
                                    const uint8_t salt[],
                                    size_t salt_len,
                                    const std::string& label = "") const
         {
         return derive_key(key_len,
                           secret.data(), secret.size(),
                           salt, salt_len,
                           reinterpret_cast<const uint8_t*>(label.data()),
                           label.size());
         }

      /**
      * Derive a key
      * @param key_len the desired output length in bytes
      * @param secret the secret input
      * @param secret_len size of secret in bytes
      * @param salt a diversifier
      * @param label purpose for the derived keying material
      * @return the derived key
      */
      secure_vector<uint8_t> derive_key(size_t key_len,
                                    const uint8_t secret[],
                                    size_t secret_len,
                                    const std::string& salt = "",
                                    const std::string& label = "") const
         {
         return derive_key(key_len, secret, secret_len,
                           reinterpret_cast<const uint8_t*>(salt.data()),
                           salt.length(),
                           reinterpret_cast<const uint8_t*>(label.data()),
                           label.length());
         }

      /**
      * @return new object representing the same algorithm as *this
      */
      virtual KDF* clone() const = 0;
   };

/**
* Factory method for KDF (key derivation function)
* @param algo_spec the name of the KDF to create
* @return pointer to newly allocated object of that type
*/
BOTAN_DLL KDF* get_kdf(const std::string& algo_spec);

}


#if defined(BOTAN_HAS_SYSTEM_RNG)

namespace Botan {

/**
* Return a shared reference to a global PRNG instance provided by the
* operating system. For instance might be instantiated by /dev/urandom
* or CryptGenRandom.
*/
BOTAN_DLL RandomNumberGenerator& system_rng();

/*
* Instantiable reference to the system RNG.
*/
class BOTAN_DLL System_RNG final : public RandomNumberGenerator
   {
   public:
      std::string name() const override { return system_rng().name(); }

      void randomize(uint8_t out[], size_t len) override { system_rng().randomize(out, len); }

      void add_entropy(const uint8_t in[], size_t length) override { system_rng().add_entropy(in, length); }

      bool is_seeded() const override { return true; }

      void clear() override {}
   };

}

  #define BOTAN_PUBKEY_INCLUDE_DEPRECATED_CONSTRUCTORS
#endif

namespace Botan {

/**
* The two types of signature format supported by Botan.
*/
enum Signature_Format { IEEE_1363, DER_SEQUENCE };

/**
* Public Key Encryptor
* This is the primary interface for public key encryption
*/
class BOTAN_DLL PK_Encryptor
   {
   public:

      /**
      * Encrypt a message.
      * @param in the message as a byte array
      * @param length the length of the above byte array
      * @param rng the random number source to use
      * @return encrypted message
      */
      std::vector<uint8_t> encrypt(const uint8_t in[], size_t length,
                                 RandomNumberGenerator& rng) const
         {
         return enc(in, length, rng);
         }

      /**
      * Encrypt a message.
      * @param in the message
      * @param rng the random number source to use
      * @return encrypted message
      */
      template<typename Alloc>
      std::vector<uint8_t> encrypt(const std::vector<uint8_t, Alloc>& in,
                                RandomNumberGenerator& rng) const
         {
         return enc(in.data(), in.size(), rng);
         }

      /**
      * Return the maximum allowed message size in bytes.
      * @return maximum message size in bytes
      */
      virtual size_t maximum_input_size() const = 0;

      PK_Encryptor() {}
      virtual ~PK_Encryptor() {}

      PK_Encryptor(const PK_Encryptor&) = delete;
      PK_Encryptor& operator=(const PK_Encryptor&) = delete;

   private:
      virtual std::vector<uint8_t> enc(const uint8_t[], size_t,
                                    RandomNumberGenerator&) const = 0;
   };

/**
* Public Key Decryptor
*/
class BOTAN_DLL PK_Decryptor
   {
   public:
      /**
      * Decrypt a ciphertext, throwing an exception if the input
      * seems to be invalid (eg due to an accidental or malicious
      * error in the ciphertext).
      *
      * @param in the ciphertext as a byte array
      * @param length the length of the above byte array
      * @return decrypted message
      */
      secure_vector<uint8_t> decrypt(const uint8_t in[], size_t length) const;

      /**
      * Same as above, but taking a vector
      * @param in the ciphertext
      * @return decrypted message
      */
      template<typename Alloc>
      secure_vector<uint8_t> decrypt(const std::vector<uint8_t, Alloc>& in) const
         {
         return decrypt(in.data(), in.size());
         }

      /**
      * Decrypt a ciphertext. If the ciphertext is invalid (eg due to
      * invalid padding) or is not the expected length, instead
      * returns a random string of the expected length. Use to avoid
      * oracle attacks, especially against PKCS #1 v1.5 decryption.
      */
      secure_vector<uint8_t>
      decrypt_or_random(const uint8_t in[],
                        size_t length,
                        size_t expected_pt_len,
                        RandomNumberGenerator& rng) const;

      /**
      * Decrypt a ciphertext. If the ciphertext is invalid (eg due to
      * invalid padding) or is not the expected length, instead
      * returns a random string of the expected length. Use to avoid
      * oracle attacks, especially against PKCS #1 v1.5 decryption.
      *
      * Additionally checks (also in const time) that:
      *    contents[required_content_offsets[i]] == required_content_bytes[i]
      * for 0 <= i < required_contents
      *
      * Used for example in TLS, which encodes the client version in
      * the content bytes: if there is any timing variation the version
      * check can be used as an oracle to recover the key.
      */
      secure_vector<uint8_t>
      decrypt_or_random(const uint8_t in[],
                        size_t length,
                        size_t expected_pt_len,
                        RandomNumberGenerator& rng,
                        const uint8_t required_content_bytes[],
                        const uint8_t required_content_offsets[],
                        size_t required_contents) const;

      PK_Decryptor() {}
      virtual ~PK_Decryptor() = default;

      PK_Decryptor(const PK_Decryptor&) = delete;
      PK_Decryptor& operator=(const PK_Decryptor&) = delete;

   private:
      virtual secure_vector<uint8_t> do_decrypt(uint8_t& valid_mask,
                                             const uint8_t in[], size_t in_len) const = 0;
   };

/**
* Public Key Signer. Use the sign_message() functions for small
* messages. Use multiple calls update() to process large messages and
* generate the signature by finally calling signature().
*/
class BOTAN_DLL PK_Signer final
   {
   public:

      /**
      * Construct a PK Signer.
      * @param key the key to use inside this signer
      * @param rng the random generator to use
      * @param emsa the EMSA to use
      * An example would be "EMSA1(SHA-224)".
      * @param format the signature format to use
      * @param provider the provider to use
      */
      PK_Signer(const Private_Key& key,
                RandomNumberGenerator& rng,
                const std::string& emsa,
                Signature_Format format = IEEE_1363,
                const std::string& provider = "");

#if defined(BOTAN_PUBKEY_INCLUDE_DEPRECATED_CONSTRUCTORS)
      /**
      * Construct a PK Signer.
      * @param key the key to use inside this signer
      * @param emsa the EMSA to use
      * An example would be "EMSA1(SHA-224)".
      * @param format the signature format to use
      */
      BOTAN_DEPRECATED("Use constructor taking a RNG object")
      PK_Signer(const Private_Key& key,
                const std::string& emsa,
                Signature_Format format = IEEE_1363,
                const std::string& provider = "") :
         PK_Signer(key, system_rng(), emsa, format, provider)
         {}
#endif

      ~PK_Signer();

      PK_Signer(const PK_Signer&) = delete;
      PK_Signer& operator=(const PK_Signer&) = delete;

      /**
      * Sign a message all in one go
      * @param in the message to sign as a byte array
      * @param length the length of the above byte array
      * @param rng the rng to use
      * @return signature
      */
      std::vector<uint8_t> sign_message(const uint8_t in[], size_t length,
                                     RandomNumberGenerator& rng)
         {
         this->update(in, length);
         return this->signature(rng);
         }

      /**
      * Sign a message.
      * @param in the message to sign
      * @param rng the rng to use
      * @return signature
      */
      std::vector<uint8_t> sign_message(const std::vector<uint8_t>& in,
                                     RandomNumberGenerator& rng)
         { return sign_message(in.data(), in.size(), rng); }

      /**
      * Sign a message.
      * @param in the message to sign
      * @param rng the rng to use
      * @return signature
      */
      std::vector<uint8_t> sign_message(const secure_vector<uint8_t>& in,
                                     RandomNumberGenerator& rng)
         { return sign_message(in.data(), in.size(), rng); }

      /**
      * Add a message part (single byte).
      * @param in the byte to add
      */
      void update(uint8_t in) { update(&in, 1); }

      /**
      * Add a message part.
      * @param in the message part to add as a byte array
      * @param length the length of the above byte array
      */
      void update(const uint8_t in[], size_t length);

      /**
      * Add a message part.
      * @param in the message part to add
      */
      void update(const std::vector<uint8_t>& in) { update(in.data(), in.size()); }

      /**
      * Add a message part.
      * @param in the message part to add
      */
      void update(const std::string& in)
         {
         update(reinterpret_cast<const uint8_t*>(in.data()), in.size());
         }

      /**
      * Get the signature of the so far processed message (provided by the
      * calls to update()).
      * @param rng the rng to use
      * @return signature of the total message
      */
      std::vector<uint8_t> signature(RandomNumberGenerator& rng);

      /**
      * Set the output format of the signature.
      * @param format the signature format to use
      */
      void set_output_format(Signature_Format format) { m_sig_format = format; }
   private:
      std::unique_ptr<PK_Ops::Signature> m_op;
      Signature_Format m_sig_format;
      size_t m_parts, m_part_size;
   };

/**
* Public Key Verifier. Use the verify_message() functions for small
* messages. Use multiple calls update() to process large messages and
* verify the signature by finally calling check_signature().
*/
class BOTAN_DLL PK_Verifier final
   {
   public:
      /**
      * Construct a PK Verifier.
      * @param pub_key the public key to verify against
      * @param emsa the EMSA to use (eg "EMSA3(SHA-1)")
      * @param format the signature format to use
      * @param provider the provider to use
      */
      PK_Verifier(const Public_Key& pub_key,
                  const std::string& emsa,
                  Signature_Format format = IEEE_1363,
                  const std::string& provider = "");

      ~PK_Verifier();

      PK_Verifier& operator=(const PK_Verifier&) = delete;
      PK_Verifier(const PK_Verifier&) = delete;

      /**
      * Verify a signature.
      * @param msg the message that the signature belongs to, as a byte array
      * @param msg_length the length of the above byte array msg
      * @param sig the signature as a byte array
      * @param sig_length the length of the above byte array sig
      * @return true if the signature is valid
      */
      bool verify_message(const uint8_t msg[], size_t msg_length,
                          const uint8_t sig[], size_t sig_length);
      /**
      * Verify a signature.
      * @param msg the message that the signature belongs to
      * @param sig the signature
      * @return true if the signature is valid
      */
      template<typename Alloc, typename Alloc2>
      bool verify_message(const std::vector<uint8_t, Alloc>& msg,
                          const std::vector<uint8_t, Alloc2>& sig)
         {
         return verify_message(msg.data(), msg.size(),
                               sig.data(), sig.size());
         }

      /**
      * Add a message part (single byte) of the message corresponding to the
      * signature to be verified.
      * @param in the byte to add
      */
      void update(uint8_t in) { update(&in, 1); }

      /**
      * Add a message part of the message corresponding to the
      * signature to be verified.
      * @param msg_part the new message part as a byte array
      * @param length the length of the above byte array
      */
      void update(const uint8_t msg_part[], size_t length);

      /**
      * Add a message part of the message corresponding to the
      * signature to be verified.
      * @param in the new message part
      */
      void update(const std::vector<uint8_t>& in)
         { update(in.data(), in.size()); }

      /**
      * Add a message part of the message corresponding to the
      * signature to be verified.
      */
      void update(const std::string& in)
         {
         update(reinterpret_cast<const uint8_t*>(in.data()), in.size());
         }

      /**
      * Check the signature of the buffered message, i.e. the one build
      * by successive calls to update.
      * @param sig the signature to be verified as a byte array
      * @param length the length of the above byte array
      * @return true if the signature is valid, false otherwise
      */
      bool check_signature(const uint8_t sig[], size_t length);

      /**
      * Check the signature of the buffered message, i.e. the one build
      * by successive calls to update.
      * @param sig the signature to be verified
      * @return true if the signature is valid, false otherwise
      */
      template<typename Alloc>
      bool check_signature(const std::vector<uint8_t, Alloc>& sig)
         {
         return check_signature(sig.data(), sig.size());
         }

      /**
      * Set the format of the signatures fed to this verifier.
      * @param format the signature format to use
      */
      void set_input_format(Signature_Format format);

   private:
      std::unique_ptr<PK_Ops::Verification> m_op;
      Signature_Format m_sig_format;
      size_t m_parts, m_part_size;
   };

/**
* Key used for key agreement
*/
class BOTAN_DLL PK_Key_Agreement final
   {
   public:

      /**
      * Construct a PK Key Agreement.
      * @param key the key to use
      * @param rng the random generator to use
      * @param kdf name of the KDF to use (or 'Raw' for no KDF)
      * @param provider the algo provider to use (or empty for default)
      */
      PK_Key_Agreement(const Private_Key& key,
                       RandomNumberGenerator& rng,
                       const std::string& kdf,
                       const std::string& provider = "");

#if defined(BOTAN_PUBKEY_INCLUDE_DEPRECATED_CONSTRUCTORS)
      /**
      * Construct a PK Key Agreement.
      * @param key the key to use
      * @param kdf name of the KDF to use (or 'Raw' for no KDF)
      * @param provider the algo provider to use (or empty for default)
      */
      BOTAN_DEPRECATED("Use constructor taking a RNG object")
      PK_Key_Agreement(const Private_Key& key,
                       const std::string& kdf,
                       const std::string& provider = "") :
         PK_Key_Agreement(key, system_rng(), kdf, provider)
         {}
#endif

      ~PK_Key_Agreement();

      // For ECIES
      PK_Key_Agreement& operator=(PK_Key_Agreement&&);
      PK_Key_Agreement(PK_Key_Agreement&&);

      PK_Key_Agreement& operator=(const PK_Key_Agreement&) = delete;
      PK_Key_Agreement(const PK_Key_Agreement&) = delete;

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param in_len the length of in in bytes
      * @param params extra derivation params
      * @param params_len the length of params in bytes
      */
      SymmetricKey derive_key(size_t key_len,
                              const uint8_t in[],
                              size_t in_len,
                              const uint8_t params[],
                              size_t params_len) const;

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param in_len the length of in in bytes
      * @param params extra derivation params
      * @param params_len the length of params in bytes
      */
      SymmetricKey derive_key(size_t key_len,
                              const std::vector<uint8_t>& in,
                              const uint8_t params[],
                              size_t params_len) const
         {
         return derive_key(key_len, in.data(), in.size(),
                           params, params_len);
         }

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param in_len the length of in in bytes
      * @param params extra derivation params
      */
      SymmetricKey derive_key(size_t key_len,
                              const uint8_t in[], size_t in_len,
                              const std::string& params = "") const
         {
         return derive_key(key_len, in, in_len,
                           reinterpret_cast<const uint8_t*>(params.data()),
                           params.length());
         }

      /*
      * Perform Key Agreement Operation
      * @param key_len the desired key output size
      * @param in the other parties key
      * @param params extra derivation params
      */
      SymmetricKey derive_key(size_t key_len,
                              const std::vector<uint8_t>& in,
                              const std::string& params = "") const
         {
         return derive_key(key_len, in.data(), in.size(),
                           reinterpret_cast<const uint8_t*>(params.data()),
                           params.length());
         }

   private:
      std::unique_ptr<PK_Ops::Key_Agreement> m_op;
   };

/**
* Encryption using a standard message recovery algorithm like RSA or
* ElGamal, paired with an encoding scheme like OAEP.
*/
class BOTAN_DLL PK_Encryptor_EME final : public PK_Encryptor
   {
   public:
      size_t maximum_input_size() const override;

      /**
      * Construct an instance.
      * @param key the key to use inside the encryptor
      * @param rng the RNG to use
      * @param padding the message encoding scheme to use (eg "OAEP(SHA-256)")
      * @param provider the provider to use
      */
      PK_Encryptor_EME(const Public_Key& key,
                       RandomNumberGenerator& rng,
                       const std::string& padding,
                       const std::string& provider = "");

#if defined(BOTAN_PUBKEY_INCLUDE_DEPRECATED_CONSTRUCTORS)
      /**
      * Construct an instance.
      * @param key the key to use inside the encryptor
      * @param padding the message encoding scheme to use (eg "OAEP(SHA-256)")
      */
      BOTAN_DEPRECATED("Use constructor taking a RNG object")
      PK_Encryptor_EME(const Public_Key& key,
                       const std::string& padding,
                       const std::string& provider = "") :
         PK_Encryptor_EME(key, system_rng(), padding, provider) {}
#endif

      ~PK_Encryptor_EME();

      PK_Encryptor_EME& operator=(const PK_Encryptor_EME&) = delete;
      PK_Encryptor_EME(const PK_Encryptor_EME&) = delete;
   private:
      std::vector<uint8_t> enc(const uint8_t[], size_t,
                             RandomNumberGenerator& rng) const override;

      std::unique_ptr<PK_Ops::Encryption> m_op;
   };

/**
* Decryption with an MR algorithm and an EME.
*/
class BOTAN_DLL PK_Decryptor_EME final : public PK_Decryptor
   {
   public:
     /**
      * Construct an instance.
      * @param key the key to use inside the decryptor
      * @param rng the random generator to use
      * @param eme the EME to use
      * @param provider the provider to use
      */
      PK_Decryptor_EME(const Private_Key& key,
                       RandomNumberGenerator& rng,
                       const std::string& eme,
                       const std::string& provider = "");


#if defined(BOTAN_PUBKEY_INCLUDE_DEPRECATED_CONSTRUCTORS)
      /**
      * Construct an instance.
      * @param key the key to use inside the decryptor
      * @param padding the message encoding scheme to use (eg "OAEP(SHA-256)")
      */
      BOTAN_DEPRECATED("Use constructor taking a RNG object")
      PK_Decryptor_EME(const Private_Key& key,
                       const std::string& eme,
                       const std::string& provider = "") :
         PK_Decryptor_EME(key, system_rng(), eme, provider) {}
#endif

      ~PK_Decryptor_EME();
      PK_Decryptor_EME& operator=(const PK_Decryptor_EME&) = delete;
      PK_Decryptor_EME(const PK_Decryptor_EME&) = delete;
   private:
      secure_vector<uint8_t> do_decrypt(uint8_t& valid_mask,
                                     const uint8_t in[],
                                     size_t in_len) const override;

      std::unique_ptr<PK_Ops::Decryption> m_op;
   };

/**
* Public Key Key Encapsulation Mechanism Encryption.
*/
class BOTAN_DLL PK_KEM_Encryptor final
   {
   public:
      /**
      * Construct an instance.
      * @param key the key to use inside the encryptor
      * @param rng the RNG to use
      * @param kem_param additional KEM parameters
      * @param provider the provider to use
      */
      PK_KEM_Encryptor(const Public_Key& key,
                       RandomNumberGenerator& rng,
                       const std::string& kem_param = "",
                       const std::string& provider = "");

#if defined(BOTAN_PUBKEY_INCLUDE_DEPRECATED_CONSTRUCTORS)
      BOTAN_DEPRECATED("Use constructor taking a RNG object")
      PK_KEM_Encryptor(const Public_Key& key,
                       const std::string& kem_param = "",
                       const std::string& provider = "") :
         PK_KEM_Encryptor(key, system_rng(), kem_param, provider) {}
#endif

      ~PK_KEM_Encryptor();

      PK_KEM_Encryptor& operator=(const PK_KEM_Encryptor&) = delete;
      PK_KEM_Encryptor(const PK_KEM_Encryptor&) = delete;

      /**
      * Generate a shared key for data encryption.
      * @param out_encapsulated_key the generated encapsulated key
      * @param out_shared_key the generated shared key
      * @param desired_shared_key_len desired size of the shared key in bytes
      * @param rng the RNG to use
      * @param salt a salt value used in the KDF
      * @param salt_len size of the salt value in bytes
      */
      void encrypt(secure_vector<uint8_t>& out_encapsulated_key,
                   secure_vector<uint8_t>& out_shared_key,
                   size_t desired_shared_key_len,
                   Botan::RandomNumberGenerator& rng,
                   const uint8_t salt[],
                   size_t salt_len);

      /**
      * Generate a shared key for data encryption.
      * @param out_encapsulated_key the generated encapsulated key
      * @param out_shared_key the generated shared key
      * @param desired_shared_key_len desired size of the shared key in bytes
      * @param rng the RNG to use
      * @param salt a salt value used in the KDF
      */
      template<typename Alloc>
         void encrypt(secure_vector<uint8_t>& out_encapsulated_key,
                      secure_vector<uint8_t>& out_shared_key,
                      size_t desired_shared_key_len,
                      Botan::RandomNumberGenerator& rng,
                      const std::vector<uint8_t, Alloc>& salt)
         {
         this->encrypt(out_encapsulated_key,
                       out_shared_key,
                       desired_shared_key_len,
                       rng,
                       salt.data(), salt.size());
         }


      /**
      * Generate a shared key for data encryption.
      * @param out_encapsulated_key the generated encapsulated key
      * @param out_shared_key the generated shared key
      * @param desired_shared_key_len desired size of the shared key in bytes
      * @param rng the RNG to use
      */
      void encrypt(secure_vector<uint8_t>& out_encapsulated_key,
                   secure_vector<uint8_t>& out_shared_key,
                   size_t desired_shared_key_len,
                   Botan::RandomNumberGenerator& rng)
         {
         this->encrypt(out_encapsulated_key,
                       out_shared_key,
                       desired_shared_key_len,
                       rng,
                       nullptr,
                       0);
         }

   private:
      std::unique_ptr<PK_Ops::KEM_Encryption> m_op;
   };

/**
* Public Key Key Encapsulation Mechanism Decryption.
*/
class BOTAN_DLL PK_KEM_Decryptor final
   {
   public:
      /**
      * Construct an instance.
      * @param key the key to use inside the decryptor
      * @param rng the RNG to use
      * @param kem_param additional KEM parameters
      * @param provider the provider to use
      */
      PK_KEM_Decryptor(const Private_Key& key,
                       RandomNumberGenerator& rng,
                       const std::string& kem_param = "",
                       const std::string& provider = "");

#if defined(BOTAN_PUBKEY_INCLUDE_DEPRECATED_CONSTRUCTORS)
      BOTAN_DEPRECATED("Use constructor taking a RNG object")
      PK_KEM_Decryptor(const Private_Key& key,
                       const std::string& kem_param = "",
                       const std::string& provider = "") :
         PK_KEM_Decryptor(key, system_rng(), kem_param, provider)
         {}
#endif

      ~PK_KEM_Decryptor();
      PK_KEM_Decryptor& operator=(const PK_KEM_Decryptor&) = delete;
      PK_KEM_Decryptor(const PK_KEM_Decryptor&) = delete;

      /**
      * Decrypts the shared key for data encryption.
      * @param encap_key the encapsulated key
      * @param encap_key_len size of the encapsulated key in bytes
      * @param desired_shared_key_len desired size of the shared key in bytes
      * @param salt a salt value used in the KDF
      * @param salt_len size of the salt value in bytes
      * @return the shared data encryption key
      */
      secure_vector<uint8_t> decrypt(const uint8_t encap_key[],
                                  size_t encap_key_len,
                                  size_t desired_shared_key_len,
                                  const uint8_t salt[],
                                  size_t salt_len);

      /**
      * Decrypts the shared key for data encryption.
      * @param encap_key the encapsulated key
      * @param encap_key_len size of the encapsulated key in bytes
      * @param desired_shared_key_len desired size of the shared key in bytes
      * @return the shared data encryption key
      */
      secure_vector<uint8_t> decrypt(const uint8_t encap_key[],
                                  size_t encap_key_len,
                                  size_t desired_shared_key_len)
         {
         return this->decrypt(encap_key, encap_key_len,
                              desired_shared_key_len,
                              nullptr, 0);
         }

      /**
      * Decrypts the shared key for data encryption.
      * @param encap_key the encapsulated key
      * @param desired_shared_key_len desired size of the shared key in bytes
      * @param salt a salt value used in the KDF
      * @return the shared data encryption key
      */
      template<typename Alloc1, typename Alloc2>
         secure_vector<uint8_t> decrypt(const std::vector<uint8_t, Alloc1>& encap_key,
                                     size_t desired_shared_key_len,
                                     const std::vector<uint8_t, Alloc2>& salt)
         {
         return this->decrypt(encap_key.data(), encap_key.size(),
                              desired_shared_key_len,
                              salt.data(), salt.size());
         }

   private:
      std::unique_ptr<PK_Ops::KEM_Decryption> m_op;
   };

}


namespace Botan {

/**
* DLIES Encryption
*/
class BOTAN_DLL DLIES_Encryptor : public PK_Encryptor
   {
   public:
      /**
      * Stream mode: use KDF to provide a stream of bytes to xor with the message
      *
      * @param own_priv_key own (ephemeral) DH private key
      * @param rng the RNG to use
      * @param kdf the KDF that should be used
      * @param mac the MAC function that should be used
      * @param mac_key_len key length of the MAC function. Default = 20 bytes
      *
      * output = (ephemeral) public key + ciphertext + tag
      */
      DLIES_Encryptor(const DH_PrivateKey& own_priv_key,
                      RandomNumberGenerator& rng,
                      KDF* kdf,
                      MessageAuthenticationCode* mac,
                      size_t mac_key_len = 20);

      /**
      * Block cipher mode
      *
      * @param own_priv_key own (ephemeral) DH private key
      * @param rng the RNG to use
      * @param kdf the KDF that should be used
      * @param cipher the block cipher that should be used
      * @param cipher_key_len the key length of the block cipher
      * @param mac the MAC function that should be used
      * @param mac_key_len key length of the MAC function. Default = 20 bytes
      *
      * output = (ephemeral) public key + ciphertext + tag
      */
      DLIES_Encryptor(const DH_PrivateKey& own_priv_key,
                      RandomNumberGenerator& rng,
                      KDF* kdf,
                      Cipher_Mode* cipher,
                      size_t cipher_key_len,
                      MessageAuthenticationCode* mac,
                      size_t mac_key_len = 20);

      // Set the other parties public key
      inline void set_other_key(const std::vector<uint8_t>& other_pub_key)
         {
         m_other_pub_key = other_pub_key;
         }

      /// Set the initialization vector for the data encryption method
      inline void set_initialization_vector(const InitializationVector& iv)
         {
         m_iv = iv;
         }

   private:
      std::vector<uint8_t> enc(const uint8_t[], size_t,
                            RandomNumberGenerator&) const override;

      size_t maximum_input_size() const override;

      std::vector<uint8_t> m_other_pub_key;
      std::vector<uint8_t> m_own_pub_key;
      PK_Key_Agreement m_ka;
      std::unique_ptr<KDF> m_kdf;
      std::unique_ptr<Cipher_Mode> m_cipher;
      const size_t m_cipher_key_len;
      std::unique_ptr<MessageAuthenticationCode> m_mac;
      const size_t m_mac_keylen;
      InitializationVector m_iv;
   };

/**
* DLIES Decryption
*/
class BOTAN_DLL DLIES_Decryptor : public PK_Decryptor
   {
   public:
      /**
      * Stream mode: use KDF to provide a stream of bytes to xor with the message
      *
      * @param own_priv_key own (ephemeral) DH private key
      * @param rng the RNG to use
      * @param kdf the KDF that should be used
      * @param mac the MAC function that should be used
      * @param mac_key_len key length of the MAC function. Default = 20 bytes
      *
      * input = (ephemeral) public key + ciphertext + tag
      */
      DLIES_Decryptor(const DH_PrivateKey& own_priv_key,
                      RandomNumberGenerator& rng,
                      KDF* kdf,
                      MessageAuthenticationCode* mac,
                      size_t mac_key_len = 20);

      /**
      * Block cipher mode
      *
      * @param own_priv_key own (ephemeral) DH private key
      * @param rng the RNG to use
      * @param kdf the KDF that should be used
      * @param cipher the block cipher that should be used
      * @param cipher_key_len the key length of the block cipher
      * @param mac the MAC function that should be used
      * @param mac_key_len key length of the MAC function. Default = 20 bytes
      *
      * input = (ephemeral) public key + ciphertext + tag
      */
      DLIES_Decryptor(const DH_PrivateKey& own_priv_key,
                      RandomNumberGenerator& rng,
                      KDF* kdf,
                      Cipher_Mode* cipher,
                      size_t cipher_key_len,
                      MessageAuthenticationCode* mac,
                      size_t mac_key_len = 20);

      /// Set the initialization vector for the data decryption method
      inline void set_initialization_vector(const InitializationVector& iv)
         {
         m_iv = iv;
         }

   private:
      secure_vector<uint8_t> do_decrypt(uint8_t& valid_mask,
                                     const uint8_t in[], size_t in_len) const override;

      const size_t m_pub_key_size;
      PK_Key_Agreement m_ka;
      std::unique_ptr<KDF> m_kdf;
      std::unique_ptr<Cipher_Mode> m_cipher;
      const size_t m_cipher_key_len;
      std::unique_ptr<MessageAuthenticationCode> m_mac;
      const size_t m_mac_keylen;
      InitializationVector m_iv;
   };

}


namespace Botan {

/**
* DSA Public Key
*/
class BOTAN_DLL DSA_PublicKey : public virtual DL_Scheme_PublicKey
   {
   public:
      std::string algo_name() const override { return "DSA"; }

      DL_Group::Format group_format() const override { return DL_Group::ANSI_X9_57; }
      size_t message_parts() const override { return 2; }
      size_t message_part_size() const override { return group_q().bytes(); }

      /**
      * Load a public key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded public key bits
      */
      DSA_PublicKey(const AlgorithmIdentifier& alg_id,
                    const std::vector<uint8_t>& key_bits) :
         DL_Scheme_PublicKey(alg_id, key_bits, DL_Group::ANSI_X9_57)
         {
         }

      /**
      * Create a public key.
      * @param group the underlying DL group
      * @param y the public value y = g^x mod p
      */
      DSA_PublicKey(const DL_Group& group, const BigInt& y);

      std::unique_ptr<PK_Ops::Verification>
         create_verification_op(const std::string& params,
                                const std::string& provider) const override;
   protected:
      DSA_PublicKey() {}
   };

/**
* DSA Private Key
*/
class BOTAN_DLL DSA_PrivateKey : public DSA_PublicKey,
                                 public virtual DL_Scheme_PrivateKey
   {
   public:
      /**
      * Load a private key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits PKCS#8 structure
      */
      DSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                     const secure_vector<uint8_t>& key_bits);

      /**
      * Create a private key.
      * @param rng the RNG to use
      * @param group the underlying DL group
      * @param private_key the private key (if zero, a new random key is generated)
      */
      DSA_PrivateKey(RandomNumberGenerator& rng,
                     const DL_Group& group,
                     const BigInt& private_key = 0);

      bool check_key(RandomNumberGenerator& rng, bool strong) const override;

      std::unique_ptr<PK_Ops::Signature>
         create_signature_op(RandomNumberGenerator& rng,
                             const std::string& params,
                             const std::string& provider) const override;
   };

}


namespace Botan {

/**
* EAX base class
*/
class BOTAN_DLL EAX_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const uint8_t ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      // EAX supports arbitrary nonce lengths
      bool valid_nonce_length(size_t) const override { return true; }

      size_t tag_size() const override { return m_tag_size; }

      void clear() override;

      void reset() override;

   protected:
      /**
      * @param cipher the cipher to use
      * @param tag_size is how big the auth tag will be
      */
      EAX_Mode(BlockCipher* cipher, size_t tag_size);

      size_t block_size() const { return m_cipher->block_size(); }

      size_t m_tag_size;

      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<StreamCipher> m_ctr;
      std::unique_ptr<MessageAuthenticationCode> m_cmac;

      secure_vector<uint8_t> m_ad_mac;

      secure_vector<uint8_t> m_nonce_mac;
   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;

      void key_schedule(const uint8_t key[], size_t length) override;
   };

/**
* EAX Encryption
*/
class BOTAN_DLL EAX_Encryption final : public EAX_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      * @param tag_size is how big the auth tag will be
      */
      EAX_Encryption(BlockCipher* cipher, size_t tag_size = 0) :
         EAX_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   };

/**
* EAX Decryption
*/
class BOTAN_DLL EAX_Decryption final : public EAX_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      * @param tag_size is how big the auth tag will be
      */
      EAX_Decryption(BlockCipher* cipher, size_t tag_size = 0) :
         EAX_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length >= tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   };

}


namespace Botan {

/**
* Exception thrown if you try to convert a zero point to an affine
* coordinate
*/
struct BOTAN_DLL Illegal_Transformation : public Exception
   {
   explicit Illegal_Transformation(const std::string& err =
                          "Requested transformation is not possible") :
      Exception(err) {}
   };

/**
* Exception thrown if some form of illegal point is decoded
*/
struct BOTAN_DLL Illegal_Point : public Exception
   {
   explicit Illegal_Point(const std::string& err = "Malformed ECP point detected") :
      Exception(err) {}
   };

/**
* This class represents one point on a curve of GF(p)
*/
class BOTAN_DLL PointGFp
   {
   public:
      enum Compression_Type {
         UNCOMPRESSED = 0,
         COMPRESSED   = 1,
         HYBRID       = 2
      };

      /**
      * Construct an uninitialized PointGFp
      */
      PointGFp() {}

      /**
      * Construct the zero point
      * @param curve The base curve
      */
      explicit PointGFp(const CurveGFp& curve);

      static PointGFp zero_of(const CurveGFp& curve)
         {
         return PointGFp(curve);
         }

      /**
      * Copy constructor
      */
      PointGFp(const PointGFp&) = default;

      /**
      * Move Constructor
      */
      PointGFp(PointGFp&& other)
         {
         this->swap(other);
         }

      /**
      * Standard Assignment
      */
      PointGFp& operator=(const PointGFp&) = default;

      /**
      * Move Assignment
      */
      PointGFp& operator=(PointGFp&& other)
         {
         if(this != &other)
            this->swap(other);
         return (*this);
         }

      /**
      * Construct a point from its affine coordinates
      * @param curve the base curve
      * @param x affine x coordinate
      * @param y affine y coordinate
      */
      PointGFp(const CurveGFp& curve, const BigInt& x, const BigInt& y);

      /**
      * += Operator
      * @param rhs the PointGFp to add to the local value
      * @result resulting PointGFp
      */
      PointGFp& operator+=(const PointGFp& rhs);

      /**
      * -= Operator
      * @param rhs the PointGFp to subtract from the local value
      * @result resulting PointGFp
      */
      PointGFp& operator-=(const PointGFp& rhs);

      /**
      * *= Operator
      * @param scalar the PointGFp to multiply with *this
      * @result resulting PointGFp
      */

      PointGFp& operator*=(const BigInt& scalar);

      /**
      * Multiplication Operator
      * @param scalar the scalar value
      * @param point the point value
      * @return scalar*point on the curve
      */
      friend BOTAN_DLL PointGFp operator*(const BigInt& scalar, const PointGFp& point);

      /**
      * Multiexponentiation
      * @param p1 a point
      * @param z1 a scalar
      * @param p2 a point
      * @param z2 a scalar
      * @result (p1 * z1 + p2 * z2)
      */
      friend BOTAN_DLL PointGFp multi_exponentiate(
        const PointGFp& p1, const BigInt& z1,
        const PointGFp& p2, const BigInt& z2);

      /**
      * Negate this point
      * @return *this
      */
      PointGFp& negate()
         {
         if(!is_zero())
            m_coord_y = m_curve.get_p() - m_coord_y;
         return *this;
         }

      /**
      * Return base curve of this point
      * @result the curve over GF(p) of this point
      */
      const CurveGFp& get_curve() const { return m_curve; }

      /**
      * get affine x coordinate
      * @result affine x coordinate
      */
      BigInt get_affine_x() const;

      /**
      * get affine y coordinate
      * @result affine y coordinate
      */
      BigInt get_affine_y() const;

      /**
      * Is this the point at infinity?
      * @result true, if this point is at infinity, false otherwise.
      */
      bool is_zero() const
         { return (m_coord_x.is_zero() && m_coord_z.is_zero()); }

      /**
      * Checks whether the point is to be found on the underlying
      * curve; used to prevent fault attacks.
      * @return if the point is on the curve
      */
      bool on_the_curve() const;

      /**
      * swaps the states of *this and other, does not throw!
      * @param other the object to swap values with
      */
      void swap(PointGFp& other);

      /**
      * Randomize the point representation
      * The actual value (get_affine_x, get_affine_y) does not change
      */
      void randomize_repr(RandomNumberGenerator& rng);

      /**
      * Equality operator
      */
      bool operator==(const PointGFp& other) const;
   private:
      friend class Blinded_Point_Multiply;

      BigInt curve_mult(const BigInt& x, const BigInt& y) const
         {
         BigInt z;
         m_curve.mul(z, x, y, m_monty_ws);
         return z;
         }

      void curve_mult(BigInt& z, const BigInt& x, const BigInt& y) const
         {
         m_curve.mul(z, x, y, m_monty_ws);
         }

      BigInt curve_sqr(const BigInt& x) const
         {
         BigInt z;
         m_curve.sqr(z, x, m_monty_ws);
         return z;
         }

      void curve_sqr(BigInt& z, const BigInt& x) const
         {
         m_curve.sqr(z, x, m_monty_ws);
         }

      /**
      * Point addition
      * @param workspace temp space, at least 11 elements
      */
      void add(const PointGFp& other, std::vector<BigInt>& workspace);

      /**
      * Point doubling
      * @param workspace temp space, at least 9 elements
      */
      void mult2(std::vector<BigInt>& workspace);

      CurveGFp m_curve;
      BigInt m_coord_x, m_coord_y, m_coord_z;
      mutable secure_vector<word> m_monty_ws; // workspace for Montgomery
   };

// relational operators
inline bool operator!=(const PointGFp& lhs, const PointGFp& rhs)
   {
   return !(rhs == lhs);
   }

// arithmetic operators
inline PointGFp operator-(const PointGFp& lhs)
   {
   return PointGFp(lhs).negate();
   }

inline PointGFp operator+(const PointGFp& lhs, const PointGFp& rhs)
   {
   PointGFp tmp(lhs);
   return tmp += rhs;
   }

inline PointGFp operator-(const PointGFp& lhs, const PointGFp& rhs)
   {
   PointGFp tmp(lhs);
   return tmp -= rhs;
   }

inline PointGFp operator*(const PointGFp& point, const BigInt& scalar)
   {
   return scalar * point;
   }

// encoding and decoding
secure_vector<uint8_t> BOTAN_DLL EC2OSP(const PointGFp& point, uint8_t format);

PointGFp BOTAN_DLL OS2ECP(const uint8_t data[], size_t data_len,
                          const CurveGFp& curve);

template<typename Alloc>
PointGFp OS2ECP(const std::vector<uint8_t, Alloc>& data, const CurveGFp& curve)
   { return OS2ECP(data.data(), data.size(), curve); }

/**

*/
class BOTAN_DLL Blinded_Point_Multiply
   {
   public:
      Blinded_Point_Multiply(const PointGFp& base, const BigInt& order, size_t h = 0);

      PointGFp blinded_multiply(const BigInt& scalar, RandomNumberGenerator& rng);
   private:
      const size_t m_h;
      const BigInt& m_order;
      std::vector<BigInt> m_ws;
      std::vector<PointGFp> m_U;
   };

}

namespace std {

template<>
inline void swap<Botan::PointGFp>(Botan::PointGFp& x, Botan::PointGFp& y)
   { x.swap(y); }

}


namespace Botan {

/**
* This class represents elliptic curce domain parameters
*/
enum EC_Group_Encoding {
   EC_DOMPAR_ENC_EXPLICIT = 0,
   EC_DOMPAR_ENC_IMPLICITCA = 1,
   EC_DOMPAR_ENC_OID = 2
};

/**
* Class representing an elliptic curve
*/
class BOTAN_DLL EC_Group
   {
   public:

      /**
      * Construct Domain paramers from specified parameters
      * @param curve elliptic curve
      * @param base_point a base point
      * @param order the order of the base point
      * @param cofactor the cofactor
      */
      EC_Group(const CurveGFp& curve,
               const PointGFp& base_point,
               const BigInt& order,
               const BigInt& cofactor) :
         m_curve(curve),
         m_base_point(base_point),
         m_order(order),
         m_cofactor(cofactor),
         m_oid("")
         {}

      /**
      * Decode a BER encoded ECC domain parameter set
      * @param ber_encoding the bytes of the BER encoding
      */
      explicit EC_Group(const std::vector<uint8_t>& ber_encoding);

      /**
      * Create an EC domain by OID (or throw if unknown)
      * @param oid the OID of the EC domain to create
      */
      explicit EC_Group(const OID& oid);

      /**
      * Create an EC domain from PEM encoding (as from PEM_encode), or
      * from an OID name (eg "secp256r1", or "1.2.840.10045.3.1.7")
      * @param pem_or_oid PEM-encoded data, or an OID
      */
      EC_Group(const std::string& pem_or_oid = "");

      /**
      * Create the DER encoding of this domain
      * @param form of encoding to use
      * @returns bytes encododed as DER
      */
      std::vector<uint8_t> DER_encode(EC_Group_Encoding form) const;

      /**
      * Return the PEM encoding (always in explicit form)
      * @return string containing PEM data
      */
      std::string PEM_encode() const;

      /**
      * Return domain parameter curve
      * @result domain parameter curve
      */
      const CurveGFp& get_curve() const { return m_curve; }

      /**
      * Return group base point
      * @result base point
      */
      const PointGFp& get_base_point() const { return m_base_point; }

      /**
      * Return the order of the base point
      * @result order of the base point
      */
      const BigInt& get_order() const { return m_order; }

      /**
      * Return the cofactor
      * @result the cofactor
      */
      const BigInt& get_cofactor() const { return m_cofactor; }

      bool initialized() const { return !m_base_point.is_zero(); }

      /**
      * Return the OID of these domain parameters
      * @result the OID
      */
      std::string get_oid() const { return m_oid; }

      bool operator==(const EC_Group& other) const
         {
         return ((get_curve() == other.get_curve()) &&
                 (get_base_point() == other.get_base_point()) &&
                 (get_order() == other.get_order()) &&
                 (get_cofactor() == other.get_cofactor()));
         }

      /**
      * Return PEM representation of named EC group
      */
      static std::string PEM_for_named_group(const std::string& name);

   private:
      CurveGFp m_curve;
      PointGFp m_base_point;
      BigInt m_order, m_cofactor;
      std::string m_oid;
   };

inline bool operator!=(const EC_Group& lhs,
                       const EC_Group& rhs)
   {
   return !(lhs == rhs);
   }

// For compatibility with 1.8
typedef EC_Group EC_Domain_Params;

}


namespace Botan {

/**
* This class represents abstract ECC public keys. When encoding a key
* via an encoder that can be accessed via the corresponding member
* functions, the key will decide upon its internally stored encoding
* information whether to encode itself with or without domain
* parameters, or using the domain parameter oid. Furthermore, a public
* key without domain parameters can be decoded. In that case, it
* cannot be used for verification until its domain parameters are set
* by calling the corresponding member function.
*/
class BOTAN_DLL EC_PublicKey : public virtual Public_Key
   {
   public:
      /**
      * Create a public key.
      * @param dom_par EC domain parameters
      * @param pub_point public point on the curve
      */
      EC_PublicKey(const EC_Group& dom_par,
                   const PointGFp& pub_point);

      /**
      * Load a public key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded public key bits
      */
      EC_PublicKey(const AlgorithmIdentifier& alg_id,
                   const std::vector<uint8_t>& key_bits);

      /**
      * Get the public point of this key.
      * @throw Invalid_State is thrown if the
      * domain parameters of this point are not set
      * @result the public point of this key
      */
      const PointGFp& public_point() const { return m_public_key; }

      AlgorithmIdentifier algorithm_identifier() const override;

      std::vector<uint8_t> public_key_bits() const override;

      bool check_key(RandomNumberGenerator& rng,
                     bool strong) const override;

      /**
      * Get the domain parameters of this key.
      * @throw Invalid_State is thrown if the
      * domain parameters of this point are not set
      * @result the domain parameters of this key
      */
      const EC_Group& domain() const { return m_domain_params; }

      /**
      * Set the domain parameter encoding to be used when encoding this key.
      * @param enc the encoding to use
      */
      void set_parameter_encoding(EC_Group_Encoding enc);

      /**
      * Return the DER encoding of this keys domain in whatever format
      * is preset for this particular key
      */
      std::vector<uint8_t> DER_domain() const
         { return domain().DER_encode(domain_format()); }

      /**
      * Get the domain parameter encoding to be used when encoding this key.
      * @result the encoding to use
      */
      EC_Group_Encoding domain_format() const
         { return m_domain_encoding; }

      size_t key_length() const override;
      size_t estimated_strength() const override;

   protected:
      EC_PublicKey() : m_domain_params{}, m_public_key{}, m_domain_encoding(EC_DOMPAR_ENC_EXPLICIT)
      {}

      EC_Group m_domain_params;
      PointGFp m_public_key;
      EC_Group_Encoding m_domain_encoding;
   };

/**
* This abstract class represents ECC private keys
*/
class BOTAN_DLL EC_PrivateKey : public virtual EC_PublicKey,
                                public virtual Private_Key
   {
   public:
      /*
      * If x=0, creates a new private key in the domain
      * using the given rng. If with_modular_inverse is set,
      * the public key will be calculated by multiplying
      * the base point with the modular inverse of
      * x (as in ECGDSA and ECKCDSA), otherwise by
      * multiplying directly with x (as in ECDSA).
      */
      EC_PrivateKey(RandomNumberGenerator& rng,
                    const EC_Group& domain,
                    const BigInt& x,
                    bool with_modular_inverse=false);

      /*
      * Creates a new private key object from the given
      * key_bits. If with_modular_inverse is set,
      * the public key will be calculated by multiplying
      * the base point with the modular inverse of
      * x (as in ECGDSA and ECKCDSA), otherwise by
      * multiplying directly with x (as in ECDSA).
      */
      EC_PrivateKey(const AlgorithmIdentifier& alg_id,
                    const secure_vector<uint8_t>& key_bits,
                    bool with_modular_inverse=false);

      secure_vector<uint8_t> private_key_bits() const override;

      /**
      * Get the private key value of this key object.
      * @result the private key value of this key object
      */
      const BigInt& private_value() const;
   protected:
      EC_PrivateKey() {}

      BigInt m_private_key;
   };

}


namespace Botan {

/**
* This class represents ECDH Public Keys.
*/
class BOTAN_DLL ECDH_PublicKey : public virtual EC_PublicKey
   {
   public:
      /**
      * Create an ECDH public key.
      * @param alg_id algorithm identifier
      * @param key_bits DER encoded public key bits
      */
      ECDH_PublicKey(const AlgorithmIdentifier& alg_id,
                     const std::vector<uint8_t>& key_bits) :
         EC_PublicKey(alg_id, key_bits) {}

      /**
      * Construct a public key from a given public point.
      * @param dom_par the domain parameters associated with this key
      * @param public_point the public point defining this key
      */
      ECDH_PublicKey(const EC_Group& dom_par,
                     const PointGFp& public_point) :
         EC_PublicKey(dom_par, public_point) {}

      /**
      * Get this keys algorithm name.
      * @return this keys algorithm name
      */
      std::string algo_name() const override { return "ECDH"; }

      /**
      * @return public point value
      */
      std::vector<uint8_t> public_value() const
         { return unlock(EC2OSP(public_point(), PointGFp::UNCOMPRESSED)); }

      /**
      * @return public point value
      */
      std::vector<uint8_t> public_value(PointGFp::Compression_Type type) const
         { return unlock(EC2OSP(public_point(), type)); }

   protected:
      ECDH_PublicKey();
   };

/**
* This class represents ECDH Private Keys.
*/
class BOTAN_DLL ECDH_PrivateKey : public ECDH_PublicKey,
                                  public EC_PrivateKey,
                                  public PK_Key_Agreement_Key
   {
   public:

      /**
      * Create an ECDH public key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits X.509 subject public key info structure
      */
      ECDH_PrivateKey(const AlgorithmIdentifier& alg_id,
                      const secure_vector<uint8_t>& key_bits) :
         EC_PrivateKey(alg_id, key_bits) {}

      /**
      * Generate a new private key
      * @param rng a random number generator
      * @param domain parameters to used for this key
      * @param x the private key; if zero, a new random key is generated
      */
      ECDH_PrivateKey(RandomNumberGenerator& rng,
                      const EC_Group& domain,
                      const BigInt& x = 0) :
         EC_PrivateKey(rng, domain, x) {}

      std::vector<uint8_t> public_value() const override
         { return ECDH_PublicKey::public_value(PointGFp::UNCOMPRESSED); }

      std::vector<uint8_t> public_value(PointGFp::Compression_Type type) const
         { return ECDH_PublicKey::public_value(type); }

      std::unique_ptr<PK_Ops::Key_Agreement>
         create_key_agreement_op(RandomNumberGenerator& rng,
                                 const std::string& params,
                                 const std::string& provider) const override;
   };

}


namespace Botan {

/**
* This class represents ECDSA Public Keys.
*/
class BOTAN_DLL ECDSA_PublicKey : public virtual EC_PublicKey
   {
   public:

      /**
      * Create a public key from a given public point.
      * @param dom_par the domain parameters associated with this key
      * @param public_point the public point defining this key
      */
      ECDSA_PublicKey(const EC_Group& dom_par,
                      const PointGFp& public_point) :
         EC_PublicKey(dom_par, public_point) {}

      /**
      * Load a public key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded public key bits
      */
      ECDSA_PublicKey(const AlgorithmIdentifier& alg_id,
                      const std::vector<uint8_t>& key_bits) :
         EC_PublicKey(alg_id, key_bits) {}

      /**
      * Get this keys algorithm name.
      * @result this keys algorithm name ("ECDSA")
      */
      std::string algo_name() const override { return "ECDSA"; }

      size_t message_parts() const override { return 2; }

      size_t message_part_size() const override
         { return domain().get_order().bytes(); }

      std::unique_ptr<PK_Ops::Verification>
         create_verification_op(const std::string& params,
                                const std::string& provider) const override;
   protected:
      ECDSA_PublicKey() {}
   };

/**
* This class represents ECDSA Private Keys
*/
class BOTAN_DLL ECDSA_PrivateKey : public ECDSA_PublicKey,
                                   public EC_PrivateKey
   {
   public:

      /**
      * Load a private key
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits PKCS #8 structure
      */
      ECDSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                       const secure_vector<uint8_t>& key_bits) :
         EC_PrivateKey(alg_id, key_bits) {}

      /**
      * Create a private key.
      * @param rng a random number generator
      * @param domain parameters to used for this key
      * @param x the private key (if zero, generate a new random key)
      */
      ECDSA_PrivateKey(RandomNumberGenerator& rng,
                       const EC_Group& domain,
                       const BigInt& x = 0) :
         EC_PrivateKey(rng, domain, x) {}

      bool check_key(RandomNumberGenerator& rng, bool) const override;

      std::unique_ptr<PK_Ops::Signature>
         create_signature_op(RandomNumberGenerator& rng,
                             const std::string& params,
                             const std::string& provider) const override;
   };

}


namespace Botan {

/**
* This class represents ECGDSA public keys.
*/
class BOTAN_DLL ECGDSA_PublicKey : public virtual EC_PublicKey
   {
   public:

      /**
      * Construct a public key from a given public point.
      * @param dom_par the domain parameters associated with this key
      * @param public_point the public point defining this key
      */
      ECGDSA_PublicKey(const EC_Group& dom_par,
                      const PointGFp& public_point) :
         EC_PublicKey(dom_par, public_point) {}

      /**
      * Load a public key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded public key bits
      */
      ECGDSA_PublicKey(const AlgorithmIdentifier& alg_id,
                      const std::vector<uint8_t>& key_bits) :
         EC_PublicKey(alg_id, key_bits) {}

      /**
      * Get this keys algorithm name.
      * @result this keys algorithm name ("ECGDSA")
      */
      std::string algo_name() const override { return "ECGDSA"; }

      size_t message_parts() const override { return 2; }

      size_t message_part_size() const override
         { return domain().get_order().bytes(); }

      std::unique_ptr<PK_Ops::Verification>
         create_verification_op(const std::string& params,
                                const std::string& provider) const override;
   protected:
      ECGDSA_PublicKey() {}
   };

/**
* This class represents ECGDSA private keys.
*/
class BOTAN_DLL ECGDSA_PrivateKey : public ECGDSA_PublicKey,
                                    public EC_PrivateKey
   {
   public:

      /**
      * Load a private key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits PKCS #8 structure
      */
      ECGDSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                       const secure_vector<uint8_t>& key_bits) :
         EC_PrivateKey(alg_id, key_bits, true) {}

      /**
      * Generate a new private key.
      * @param rng a random number generator
      * @param domain parameters to used for this key
      * @param x the private key (if zero, generate a new random key)
      */
      ECGDSA_PrivateKey(RandomNumberGenerator& rng,
                       const EC_Group& domain,
                       const BigInt& x = 0) :
         EC_PrivateKey(rng, domain, x, true) {}

      bool check_key(RandomNumberGenerator& rng, bool) const override;

      std::unique_ptr<PK_Ops::Signature>
         create_signature_op(RandomNumberGenerator& rng,
                             const std::string& params,
                             const std::string& provider) const override;
   };

}


namespace Botan {

class RandomNumberGenerator;

enum class ECIES_Flags : uint32_t
   {
   NONE = 0,

   /// if set: prefix the input of the (ecdh) key agreement with the encoded (ephemeral) public key
   SINGLE_HASH_MODE = 1,

   /// (decryption only) if set: use cofactor multiplication during (ecdh) key agreement
   COFACTOR_MODE = 2,

   /// if set: use ecdhc instead of ecdh
   OLD_COFACTOR_MODE = 4,

   /// (decryption only) if set: test if the (ephemeral) public key is on the curve
   CHECK_MODE = 8
   };

inline ECIES_Flags operator |(ECIES_Flags a, ECIES_Flags b)
   {
   return static_cast<ECIES_Flags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
   }

inline ECIES_Flags operator &(ECIES_Flags a, ECIES_Flags b)
   {
   return static_cast<ECIES_Flags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
   }

/**
* Parameters for ECIES secret derivation
*/
class BOTAN_DLL ECIES_KA_Params
   {
   public:
      /**
      * @param domain ec domain parameters of the involved ec keys
      * @param kdf_spec name of the key derivation function
      * @param length length of the secret to be derived
      * @param compression_type format of encoded keys (affects the secret derivation if single_hash_mode is used)
      * @param flags options, see documentation of ECIES_Flags
      */
      ECIES_KA_Params(const EC_Group& domain, const std::string& kdf_spec, size_t length,
                      PointGFp::Compression_Type compression_type, ECIES_Flags flags);

      virtual ~ECIES_KA_Params() = default;

      inline const EC_Group& domain() const
         {
         return m_domain;
         }

      inline size_t secret_length() const
         {
         return m_length;
         }

      inline bool single_hash_mode() const
         {
         return (m_flags & ECIES_Flags::SINGLE_HASH_MODE) == ECIES_Flags::SINGLE_HASH_MODE;
         }

      inline bool cofactor_mode() const
         {
         return (m_flags & ECIES_Flags::COFACTOR_MODE) == ECIES_Flags::COFACTOR_MODE;
         }

      inline bool old_cofactor_mode() const
         {
         return (m_flags & ECIES_Flags::OLD_COFACTOR_MODE) == ECIES_Flags::OLD_COFACTOR_MODE;
         }

      inline bool check_mode() const
         {
         return (m_flags & ECIES_Flags::CHECK_MODE) == ECIES_Flags::CHECK_MODE;
         }

      inline PointGFp::Compression_Type compression_type() const
         {
         return m_compression_mode;
         }

      const std::string& kdf_spec() const
         {
         return m_kdf_spec;
         }

   private:
      const EC_Group m_domain;
      const std::string m_kdf_spec;
      const size_t m_length;
      const PointGFp::Compression_Type m_compression_mode;
      const ECIES_Flags m_flags;
   };


class BOTAN_DLL ECIES_System_Params : public ECIES_KA_Params
   {
   public:
      /**
      * @param domain ec domain parameters of the involved ec keys
      * @param kdf_spec name of the key derivation function
      * @param dem_algo_spec name of the data encryption method
      * @param dem_key_len length of the key used for the data encryption method
      * @param mac_spec name of the message authentication code
      * @param mac_key_len length of the key used for the message authentication code
      */
      ECIES_System_Params(const EC_Group& domain, const std::string& kdf_spec, const std::string& dem_algo_spec,
                          size_t dem_key_len, const std::string& mac_spec, size_t mac_key_len);

      /**
      * @param domain ec domain parameters of the involved ec keys
      * @param kdf_spec name of the key derivation function
      * @param dem_algo_spec name of the data encryption method
      * @param dem_key_len length of the key used for the data encryption method
      * @param mac_spec name of the message authentication code
      * @param mac_key_len length of the key used for the message authentication code
      * @param compression_type format of encoded keys (affects the secret derivation if single_hash_mode is used)
      * @param flags options, see documentation of ECIES_Flags
      */
      ECIES_System_Params(const EC_Group& domain, const std::string& kdf_spec, const std::string& dem_algo_spec,
                          size_t dem_key_len, const std::string& mac_spec, size_t mac_key_len,
                          PointGFp::Compression_Type compression_type, ECIES_Flags flags);

      virtual ~ECIES_System_Params() = default;

      /// creates an instance of the message authentication code
      std::unique_ptr<MessageAuthenticationCode> create_mac() const;

      /// creates an instance of the data encryption method
      std::unique_ptr<Cipher_Mode> create_cipher(Botan::Cipher_Dir direction) const;

      /// returns the length of the key used by the data encryption method
      inline size_t dem_keylen() const
         {
         return m_dem_keylen;
         }

      /// returns the length of the key used by the message authentication code
      inline size_t mac_keylen() const
         {
         return m_mac_keylen;
         }

   private:
      const std::string m_dem_spec;
      const size_t m_dem_keylen;
      const std::string m_mac_spec;
      const size_t m_mac_keylen;
   };


/**
* ECIES secret derivation according to ISO 18033-2
*/
class BOTAN_DLL ECIES_KA_Operation
   {
   public:
      /**
      * @param private_key the (ephemeral) private key which is used to derive the secret
      * @param ecies_params settings for ecies
      * @param for_encryption disable cofactor mode if the secret will be used for encryption
      * (according to ISO 18033 cofactor mode is only used during decryption)
      * @param rng the RNG to use
      */
      ECIES_KA_Operation(const PK_Key_Agreement_Key& private_key,
                         const ECIES_KA_Params& ecies_params,
                         bool for_encryption,
                         RandomNumberGenerator& rng);

      /**
      * Performs a key agreement with the provided keys and derives the secret from the result
      * @param eph_public_key_bin the encoded (ephemeral) public key which belongs to the used (ephemeral) private key
      * @param other_public_key_point public key point of the other party
      */
      SymmetricKey derive_secret(const std::vector<uint8_t>& eph_public_key_bin,
                                 const PointGFp& other_public_key_point) const;

   private:
      const PK_Key_Agreement m_ka;
      const ECIES_KA_Params m_params;
   };


/**
* ECIES Encryption according to ISO 18033-2
*/
class BOTAN_DLL ECIES_Encryptor : public PK_Encryptor
   {
   public:
      /**
      * @param private_key the (ephemeral) private key which is used for the key agreement
      * @param ecies_params settings for ecies
      * @param rng random generator to use
      */
      ECIES_Encryptor(const PK_Key_Agreement_Key& private_key,
                      const ECIES_System_Params& ecies_params,
                      RandomNumberGenerator& rng);

      /**
      * Creates an ephemeral private key which is used for the key agreement
      * @param rng random generator used during private key generation
      * @param ecies_params settings for ecies
      */
      ECIES_Encryptor(RandomNumberGenerator& rng, const ECIES_System_Params& ecies_params);

      /// Set the public key of the other party
      inline void set_other_key(const Botan::PointGFp& public_point)
         {
         m_other_point = public_point;
         }

      /// Set the initialization vector for the data encryption method
      inline void set_initialization_vector(const InitializationVector& iv)
         {
         m_iv = iv;
         }

      /// Set the label which is appended to the input for the message authentication code
      inline void set_label(const std::string& label)
         {
         m_label = std::vector<uint8_t>(label.begin(), label.end());
         }

   private:
      std::vector<uint8_t> enc(const uint8_t data[], size_t length, RandomNumberGenerator&) const override;

      inline size_t maximum_input_size() const override
         {
         return std::numeric_limits<size_t>::max();
         }

      const ECIES_KA_Operation m_ka;
      const ECIES_System_Params m_params;
      std::vector<uint8_t> m_eph_public_key_bin;
      InitializationVector m_iv;
      PointGFp m_other_point;
      std::vector<uint8_t> m_label;
   };


/**
* ECIES Decryption according to ISO 18033-2
*/
class BOTAN_DLL ECIES_Decryptor : public PK_Decryptor
   {
   public:
      /**
      * @param private_key the private key which is used for the key agreement
      * @param ecies_params settings for ecies
      * @param rng the random generator to use
      */
      ECIES_Decryptor(const PK_Key_Agreement_Key& private_key,
                      const ECIES_System_Params& ecies_params,
                      RandomNumberGenerator& rng);

      /// Set the initialization vector for the data encryption method
      inline void set_initialization_vector(const InitializationVector& iv)
         {
         m_iv = iv;
         }

      /// Set the label which is appended to the input for the message authentication code
      inline void set_label(const std::string& label)
         {
         m_label = std::vector<uint8_t>(label.begin(), label.end());
         }

   private:
      secure_vector<uint8_t> do_decrypt(uint8_t& valid_mask, const uint8_t in[], size_t in_len) const override;

      const ECIES_KA_Operation m_ka;
      const ECIES_System_Params m_params;
      InitializationVector m_iv;
      std::vector<uint8_t> m_label;
   };

}


namespace Botan {

/**
* This class represents ECKCDSA public keys.
*/
class BOTAN_DLL ECKCDSA_PublicKey : public virtual EC_PublicKey
   {
   public:

      /**
      * Construct a public key from a given public point.
      * @param dom_par the domain parameters associated with this key
      * @param public_point the public point defining this key
      */
      ECKCDSA_PublicKey(const EC_Group& dom_par,
                      const PointGFp& public_point) :
         EC_PublicKey(dom_par, public_point) {}

      /**
      * Load a public key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded public key bits
      */
      ECKCDSA_PublicKey(const AlgorithmIdentifier& alg_id,
                      const std::vector<uint8_t>& key_bits) :
         EC_PublicKey(alg_id, key_bits) {}

      /**
      * Get this keys algorithm name.
      * @result this keys algorithm name ("ECGDSA")
      */
      std::string algo_name() const override { return "ECKCDSA"; }

      size_t message_parts() const override { return 2; }

      size_t message_part_size() const override
         { return domain().get_order().bytes(); }

      std::unique_ptr<PK_Ops::Verification>
         create_verification_op(const std::string& params,
                                const std::string& provider) const override;
   protected:
      ECKCDSA_PublicKey() {}
   };

/**
* This class represents ECKCDSA private keys.
*/
class BOTAN_DLL ECKCDSA_PrivateKey : public ECKCDSA_PublicKey,
                                    public EC_PrivateKey
   {
   public:

      /**
      * Load a private key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits PKCS #8 structure
      */
      ECKCDSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                       const secure_vector<uint8_t>& key_bits) :
         EC_PrivateKey(alg_id, key_bits, true) {}

      /**
      * Create a private key.
      * @param rng a random number generator
      * @param domain parameters to used for this key
      * @param x the private key (if zero, generate a new random key)
      */
      ECKCDSA_PrivateKey(RandomNumberGenerator& rng,
                       const EC_Group& domain,
                       const BigInt& x = 0) :
         EC_PrivateKey(rng, domain, x, true) {}

      bool check_key(RandomNumberGenerator& rng, bool) const override;

      std::unique_ptr<PK_Ops::Signature>
         create_signature_op(RandomNumberGenerator& rng,
                             const std::string& params,
                             const std::string& provider) const override;
   };

}


namespace Botan {

/**
* ElGamal Public Key
*/
class BOTAN_DLL ElGamal_PublicKey : public virtual DL_Scheme_PublicKey
   {
   public:
      std::string algo_name() const override { return "ElGamal"; }
      DL_Group::Format group_format() const override { return DL_Group::ANSI_X9_42; }

      /**
      * Load a public key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded public key bits
      */
      ElGamal_PublicKey(const AlgorithmIdentifier& alg_id,
                        const std::vector<uint8_t>& key_bits) :
         DL_Scheme_PublicKey(alg_id, key_bits, DL_Group::ANSI_X9_42)
         {}

      /**
      * Create a public key.
      * @param group the underlying DL group
      * @param y the public value y = g^x mod p
      */
      ElGamal_PublicKey(const DL_Group& group, const BigInt& y);

      std::unique_ptr<PK_Ops::Encryption>
         create_encryption_op(RandomNumberGenerator& rng,
                              const std::string& params,
                              const std::string& provider) const override;

   protected:
      ElGamal_PublicKey() {}
   };

/**
* ElGamal Private Key
*/
class BOTAN_DLL ElGamal_PrivateKey : public ElGamal_PublicKey,
                                     public virtual DL_Scheme_PrivateKey
   {
   public:
      bool check_key(RandomNumberGenerator& rng, bool) const override;

      /**
      * Load a private key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits PKCS #8 structure
      */
      ElGamal_PrivateKey(const AlgorithmIdentifier& alg_id,
                         const secure_vector<uint8_t>& key_bits);

      /**
      * Create a private key.
      * @param rng random number generator to use
      * @param group the group to be used in the key
      * @param priv_key the key's secret value (or if zero, generate a new key)
      */
      ElGamal_PrivateKey(RandomNumberGenerator& rng,
                         const DL_Group& group,
                         const BigInt& priv_key = 0);

      std::unique_ptr<PK_Ops::Decryption>
         create_decryption_op(RandomNumberGenerator& rng,
                              const std::string& params,
                              const std::string& provider) const override;
   };

}


namespace Botan {

/**
* EME from PKCS #1 v1.5
*/
class BOTAN_DLL EME_PKCS1v15 final : public EME
   {
   public:
      size_t maximum_input_size(size_t) const override;
   private:
      secure_vector<uint8_t> pad(const uint8_t[], size_t, size_t,
                             RandomNumberGenerator&) const override;

      secure_vector<uint8_t> unpad(uint8_t& valid_mask,
                                const uint8_t in[],
                                size_t in_len) const override;
   };

}


namespace Botan {

class BOTAN_DLL EME_Raw final : public EME
   {
   public:
      size_t maximum_input_size(size_t i) const override;

      EME_Raw() {}
   private:
      secure_vector<uint8_t> pad(const uint8_t[], size_t, size_t,
                             RandomNumberGenerator&) const override;

      secure_vector<uint8_t> unpad(uint8_t& valid_mask,
                                const uint8_t in[],
                                size_t in_len) const override;
   };

}


namespace Botan {

/**
* EMSA1 from IEEE 1363
* Essentially, sign the hash directly
*/
class BOTAN_DLL EMSA1 : public EMSA
   {
   public:
      /**
      * @param hash the hash function to use
      */
      explicit EMSA1(HashFunction* hash) : m_hash(hash) {}

      EMSA* clone() override;

   protected:
      size_t hash_output_length() const { return m_hash->output_length(); }

      std::unique_ptr<HashFunction> m_hash;

   private:
      void update(const uint8_t[], size_t) override;
      secure_vector<uint8_t> raw_data() override;

      secure_vector<uint8_t> encoding_of(const secure_vector<uint8_t>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<uint8_t>& coded,
                  const secure_vector<uint8_t>& raw,
                  size_t key_bits) override;

   };

}


namespace Botan {

/**
* PKCS #1 v1.5 signature padding
* aka PKCS #1 block type 1
* aka EMSA3 from IEEE 1363
*/
class BOTAN_DLL EMSA_PKCS1v15 final : public EMSA
   {
   public:
      /**
      * @param hash the hash function to use
      */
      explicit EMSA_PKCS1v15(HashFunction* hash);

      EMSA* clone() override { return new EMSA_PKCS1v15(m_hash->clone()); }

      void update(const uint8_t[], size_t) override;

      secure_vector<uint8_t> raw_data() override;

      secure_vector<uint8_t> encoding_of(const secure_vector<uint8_t>&, size_t,
                                     RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<uint8_t>&, const secure_vector<uint8_t>&,
                  size_t) override;
   private:
      std::unique_ptr<HashFunction> m_hash;
      std::vector<uint8_t> m_hash_id;
   };

/**
* EMSA_PKCS1v15_Raw which is EMSA_PKCS1v15 without a hash or digest id
* (which according to QCA docs is "identical to PKCS#11's CKM_RSA_PKCS
* mechanism", something I have not confirmed)
*/
class BOTAN_DLL EMSA_PKCS1v15_Raw final : public EMSA
   {
   public:
      EMSA* clone() override { return new EMSA_PKCS1v15_Raw(); }

      void update(const uint8_t[], size_t) override;

      secure_vector<uint8_t> raw_data() override;

      secure_vector<uint8_t> encoding_of(const secure_vector<uint8_t>&, size_t,
                                     RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<uint8_t>&, const secure_vector<uint8_t>&,
                  size_t) override;

   private:
      secure_vector<uint8_t> m_message;
   };

}


namespace Botan {

/**
* EMSA-Raw - sign inputs directly
* Don't use this unless you know what you are doing.
*/
class BOTAN_DLL EMSA_Raw final : public EMSA
   {
   public:
      EMSA* clone() override { return new EMSA_Raw(); }

   private:
      void update(const uint8_t[], size_t) override;
      secure_vector<uint8_t> raw_data() override;

      secure_vector<uint8_t> encoding_of(const secure_vector<uint8_t>&, size_t,
                                     RandomNumberGenerator&) override;
      bool verify(const secure_vector<uint8_t>&, const secure_vector<uint8_t>&,
                  size_t) override;

      secure_vector<uint8_t> m_message;
   };

}


namespace Botan {

/**
* EMSA from X9.31 (EMSA2 in IEEE 1363)
* Useful for Rabin-Williams, also sometimes used with RSA in
* odd protocols.
*/
class BOTAN_DLL EMSA_X931 final : public EMSA
   {
   public:
      /**
      * @param hash the hash function to use
      */
      explicit EMSA_X931(HashFunction* hash);

      EMSA* clone() override { return new EMSA_X931(m_hash->clone()); }
   private:
      void update(const uint8_t[], size_t) override;
      secure_vector<uint8_t> raw_data() override;

      secure_vector<uint8_t> encoding_of(const secure_vector<uint8_t>&, size_t,
                                     RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<uint8_t>&, const secure_vector<uint8_t>&,
                  size_t) override;

      secure_vector<uint8_t> m_empty_hash;
      std::unique_ptr<HashFunction> m_hash;
      uint8_t m_hash_id;
   };

}

#ifdef __cplusplus
extern "C" {
#endif

/*
This header exports some of botan's functionality via a C89
interface. This API is uesd by the Python and OCaml bindings via those
languages respective ctypes libraries.

The API is intended to be as easy as possible to call from other
languages, which often have easy ways to call C, because C. But some C
code is easier to deal with than others, so to make things easy this
API follows a few simple rules:

- All interactions are via pointers to opaque structs. No need to worry about
  structure padding issues and the like.

- All functions return an int error code (except the version calls, which are
  assumed to always have something to say).

- Use simple types: size_t for lengths, const char* NULL terminated strings,
  uint8_t for binary.

- No ownership of memory transfers across the API boundary. The API will
  consume data from const pointers, and will produce output by writing to
  variables provided by the caller.

- If exporting a value (a string or a blob) the function takes a pointer to the
  output array and a read/write pointer to the length. If the length is insufficient, an
  error is returned. So passing nullptr/0 allows querying the final value.

  Note this does not apply to all functions, like `botan_hash_final`
  which is not idempotent and are documented specially. But it's a
  general theory of operation.

The API is not currently documented, nor should it be considered
stable. It is buggy as heck, most likely, and error handling is a
mess. However the goal is to provide a long term API usable for
language bindings, or for use by systems written in C. Suggestions on
how to provide the cleanest API for such users would be most welcome.

* TODO:
* - Better error reporting
* - User callback for exception logging?
* - Doxygen comments for all functions/params
* - X.509 certs and PKIX path validation goo
* - TLS
*/

#include <stdint.h>

/*
* Versioning
*/
BOTAN_DLL uint32_t botan_ffi_api_version();

/*
* Return 0 (ok) if the version given is one this library supports.
* botan_ffi_supports_api(botan_ffi_api_version()) will always return 0.
*/
BOTAN_DLL int botan_ffi_supports_api(uint32_t api_version);

BOTAN_DLL const char* botan_version_string();
BOTAN_DLL uint32_t botan_version_major();
BOTAN_DLL uint32_t botan_version_minor();
BOTAN_DLL uint32_t botan_version_patch();
BOTAN_DLL uint32_t botan_version_datestamp();

/*
* Error handling
*
* Some way of exporting these values to other languages would be useful


 THIS FUNCTION ASSUMES BOTH ARGUMENTS ARE LITERAL STRINGS
 so it retains only the pointers and does not make a copy.

int botan_make_error(const char* msg, const char* func, int line);
* This value is returned to callers ^^

 normally called like
   return botan_make_error(BOTAN_ERROR_STRING_NOT_IMPLEMENTED, BOTAN_FUNCTION, __LINE__);

// This would seem to require both saving the message permanently
catch(std::exception& e) {
return botan_make_error_from_transient_string(e.what(), BOTAN_FUNCTION, __LINE__);
}

#define botan_make_error_inf(s) return botan_make_error(s, BOTAN_FUNCTION, __LINE__);

Easier to return a const char* from each function directly? However,

catch(std::exception& e) { return e.what(); }

doesn't exactly work well either!

*
* Later call:
* const char* botan_get_error_str(int);
* To recover the msg, func, and line

*/
#define BOTAN_FFI_ERROR_INSUFFICIENT_BUFFER_SPACE (-10)
#define BOTAN_FFI_ERROR_EXCEPTION_THROWN (-20)
#define BOTAN_FFI_ERROR_BAD_FLAG (-30)
#define BOTAN_FFI_ERROR_NULL_POINTER (-31)
#define BOTAN_FFI_ERROR_NOT_IMPLEMENTED (-40)

//const char* botan_error_description(int err);

/*
* Returns 0 if x[0..len] == y[0..len], or otherwise -1
*/
BOTAN_DLL int botan_same_mem(const uint8_t* x, const uint8_t* y, size_t len);

#define BOTAN_FFI_HEX_LOWER_CASE 1

BOTAN_DLL int botan_hex_encode(const uint8_t* x, size_t len, char* out, uint32_t flags);
// TODO: botan_hex_decode
// TODO: botan_base64_encode
// TODO: botan_base64_decode

/*
* RNG
*/
typedef struct botan_rng_struct* botan_rng_t;

/**
* TODO: replace rng_type with simple flags? 
*/
BOTAN_DLL int botan_rng_init(botan_rng_t* rng, const char* rng_type);

/**
* TODO: better name
*/
BOTAN_DLL int botan_rng_get(botan_rng_t rng, uint8_t* out, size_t out_len);
BOTAN_DLL int botan_rng_reseed(botan_rng_t rng, size_t bits);
BOTAN_DLL int botan_rng_destroy(botan_rng_t rng);

/*
* Hashing
*/
typedef struct botan_hash_struct* botan_hash_t;

/**
* Initialize a hash object:
*   botan_hash_t hash
*   botan_hash_init(&hash, "SHA-384", 0);
*
* Flags should be 0 in current API revision, all other uses are reserved.
*  and return BOTAN_FFI_ERROR_BAD_FLAG.

* TODO: since output_length is effectively required to use this API,
* return it from init as an output parameter
*/
BOTAN_DLL int botan_hash_init(botan_hash_t* hash, const char* hash_name, uint32_t flags);

/**
* Writes the output length of the hash object to *output_length
*/
BOTAN_DLL int botan_hash_output_length(botan_hash_t hash, size_t* output_length);

/**
* Send more input to the hash function.
*/
BOTAN_DLL int botan_hash_update(botan_hash_t hash, const uint8_t* in, size_t in_len);

/**
* Finalizes the hash computation and writes the output to
* out[0:botan_hash_output_length()] then reinitializes for computing
* another digest as if botan_hash_clear had been called.
*/
BOTAN_DLL int botan_hash_final(botan_hash_t hash, uint8_t out[]);

/**
* Reinitializes the state of the hash computation. A hash can
* be computed (with update/final) immediately.
*/
BOTAN_DLL int botan_hash_clear(botan_hash_t hash);

/**
* Frees all resources of this object
*/
BOTAN_DLL int botan_hash_destroy(botan_hash_t hash);

BOTAN_DLL int botan_hash_name(botan_hash_t hash, char* name, size_t name_len);

/*
* Message Authentication
*/
typedef struct botan_mac_struct* botan_mac_t;

BOTAN_DLL int botan_mac_init(botan_mac_t* mac, const char* mac_name, uint32_t flags);
BOTAN_DLL int botan_mac_output_length(botan_mac_t mac, size_t* output_length);
BOTAN_DLL int botan_mac_set_key(botan_mac_t mac, const uint8_t* key, size_t key_len);
BOTAN_DLL int botan_mac_update(botan_mac_t mac, const uint8_t* buf, size_t len);
BOTAN_DLL int botan_mac_final(botan_mac_t mac, uint8_t out[]);
BOTAN_DLL int botan_mac_clear(botan_mac_t hash);
BOTAN_DLL int botan_mac_destroy(botan_mac_t mac);

/*
* Cipher modes
*/
typedef struct botan_cipher_struct* botan_cipher_t;

#define BOTAN_CIPHER_INIT_FLAG_MASK_DIRECTION 1
#define BOTAN_CIPHER_INIT_FLAG_ENCRYPT 0
#define BOTAN_CIPHER_INIT_FLAG_DECRYPT 1

BOTAN_DLL int botan_cipher_init(botan_cipher_t* cipher, const char* name, uint32_t flags);

BOTAN_DLL int botan_cipher_valid_nonce_length(botan_cipher_t cipher, size_t nl);
BOTAN_DLL int botan_cipher_get_tag_length(botan_cipher_t cipher, size_t* tag_size);
BOTAN_DLL int botan_cipher_get_default_nonce_length(botan_cipher_t cipher, size_t* nl);
BOTAN_DLL int botan_cipher_get_update_granularity(botan_cipher_t cipher, size_t* ug);

BOTAN_DLL int botan_cipher_query_keylen(botan_cipher_t,
                                        size_t* out_minimum_keylength,
                                        size_t* out_maximum_keylength);

BOTAN_DLL int botan_cipher_set_key(botan_cipher_t cipher,
                                   const uint8_t* key, size_t key_len);

BOTAN_DLL int botan_cipher_set_associated_data(botan_cipher_t cipher,
                                               const uint8_t* ad, size_t ad_len);

BOTAN_DLL int botan_cipher_start(botan_cipher_t cipher,
                                 const uint8_t* nonce, size_t nonce_len);

#define BOTAN_CIPHER_UPDATE_FLAG_FINAL (1U << 0)

BOTAN_DLL int botan_cipher_update(botan_cipher_t cipher,
                                  uint32_t flags,
                                  uint8_t output[],
                                  size_t output_size,
                                  size_t* output_written,
                                  const uint8_t input_bytes[],
                                  size_t input_size,
                                  size_t* input_consumed);

BOTAN_DLL int botan_cipher_clear(botan_cipher_t hash);
BOTAN_DLL int botan_cipher_destroy(botan_cipher_t cipher);

/*
* PBKDF
*/
BOTAN_DLL int botan_pbkdf(const char* pbkdf_algo,
                          uint8_t out[], size_t out_len,
                          const char* password,
                          const uint8_t salt[], size_t salt_len,
                          size_t iterations);

BOTAN_DLL int botan_pbkdf_timed(const char* pbkdf_algo,
                                uint8_t out[], size_t out_len,
                                const char* password,
                                const uint8_t salt[], size_t salt_len,
                                size_t milliseconds_to_run,
                                size_t* out_iterations_used);

/*
* KDF
*/
BOTAN_DLL int botan_kdf(const char* kdf_algo,
                        uint8_t out[], size_t out_len,
                        const uint8_t secret[], size_t secret_len,
                        const uint8_t salt[], size_t salt_len,
                        const uint8_t label[], size_t label_len);

/*
* Bcrypt
* *out_len should be 64 bytes
* Output is formatted bcrypt $2a$...
*/
BOTAN_DLL int botan_bcrypt_generate(uint8_t* out, size_t* out_len,
                                    const char* password,
                                    botan_rng_t rng,
                                    size_t work_factor,
                                    uint32_t flags);

/**
* Returns 0 if if this password/hash combination is valid
* Returns 1 if the combination is not valid (but otherwise well formed)
* Returns negative on error
*/
BOTAN_DLL int botan_bcrypt_is_valid(const char* pass, const char* hash);

/*
* Public/private key creation, import, ...
*/
typedef struct botan_privkey_struct* botan_privkey_t;

BOTAN_DLL int botan_privkey_create(botan_privkey_t* key,
                                   const char* algo_name,
                                   const char* algo_params,
                                   botan_rng_t rng);

BOTAN_DLL int botan_privkey_create_rsa(botan_privkey_t* key, botan_rng_t rng, size_t n_bits);
BOTAN_DLL int botan_privkey_create_ecdsa(botan_privkey_t* key, botan_rng_t rng, const char* params);
BOTAN_DLL int botan_privkey_create_ecdh(botan_privkey_t* key, botan_rng_t rng, const char* params);
BOTAN_DLL int botan_privkey_create_mceliece(botan_privkey_t* key, botan_rng_t rng, size_t n, size_t t);


/*
* Input currently assumed to be PKCS #8 structure;
* Set password to NULL to indicate no encryption expected
*/
BOTAN_DLL int botan_privkey_load(botan_privkey_t* key, botan_rng_t rng,
                                 const uint8_t bits[], size_t len,
                                 const char* password);

BOTAN_DLL int botan_privkey_destroy(botan_privkey_t key);

#define BOTAN_PRIVKEY_EXPORT_FLAG_DER 0
#define BOTAN_PRIVKEY_EXPORT_FLAG_PEM 1

/*
* On input *out_len is number of bytes in out[]
* On output *out_len is number of bytes written (or required)
* If out is not big enough no output is written, *out_len is set and 1 is returned
* Returns 0 on success and sets
* If some other error occurs a negative integer is returned.
*/
BOTAN_DLL int botan_privkey_export(botan_privkey_t key,
                                   uint8_t out[], size_t* out_len,
                                   uint32_t flags);

/*
* Set encryption_algo to NULL or "" to have the library choose a default (recommended)
*/
BOTAN_DLL int botan_privkey_export_encrypted(botan_privkey_t key,
                                             uint8_t out[], size_t* out_len,
                                             botan_rng_t rng,
                                             const char* passphrase,
                                             const char* encryption_algo,
                                             uint32_t flags);

typedef struct botan_pubkey_struct* botan_pubkey_t;

BOTAN_DLL int botan_pubkey_load(botan_pubkey_t* key, const uint8_t bits[], size_t len);

BOTAN_DLL int botan_privkey_export_pubkey(botan_pubkey_t* out, botan_privkey_t in);

BOTAN_DLL int botan_pubkey_export(botan_pubkey_t key, uint8_t out[], size_t* out_len, uint32_t flags);

BOTAN_DLL int botan_pubkey_algo_name(botan_pubkey_t key, char out[], size_t* out_len);

BOTAN_DLL int botan_pubkey_estimated_strength(botan_pubkey_t key, size_t* estimate);

BOTAN_DLL int botan_pubkey_fingerprint(botan_pubkey_t key, const char* hash,
                                       uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_pubkey_destroy(botan_pubkey_t key);


/*
* Public Key Encryption
*/
typedef struct botan_pk_op_encrypt_struct* botan_pk_op_encrypt_t;

BOTAN_DLL int botan_pk_op_encrypt_create(botan_pk_op_encrypt_t* op,
                                         botan_pubkey_t key,
                                         const char* padding,
                                         uint32_t flags);

BOTAN_DLL int botan_pk_op_encrypt_destroy(botan_pk_op_encrypt_t op);

BOTAN_DLL int botan_pk_op_encrypt(botan_pk_op_encrypt_t op,
                                  botan_rng_t rng,
                                  uint8_t out[], size_t* out_len,
                                  const uint8_t plaintext[], size_t plaintext_len);

/*
* Public Key Decryption
*/
typedef struct botan_pk_op_decrypt_struct* botan_pk_op_decrypt_t;

BOTAN_DLL int botan_pk_op_decrypt_create(botan_pk_op_decrypt_t* op,
                                         botan_privkey_t key,
                                         const char* padding,
                                         uint32_t flags);
BOTAN_DLL int botan_pk_op_decrypt_destroy(botan_pk_op_decrypt_t op);

BOTAN_DLL int botan_pk_op_decrypt(botan_pk_op_decrypt_t op,
                                  uint8_t out[], size_t* out_len,
                                  uint8_t ciphertext[], size_t ciphertext_len);

/*
* Signature Generation
*/
typedef struct botan_pk_op_sign_struct* botan_pk_op_sign_t;

BOTAN_DLL int botan_pk_op_sign_create(botan_pk_op_sign_t* op,
                                      botan_privkey_t key,
                                      const char* hash_and_padding,
                                      uint32_t flags);
BOTAN_DLL int botan_pk_op_sign_destroy(botan_pk_op_sign_t op);

BOTAN_DLL int botan_pk_op_sign_update(botan_pk_op_sign_t op, const uint8_t in[], size_t in_len);
BOTAN_DLL int botan_pk_op_sign_finish(botan_pk_op_sign_t op, botan_rng_t rng,
                                      uint8_t sig[], size_t* sig_len);

/*
* Signature Verification
*/
typedef struct botan_pk_op_verify_struct* botan_pk_op_verify_t;

BOTAN_DLL int botan_pk_op_verify_create(botan_pk_op_verify_t* op,
                                        botan_pubkey_t key,
                                        const char* hash_and_padding,
                                        uint32_t flags);
BOTAN_DLL int botan_pk_op_verify_destroy(botan_pk_op_verify_t op);

BOTAN_DLL int botan_pk_op_verify_update(botan_pk_op_verify_t op, const uint8_t in[], size_t in_len);
BOTAN_DLL int botan_pk_op_verify_finish(botan_pk_op_verify_t op, const uint8_t sig[], size_t sig_len);

/*
* Key Agreement
*/
typedef struct botan_pk_op_ka_struct* botan_pk_op_ka_t;

BOTAN_DLL int botan_pk_op_key_agreement_create(botan_pk_op_ka_t* op,
                                               botan_privkey_t key,
                                               const char* kdf,
                                               uint32_t flags);
BOTAN_DLL int botan_pk_op_key_agreement_destroy(botan_pk_op_ka_t op);

BOTAN_DLL int botan_pk_op_key_agreement_export_public(botan_privkey_t key,
                                                      uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_pk_op_key_agreement(botan_pk_op_ka_t op,
                                        uint8_t out[], size_t* out_len,
                                        const uint8_t other_key[], size_t other_key_len,
                                        const uint8_t salt[], size_t salt_len);


/*
*
* @param mce_key must be a McEliece key
* ct_len should be pt_len + n/8 + a few?
*/
BOTAN_DLL int botan_mceies_encrypt(botan_pubkey_t mce_key,
                                   botan_rng_t rng,
                                   const char* aead,
                                   const uint8_t pt[], size_t pt_len,
                                   const uint8_t ad[], size_t ad_len,
                                   uint8_t ct[], size_t* ct_len);

BOTAN_DLL int botan_mceies_decrypt(botan_privkey_t mce_key,
                                   const char* aead,
                                   const uint8_t ct[], size_t ct_len,
                                   const uint8_t ad[], size_t ad_len,
                                   uint8_t pt[], size_t* pt_len);



typedef struct botan_x509_cert_struct* botan_x509_cert_t;
BOTAN_DLL int botan_x509_cert_load(botan_x509_cert_t* cert_obj, const uint8_t cert[], size_t cert_len);
BOTAN_DLL int botan_x509_cert_load_file(botan_x509_cert_t* cert_obj, const char* filename);
BOTAN_DLL int botan_x509_cert_destroy(botan_x509_cert_t cert);

BOTAN_DLL int botan_x509_cert_gen_selfsigned(botan_x509_cert_t* cert,
                                             botan_privkey_t key,
                                             botan_rng_t rng,
                                             const char* common_name,
                                             const char* org_name);

// TODO: return botan_time_struct instead
BOTAN_DLL int botan_x509_cert_get_time_starts(botan_x509_cert_t cert, char out[], size_t* out_len);
BOTAN_DLL int botan_x509_cert_get_time_expires(botan_x509_cert_t cert, char out[], size_t* out_len);

BOTAN_DLL int botan_x509_cert_get_fingerprint(botan_x509_cert_t cert, const char* hash, uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_x509_cert_get_serial_number(botan_x509_cert_t cert, uint8_t out[], size_t* out_len);
BOTAN_DLL int botan_x509_cert_get_authority_key_id(botan_x509_cert_t cert, uint8_t out[], size_t* out_len);
BOTAN_DLL int botan_x509_cert_get_subject_key_id(botan_x509_cert_t cert, uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_x509_cert_path_verify(botan_x509_cert_t cert,
                                          const char* ca_dir);

BOTAN_DLL int botan_x509_cert_get_public_key_bits(botan_x509_cert_t cert,
                                                  uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_x509_cert_get_public_key(botan_x509_cert_t cert, botan_pubkey_t* key);

BOTAN_DLL int botan_x509_cert_get_issuer_dn(botan_x509_cert_t cert,
                                            const char* key, size_t index,
                                            uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_x509_cert_get_subject_dn(botan_x509_cert_t cert,
                                             const char* key, size_t index,
                                             uint8_t out[], size_t* out_len);

BOTAN_DLL int botan_x509_cert_to_string(botan_x509_cert_t cert, char out[], size_t* out_len);

// Must match values of Key_Constraints in key_constraints.h
enum botan_x509_cert_key_constraints {
   NO_CONSTRAINTS     = 0,
   DIGITAL_SIGNATURE  = 32768,
   NON_REPUDIATION    = 16384,
   KEY_ENCIPHERMENT   = 8192,
   DATA_ENCIPHERMENT  = 4096,
   KEY_AGREEMENT      = 2048,
   KEY_CERT_SIGN      = 1024,
   CRL_SIGN           = 512,
   ENCIPHER_ONLY      = 256,
   DECIPHER_ONLY      = 128
};

BOTAN_DLL int botan_x509_cert_allowed_usage(botan_x509_cert_t cert, unsigned int key_usage);

/*
* TLS (WIP)
*/
#if defined(BOTAN_HAS_TLS) && 0

typedef struct botan_tls_session_struct* botan_tls_session_t;

BOTAN_DLL int botan_tls_session_decrypt(botan_tls_session_t* session,
                                        const uint8_t key[], size_t key_len,
                                        const uint8_t blob[], size_t blob_len);

BOTAN_DLL int botan_tls_session_get_version(botan_tls_session_t session, uint16_t* tls_version);
BOTAN_DLL int botan_tls_session_get_ciphersuite(botan_tls_session_t session, uint16_t* ciphersuite);
BOTAN_DLL int botan_tls_session_encrypt(botan_tls_session_t session, botan_rng_t rng, uint8_t key[], size_t* key_len);

BOTAN_DLL int botan_tls_session_get_peer_certs(botan_tls_session_t session, botan_x509_cert_t certs[], size_t* cert_len);

// TODO: peer certs, validation, ...

typedef struct botan_tls_channel_struct* botan_tls_channel_t;

typedef void (*botan_tls_channel_output_fn)(void* application_data, const uint8_t* data, size_t data_len);

typedef void (*botan_tls_channel_data_cb)(void* application_data, const uint8_t* data, size_t data_len);

typedef void (*botan_tls_channel_alert_cb)(void* application_data, uint16_t alert_code);

typedef void (*botan_tls_channel_session_established)(void* application_data,
                                                      botan_tls_channel_t channel,
                                                      botan_tls_session_t session);

BOTAN_DLL int botan_tls_channel_init_client(botan_tls_channel_t* channel,
                                            botan_tls_channel_output_fn output_fn,
                                            botan_tls_channel_data_cb data_cb,
                                            botan_tls_channel_alert_cb alert_cb,
                                            botan_tls_channel_session_established session_cb,
                                            const char* server_name);

BOTAN_DLL int botan_tls_channel_init_server(botan_tls_channel_t* channel,
                                            botan_tls_channel_output_fn output_fn,
                                            botan_tls_channel_data_cb data_cb,
                                            botan_tls_channel_alert_cb alert_cb,
                                            botan_tls_channel_session_established session_cb);

BOTAN_DLL int botan_tls_channel_received_data(botan_tls_channel_t chan,
                                              const uint8_t input[], size_t len);

/**
* Returns 0 for client, 1 for server, negative for error
*/
BOTAN_DLL int botan_tls_channel_type(botan_tls_channel_t chan);

BOTAN_DLL int botan_tls_channel_send(botan_tls_channel_t chan,
                                     const uint8_t input[], size_t len);

BOTAN_DLL int botan_tls_channel_close(botan_tls_channel_t chan);

BOTAN_DLL int botan_tls_channel_destroy(botan_tls_channel_t chan);

#endif
#ifdef __cplusplus
}
#endif



namespace Botan {

/**
* This class represents pipe objects.
* A set of filters can be placed into a pipe, and information flows
* through the pipe until it reaches the end, where the output is
* collected for retrieval.  If you're familiar with the Unix shell
* environment, this design will sound quite familiar.
*/
class BOTAN_DLL Pipe final : public DataSource
   {
   public:
      /**
      * An opaque type that identifies a message in this Pipe
      */
      typedef size_t message_id;

      /**
      * Exception if you use an invalid message as an argument to
      * read, remaining, etc
      */
      struct BOTAN_DLL Invalid_Message_Number : public Invalid_Argument
         {
         /**
         * @param where the error occurred
         * @param msg the invalid message id that was used
         */
         Invalid_Message_Number(const std::string& where, message_id msg) :
            Invalid_Argument("Pipe::" + where + ": Invalid message number " +
                             std::to_string(msg))
            {}
         };

      /**
      * A meta-id for whatever the last message is
      */
      static const message_id LAST_MESSAGE;

      /**
      * A meta-id for the default message (set with set_default_msg)
      */
      static const message_id DEFAULT_MESSAGE;

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the byte array to write
      * @param length the length of the byte array in
      */
      void write(const uint8_t in[], size_t length);

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the secure_vector containing the data to write
      */
      void write(const secure_vector<uint8_t>& in)
         { write(in.data(), in.size()); }

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the std::vector containing the data to write
      */
      void write(const std::vector<uint8_t>& in)
         { write(in.data(), in.size()); }

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the string containing the data to write
      */
      void write(const std::string& in);

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in the DataSource to read the data from
      */
      void write(DataSource& in);

      /**
      * Write input to the pipe, i.e. to its first filter.
      * @param in a single byte to be written
      */
      void write(uint8_t in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the byte array containing the data to write
      * @param length the length of the byte array to write
      */
      void process_msg(const uint8_t in[], size_t length);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the secure_vector containing the data to write
      */
      void process_msg(const secure_vector<uint8_t>& in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the secure_vector containing the data to write
      */
      void process_msg(const std::vector<uint8_t>& in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the string containing the data to write
      */
      void process_msg(const std::string& in);

      /**
      * Perform start_msg(), write() and end_msg() sequentially.
      * @param in the DataSource providing the data to write
      */
      void process_msg(DataSource& in);

      /**
      * Find out how many bytes are ready to read.
      * @param msg the number identifying the message
      * for which the information is desired
      * @return number of bytes that can still be read
      */
      size_t remaining(message_id msg = DEFAULT_MESSAGE) const BOTAN_WARN_UNUSED_RESULT;

      /**
      * Read the default message from the pipe. Moves the internal
      * offset so that every call to read will return a new portion of
      * the message.
      *
      * @param output the byte array to write the read bytes to
      * @param length the length of the byte array output
      * @return number of bytes actually read into output
      */
      size_t read(uint8_t output[], size_t length) override BOTAN_WARN_UNUSED_RESULT;

      /**
      * Read a specified message from the pipe. Moves the internal
      * offset so that every call to read will return a new portion of
      * the message.
      * @param output the byte array to write the read bytes to
      * @param length the length of the byte array output
      * @param msg the number identifying the message to read from
      * @return number of bytes actually read into output
      */
      size_t read(uint8_t output[], size_t length, message_id msg) BOTAN_WARN_UNUSED_RESULT;

      /**
      * Read a single byte from the pipe. Moves the internal offset so
      * that every call to read will return a new portion of the
      * message.
      *
      * @param output the byte to write the result to
      * @param msg the message to read from
      * @return number of bytes actually read into output
      */
      size_t read(uint8_t& output, message_id msg = DEFAULT_MESSAGE) BOTAN_WARN_UNUSED_RESULT;

      /**
      * Read the full contents of the pipe.
      * @param msg the number identifying the message to read from
      * @return secure_vector holding the contents of the pipe
      */
      secure_vector<uint8_t> read_all(message_id msg = DEFAULT_MESSAGE) BOTAN_WARN_UNUSED_RESULT;

      /**
      * Read the full contents of the pipe.
      * @param msg the number identifying the message to read from
      * @return string holding the contents of the pipe
      */
      std::string read_all_as_string(message_id = DEFAULT_MESSAGE) BOTAN_WARN_UNUSED_RESULT;

      /**
      * Read from the default message but do not modify the internal
      * offset. Consecutive calls to peek() will return portions of
      * the message starting at the same position.
      * @param output the byte array to write the peeked message part to
      * @param length the length of the byte array output
      * @param offset the offset from the current position in message
      * @return number of bytes actually peeked and written into output
      */
      size_t peek(uint8_t output[], size_t length, size_t offset) const override BOTAN_WARN_UNUSED_RESULT;

      /** Read from the specified message but do not modify the
      * internal offset. Consecutive calls to peek() will return
      * portions of the message starting at the same position.
      * @param output the byte array to write the peeked message part to
      * @param length the length of the byte array output
      * @param offset the offset from the current position in message
      * @param msg the number identifying the message to peek from
      * @return number of bytes actually peeked and written into output
      */
      size_t peek(uint8_t output[], size_t length,
                  size_t offset, message_id msg) const BOTAN_WARN_UNUSED_RESULT;

      /** Read a single byte from the specified message but do not
      * modify the internal offset. Consecutive calls to peek() will
      * return portions of the message starting at the same position.
      * @param output the byte to write the peeked message byte to
      * @param offset the offset from the current position in message
      * @param msg the number identifying the message to peek from
      * @return number of bytes actually peeked and written into output
      */
      size_t peek(uint8_t& output, size_t offset,
                  message_id msg = DEFAULT_MESSAGE) const BOTAN_WARN_UNUSED_RESULT;

      /**
      * @return the number of bytes read from the default message.
      */
      size_t get_bytes_read() const override;

      /**
      * @return the number of bytes read from the specified message.
      */
      size_t get_bytes_read(message_id msg) const;

      bool check_available(size_t n) override;
      bool check_available_msg(size_t n, message_id msg);

      /**
      * @return currently set default message
      */
      size_t default_msg() const { return m_default_read; }

      /**
      * Set the default message
      * @param msg the number identifying the message which is going to
      * be the new default message
      */
      void set_default_msg(message_id msg);

      /**
      * Get the number of messages the are in this pipe.
      * @return number of messages the are in this pipe
      */
      message_id message_count() const;

      /**
      * Test whether this pipe has any data that can be read from.
      * @return true if there is more data to read, false otherwise
      */
      bool end_of_data() const override;

      /**
      * Start a new message in the pipe. A potential other message in this pipe
      * must be closed with end_msg() before this function may be called.
      */
      void start_msg();

      /**
      * End the current message.
      */
      void end_msg();

      /**
      * Insert a new filter at the front of the pipe
      * @param filt the new filter to insert
      */
      void prepend(Filter* filt);

      /**
      * Insert a new filter at the back of the pipe
      * @param filt the new filter to insert
      */
      void append(Filter* filt);

      /**
      * Remove the first filter at the front of the pipe.
      */
      void pop();

      /**
      * Reset this pipe to an empty pipe.
      */
      void reset();

      /**
      * Construct a Pipe of up to four filters. The filters are set up
      * in the same order as the arguments.
      */
      Pipe(Filter* = nullptr, Filter* = nullptr,
           Filter* = nullptr, Filter* = nullptr);

      /**
      * Construct a Pipe from a list of filters
      * @param filters the set of filters to use
      */
      explicit Pipe(std::initializer_list<Filter*> filters);

      Pipe(const Pipe&) = delete;
      Pipe& operator=(const Pipe&) = delete;

      ~Pipe();
   private:
      void init();
      void destruct(Filter*);
      void find_endpoints(Filter*);
      void clear_endpoints(Filter*);

      message_id get_message_no(const std::string&, message_id) const;

      Filter* m_pipe;
      class Output_Buffers* m_outputs;
      message_id m_default_read;
      bool m_inside_msg;
   };

/**
* Stream output operator; dumps the results from pipe's default
* message to the output stream.
* @param out an output stream
* @param pipe the pipe
*/
BOTAN_DLL std::ostream& operator<<(std::ostream& out, Pipe& pipe);

/**
* Stream input operator; dumps the remaining bytes of input
* to the (assumed open) pipe message.
* @param in the input stream
* @param pipe the pipe
*/
BOTAN_DLL std::istream& operator>>(std::istream& in, Pipe& pipe);

}

#if defined(BOTAN_HAS_PIPE_UNIXFD_IO)
#endif


#if defined(BOTAN_HAS_CODEC_FILTERS)

namespace Botan {

/**
* Converts arbitrary binary data to hex strings, optionally with
* newlines inserted
*/
class BOTAN_DLL Hex_Encoder final : public Filter
   {
   public:
      /**
      * Whether to use uppercase or lowercase letters for the encoded string.
      */
      enum Case { Uppercase, Lowercase };

      std::string name() const override { return "Hex_Encoder"; }

      void write(const uint8_t in[], size_t length) override;
      void end_msg() override;

      /**
      * Create a hex encoder.
      * @param the_case the case to use in the encoded strings.
      */
      explicit Hex_Encoder(Case the_case);

      /**
      * Create a hex encoder.
      * @param newlines should newlines be used
      * @param line_length if newlines are used, how long are lines
      * @param the_case the case to use in the encoded strings
      */
      Hex_Encoder(bool newlines = false,
                  size_t line_length = 72,
                  Case the_case = Uppercase);
   private:
      void encode_and_send(const uint8_t[], size_t);

      const Case m_casing;
      const size_t m_line_length;
      std::vector<uint8_t> m_in, m_out;
      size_t m_position, m_counter;
   };

/**
* Converts hex strings to bytes
*/
class BOTAN_DLL Hex_Decoder final : public Filter
   {
   public:
      std::string name() const override { return "Hex_Decoder"; }

      void write(const uint8_t[], size_t) override;
      void end_msg() override;

      /**
      * Construct a Hex Decoder using the specified
      * character checking.
      * @param checking the checking to use during decoding.
      */
      explicit Hex_Decoder(Decoder_Checking checking = NONE);
   private:
      const Decoder_Checking m_checking;
      std::vector<uint8_t> m_in, m_out;
      size_t m_position;
   };

}

#endif

namespace Botan {

/**
* Stream Cipher Filter
*/
class BOTAN_DLL StreamCipher_Filter : public Keyed_Filter
   {
   public:

      std::string name() const override { return m_cipher->name(); }

      /**
      * Write input data
      * @param input data
      * @param input_len length of input in bytes
      */
      void write(const uint8_t input[], size_t input_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return m_cipher->valid_iv_length(iv_len); }

      /**
      * Set the initialization vector for this filter.
      * @param iv the initialization vector to set
      */
      void set_iv(const InitializationVector& iv) override
         {
         m_cipher->set_iv(iv.begin(), iv.length());
         }

      /**
      * Set the key of this filter.
      * @param key the key to set
      */
      void set_key(const SymmetricKey& key) override { m_cipher->set_key(key); }

      Key_Length_Specification key_spec() const override { return m_cipher->key_spec(); }

      /**
      * Construct a stream cipher filter.
      * @param cipher a cipher object to use
      */
      explicit StreamCipher_Filter(StreamCipher* cipher);

      /**
      * Construct a stream cipher filter.
      * @param cipher a cipher object to use
      * @param key the key to use inside this filter
      */
      StreamCipher_Filter(StreamCipher* cipher, const SymmetricKey& key);

      /**
      * Construct a stream cipher filter.
      * @param cipher the name of the desired cipher
      */
      explicit StreamCipher_Filter(const std::string& cipher);

      /**
      * Construct a stream cipher filter.
      * @param cipher the name of the desired cipher
      * @param key the key to use inside this filter
      */
      StreamCipher_Filter(const std::string& cipher, const SymmetricKey& key);
   private:
      secure_vector<uint8_t> m_buffer;
      std::unique_ptr<StreamCipher> m_cipher;
   };

/**
* Hash Filter.
*/
class BOTAN_DLL Hash_Filter : public Filter
   {
   public:
      void write(const uint8_t input[], size_t len) override { m_hash->update(input, len); }
      void end_msg() override;

      std::string name() const override { return m_hash->name(); }

      /**
      * Construct a hash filter.
      * @param hash the hash function to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the hashfunction
      * hash. Otherwise, specify a smaller value here so that the
      * output of the hash algorithm will be cut off.
      */
      Hash_Filter(HashFunction* hash, size_t len = 0) :
         m_hash(hash), m_out_len(len) {}

      /**
      * Construct a hash filter.
      * @param request the name of the hash algorithm to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the hashfunction
      * hash. Otherwise, specify a smaller value here so that the
      * output of the hash algorithm will be cut off.
      */
      Hash_Filter(const std::string& request, size_t len = 0);

   private:
      std::unique_ptr<HashFunction> m_hash;
      const size_t m_out_len;
   };

/**
* MessageAuthenticationCode Filter.
*/
class BOTAN_DLL MAC_Filter : public Keyed_Filter
   {
   public:
      void write(const uint8_t input[], size_t len) override { m_mac->update(input, len); }
      void end_msg() override;

      std::string name() const override { return m_mac->name(); }

      /**
      * Set the key of this filter.
      * @param key the key to set
      */
      void set_key(const SymmetricKey& key) override { m_mac->set_key(key); }

      Key_Length_Specification key_spec() const override { return m_mac->key_spec(); }

      /**
      * Construct a MAC filter. The MAC key will be left empty.
      * @param mac the MAC to use
      * @param out_len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(MessageAuthenticationCode* mac,
                 size_t out_len = 0) :
         m_mac(mac),
         m_out_len(out_len)
         {
         }

      /**
      * Construct a MAC filter.
      * @param mac the MAC to use
      * @param key the MAC key to use
      * @param out_len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(MessageAuthenticationCode* mac,
                 const SymmetricKey& key,
                 size_t out_len = 0) :
         m_mac(mac),
         m_out_len(out_len)
         {
         m_mac->set_key(key);
         }

      /**
      * Construct a MAC filter. The MAC key will be left empty.
      * @param mac the name of the MAC to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(const std::string& mac, size_t len = 0);

      /**
      * Construct a MAC filter.
      * @param mac the name of the MAC to use
      * @param key the MAC key to use
      * @param len the output length of this filter. Leave the default
      * value 0 if you want to use the full output of the
      * MAC. Otherwise, specify a smaller value here so that the
      * output of the MAC will be cut off.
      */
      MAC_Filter(const std::string& mac, const SymmetricKey& key,
                 size_t len = 0);
   private:
      std::unique_ptr<MessageAuthenticationCode> m_mac;
      const size_t m_out_len;
   };

}


namespace Botan {

namespace FPE {

/**
* Format Preserving Encryption using the scheme FE1 from the paper
* "Format-Preserving Encryption" by Bellare, Rogaway, et al
* (http://eprint.iacr.org/2009/251)
*
* Encrypt X from and onto the group Z_n using key and tweak
* @param n the modulus
* @param X the plaintext as a BigInt
* @param key a random key
* @param tweak will modify the ciphertext (think of as an IV)
*/
BigInt BOTAN_DLL fe1_encrypt(const BigInt& n, const BigInt& X,
                             const SymmetricKey& key,
                             const std::vector<uint8_t>& tweak);

/**
* Decrypt X from and onto the group Z_n using key and tweak
* @param n the modulus
* @param X the ciphertext as a BigInt
* @param key is the key used for encryption
* @param tweak the same tweak used for encryption
*/
BigInt BOTAN_DLL fe1_decrypt(const BigInt& n, const BigInt& X,
                             const SymmetricKey& key,
                             const std::vector<uint8_t>& tweak);

}

}


namespace Botan {

class GHASH;

/**
* GCM Mode
*/
class BOTAN_DLL GCM_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const uint8_t ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      // GCM supports arbitrary nonce lengths
      bool valid_nonce_length(size_t) const override { return true; }

      size_t tag_size() const override { return m_tag_size; }

      void clear() override;

      void reset() override;

      std::string provider() const override;
   protected:
      GCM_Mode(BlockCipher* cipher, size_t tag_size);

      const size_t m_BS = 16;

      const size_t m_tag_size;
      const std::string m_cipher_name;

      std::unique_ptr<StreamCipher> m_ctr;
      std::unique_ptr<GHASH> m_ghash;
   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;

      void key_schedule(const uint8_t key[], size_t length) override;
   };

/**
* GCM Encryption
*/
class BOTAN_DLL GCM_Encryption final : public GCM_Mode
   {
   public:
      /**
      * @param cipher the 128 bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      GCM_Encryption(BlockCipher* cipher, size_t tag_size = 16) :
         GCM_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   };

/**
* GCM Decryption
*/
class BOTAN_DLL GCM_Decryption final : public GCM_Mode
   {
   public:
      /**
      * @param cipher the 128 bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      GCM_Decryption(BlockCipher* cipher, size_t tag_size = 16) :
         GCM_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length >= tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   };

/**
* GCM's GHASH
* Maybe a Transform?
*/
class BOTAN_DLL GHASH : public SymmetricAlgorithm
   {
   public:
      void set_associated_data(const uint8_t ad[], size_t ad_len);

      secure_vector<uint8_t> nonce_hash(const uint8_t nonce[], size_t len);

      void start(const uint8_t nonce[], size_t len);

      /*
      * Assumes input len is multiple of 16
      */
      void update(const uint8_t in[], size_t len);

      secure_vector<uint8_t> final();

      Key_Length_Specification key_spec() const override
         { return Key_Length_Specification(16); }

      void clear() override;

      void reset();

      std::string name() const override { return "GHASH"; }
   protected:
      void ghash_update(secure_vector<uint8_t>& x,
                        const uint8_t input[], size_t input_len);

      void add_final_block(secure_vector<uint8_t>& x,
                           size_t ad_len, size_t pt_len);

      secure_vector<uint8_t> m_H;
      secure_vector<uint8_t> m_H_ad;
      secure_vector<uint8_t> m_ghash;
      size_t m_ad_len = 0;

   private:
      void key_schedule(const uint8_t key[], size_t key_len) override;

      void gcm_multiply(secure_vector<uint8_t>& x) const;

      secure_vector<uint8_t> m_nonce;
      size_t m_text_len = 0;
   };

}


namespace Botan {

typedef uint16_t gf2m;

/**
* GF(2^m) field for m = [2...16]
*/
class BOTAN_DLL GF2m_Field
   {
   public:
      explicit GF2m_Field(size_t extdeg);

      gf2m gf_mul(gf2m x, gf2m y) const
         {
         return ((x) ? gf_mul_fast(x, y) : 0);
         }

      gf2m gf_square(gf2m x) const
         {
         return ((x) ? gf_exp(_gf_modq_1(gf_log(x) << 1)) : 0);
         }

      gf2m square_rr(gf2m x) const
         {
         return _gf_modq_1(x << 1);
         }

      gf2m gf_mul_fast(gf2m x, gf2m y) const
         {
         return ((y) ? gf_exp(_gf_modq_1(gf_log(x) + gf_log(y))) : 0);
         }

      /*
      naming convention of GF(2^m) field operations:
        l logarithmic, unreduced
        r logarithmic, reduced
        n normal, non-zero
        z normal, might be zero
      */

      gf2m gf_mul_lll(gf2m a, gf2m b) const
         {
         return (a + b);
         }

      gf2m gf_mul_rrr(gf2m a, gf2m b) const
         {
         return (_gf_modq_1(gf_mul_lll(a, b)));
         }

      gf2m gf_mul_nrr(gf2m a, gf2m b) const
         {
         return (gf_exp(gf_mul_rrr(a, b)));
         }

      gf2m gf_mul_rrn(gf2m a, gf2m y) const
         {
         return _gf_modq_1(gf_mul_lll(a, gf_log(y)));
         }

      gf2m gf_mul_rnr(gf2m y, gf2m a) const
         {
         return gf_mul_rrn(a, y);
         }

      gf2m gf_mul_lnn(gf2m x, gf2m y) const
         {
         return (gf_log(x) + gf_log(y));
         }

      gf2m gf_mul_rnn(gf2m x, gf2m y) const
         {
         return _gf_modq_1(gf_mul_lnn(x, y));
         }

      gf2m gf_mul_nrn(gf2m a, gf2m y) const
         {
         return gf_exp(_gf_modq_1((a) + gf_log(y)));
         }

      /**
      * zero operand allowed
      */
      gf2m gf_mul_zrz(gf2m a, gf2m y) const
         {
         return ( (y == 0) ? 0 : gf_mul_nrn(a, y) );
         }

      gf2m gf_mul_zzr(gf2m a, gf2m y) const
         {
         return gf_mul_zrz(y, a);
         }

      /**
      * non-zero operand
      */
      gf2m gf_mul_nnr(gf2m y, gf2m a) const
         {
         return gf_mul_nrn(a, y);
         }

      gf2m gf_sqrt(gf2m x) const
         {
         return ((x) ? gf_exp(_gf_modq_1(gf_log(x) << (get_extension_degree()-1))) : 0);
         }

      gf2m gf_div_rnn(gf2m x, gf2m y) const
         {
         return _gf_modq_1(gf_log(x) - gf_log(y));
         }

      gf2m gf_div_rnr(gf2m x, gf2m b) const
         {
         return _gf_modq_1(gf_log(x) - b);
         }

      gf2m gf_div_nrr(gf2m a, gf2m b) const
         {
         return gf_exp(_gf_modq_1(a - b));
         }

      gf2m gf_div_zzr(gf2m x, gf2m b) const
         {
         return ((x) ? gf_exp(_gf_modq_1(gf_log(x) - b)) : 0);
         }

      gf2m gf_inv(gf2m x) const
         {
         return gf_exp(gf_ord() - gf_log(x));
         }

      gf2m gf_inv_rn(gf2m x) const
         {
         return (gf_ord() - gf_log(x));
         }

      gf2m gf_square_ln(gf2m x) const
         {
         return gf_log(x) << 1;
         }

      gf2m gf_square_rr(gf2m a) const
         {
         return a << 1;
         }

      gf2m gf_l_from_n(gf2m x) const
         {
         return gf_log(x);
         }

      gf2m gf_div(gf2m x, gf2m y) const;

      gf2m gf_pow(gf2m x, int i) const;

      gf2m gf_exp(gf2m i) const
         {
         return m_gf_exp_table.at(i); /* alpha^i */
         }

      gf2m gf_log(gf2m i) const
         {
         return m_gf_log_table.at(i); /* return i when x=alpha^i */
         }

      gf2m gf_ord() const
         {
         return m_gf_multiplicative_order;
         }

      gf2m get_extension_degree() const
         {
         return m_gf_extension_degree;
         }

      gf2m get_cardinality() const
         {
         return static_cast<gf2m>(1 << get_extension_degree());
         }

   private:
      gf2m _gf_modq_1(int32_t d) const
         {
         /* residual modulo q-1
         when -q < d < 0, we get (q-1+d)
         when 0 <= d < q, we get (d)
         when q <= d < 2q-1, we get (d-q+1)
         */
         return (((d) & gf_ord()) + ((d) >> get_extension_degree()));
         }

      gf2m m_gf_extension_degree, m_gf_multiplicative_order;
      const std::vector<gf2m>& m_gf_log_table;
      const std::vector<gf2m>& m_gf_exp_table;
   };

uint32_t encode_gf2m(gf2m to_enc, uint8_t* mem);

gf2m decode_gf2m(const uint8_t* mem);

}


namespace Botan {

/**
* GMAC
*/
class BOTAN_DLL GMAC : public MessageAuthenticationCode,
                       public GHASH

   {
   public:
      void clear() override;
      std::string name() const override;
      size_t output_length() const override;
      MessageAuthenticationCode* clone() const override;

      /**
      * Must be called to set the initialization vector prior to GMAC
      * calculation.
      *
      * @param nonce Initialization vector.
      * @param nonce_len size of initialization vector.
      */
      void start(const uint8_t nonce[], size_t nonce_len);

      /**
      * Must be called to set the initialization vector prior to GMAC
      * calculation.
      *
      * @param nonce Initialization vector.
      */
      void start(const secure_vector<uint8_t>& nonce);

      /**
      * Must be called to set the initialization vector prior to GMAC
      * calculation.
      *
      * @param nonce Initialization vector.
      */
      void start(const std::vector<uint8_t>& nonce);

      Key_Length_Specification key_spec() const override
         {
         return m_cipher->key_spec();
         }

      /**
      * Creates a new GMAC instance.
      *
      * @param cipher the underlying block cipher to use
      */
      explicit GMAC(BlockCipher* cipher);

      GMAC(const GMAC&) = delete;
      GMAC& operator=(const GMAC&) = delete;

   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;
      void key_schedule(const uint8_t key[], size_t size) override;

      static const size_t GCM_BS = 16;
      secure_vector<uint8_t> m_aad_buf;
      std::unique_ptr<BlockCipher> m_cipher;
      bool m_initialized;
   };

}

namespace Botan {

/**
* The GOST 28147-89 block cipher uses a set of 4 bit Sboxes, however
* the standard does not actually define these Sboxes; they are
* considered a local configuration issue. Several different sets are
* used.
*/
class BOTAN_DLL GOST_28147_89_Params
   {
   public:
      /**
      * @param row the row
      * @param col the column
      * @return sbox entry at this row/column
      */
      uint8_t sbox_entry(size_t row, size_t col) const;

      /**
      * @return name of this parameter set
      */
      std::string param_name() const { return m_name; }

      /**
      * Default GOST parameters are the ones given in GOST R 34.11 for
      * testing purposes; these sboxes are also used by Crypto++, and,
      * at least according to Wikipedia, the Central Bank of Russian
      * Federation
      * @param name of the parameter set
      */
      GOST_28147_89_Params(const std::string& name = "R3411_94_TestParam");
   private:
      const uint8_t* m_sboxes;
      std::string m_name;
   };

/**
* GOST 28147-89
*/
class BOTAN_DLL GOST_28147_89 final : public Block_Cipher_Fixed_Params<8, 32>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string name() const override;
      BlockCipher* clone() const override { return new GOST_28147_89(m_SBOX); }

      /**
      * @param params the sbox parameters to use
      */
      explicit GOST_28147_89(const GOST_28147_89_Params& params);
   private:
      explicit GOST_28147_89(const std::vector<uint32_t>& other_SBOX) :
         m_SBOX(other_SBOX), m_EK(8) {}

      void key_schedule(const uint8_t[], size_t) override;

      /*
      * The sbox is not secret, this is just a larger expansion of it
      * which we generate at runtime for faster execution
      */
      std::vector<uint32_t> m_SBOX;

      secure_vector<uint32_t> m_EK;
   };

}


namespace Botan {

/**
* GOST-34.10 Public Key
*/
class BOTAN_DLL GOST_3410_PublicKey : public virtual EC_PublicKey
   {
   public:

      /**
      * Construct a public key from a given public point.
      * @param dom_par the domain parameters associated with this key
      * @param public_point the public point defining this key
      */
      GOST_3410_PublicKey(const EC_Group& dom_par,
                          const PointGFp& public_point) :
         EC_PublicKey(dom_par, public_point) {}

      /**
      * Load a public key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded public key bits
      */
      GOST_3410_PublicKey(const AlgorithmIdentifier& alg_id,
                          const std::vector<uint8_t>& key_bits);

      /**
      * Get this keys algorithm name.
      * @result this keys algorithm name
      */
      std::string algo_name() const override { return "GOST-34.10"; }

      AlgorithmIdentifier algorithm_identifier() const override;

      std::vector<uint8_t> public_key_bits() const override;

      size_t message_parts() const override { return 2; }

      size_t message_part_size() const override
         { return domain().get_order().bytes(); }

      std::unique_ptr<PK_Ops::Verification>
         create_verification_op(const std::string& params,
                                const std::string& provider) const override;

   protected:
      GOST_3410_PublicKey() {}
   };

/**
* GOST-34.10 Private Key
*/
class BOTAN_DLL GOST_3410_PrivateKey : public GOST_3410_PublicKey,
                                       public EC_PrivateKey
   {
   public:
      /**
      * Load a private key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits PKCS #8 structure
      */
      GOST_3410_PrivateKey(const AlgorithmIdentifier& alg_id,
                           const secure_vector<uint8_t>& key_bits) :
         EC_PrivateKey(alg_id, key_bits) {}

      /**
      * Generate a new private key
      * @param rng a random number generator
      * @param domain parameters to used for this key
      * @param x the private key; if zero, a new random key is generated
      */
      GOST_3410_PrivateKey(RandomNumberGenerator& rng,
                           const EC_Group& domain,
                           const BigInt& x = 0) :
         EC_PrivateKey(rng, domain, x) {}

      AlgorithmIdentifier pkcs8_algorithm_identifier() const override
         { return EC_PublicKey::algorithm_identifier(); }

      std::unique_ptr<PK_Ops::Signature>
         create_signature_op(RandomNumberGenerator& rng,
                             const std::string& params,
                             const std::string& provider) const override;
   };

}


namespace Botan {

/**
* GOST 34.11
*/
class BOTAN_DLL GOST_34_11 final : public HashFunction
   {
   public:
      std::string name() const override { return "GOST-R-34.11-94" ; }
      size_t output_length() const override { return 32; }
      size_t hash_block_size() const override { return 32; }
      HashFunction* clone() const override { return new GOST_34_11; }

      void clear() override;

      GOST_34_11();
   private:
      void compress_n(const uint8_t input[], size_t blocks);

      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;

      GOST_28147_89 m_cipher;
      secure_vector<uint8_t> m_buffer, m_sum, m_hash;
      size_t m_position;
      uint64_t m_count;
   };

}


namespace Botan {

/**
* Return the PKCS #1 hash identifier
* @see RFC 3447 section 9.2
* @param hash_name the name of the hash function
* @return uint8_t sequence identifying the hash
* @throw Invalid_Argument if the hash has no known PKCS #1 hash id
*/
BOTAN_DLL std::vector<uint8_t> pkcs_hash_id(const std::string& hash_name);

/**
* Return the IEEE 1363 hash identifier
* @param hash_name the name of the hash function
* @return uint8_t code identifying the hash, or 0 if not known
*/
BOTAN_DLL uint8_t ieee1363_hash_id(const std::string& hash_name);

}


namespace Botan {

/**
* Perform hex encoding
* @param output an array of at least input_length*2 bytes
* @param input is some binary data
* @param input_length length of input in bytes
* @param uppercase should output be upper or lower case?
*/
void BOTAN_DLL hex_encode(char output[],
                          const uint8_t input[],
                          size_t input_length,
                          bool uppercase = true);

/**
* Perform hex encoding
* @param input some input
* @param input_length length of input in bytes
* @param uppercase should output be upper or lower case?
* @return hexadecimal representation of input
*/
std::string BOTAN_DLL hex_encode(const uint8_t input[],
                                 size_t input_length,
                                 bool uppercase = true);

/**
* Perform hex encoding
* @param input some input
* @param uppercase should output be upper or lower case?
* @return hexadecimal representation of input
*/
template<typename Alloc>
std::string hex_encode(const std::vector<uint8_t, Alloc>& input,
                       bool uppercase = true)
   {
   return hex_encode(input.data(), input.size(), uppercase);
   }

/**
* Perform hex decoding
* @param output an array of at least input_length/2 bytes
* @param input some hex input
* @param input_length length of input in bytes
* @param input_consumed is an output parameter which says how many
*        bytes of input were actually consumed. If less than
*        input_length, then the range input[consumed:length]
*        should be passed in later along with more input.
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL hex_decode(uint8_t output[],
                            const char input[],
                            size_t input_length,
                            size_t& input_consumed,
                            bool ignore_ws = true);

/**
* Perform hex decoding
* @param output an array of at least input_length/2 bytes
* @param input some hex input
* @param input_length length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL hex_decode(uint8_t output[],
                            const char input[],
                            size_t input_length,
                            bool ignore_ws = true);

/**
* Perform hex decoding
* @param output an array of at least input_length/2 bytes
* @param input some hex input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return number of bytes written to output
*/
size_t BOTAN_DLL hex_decode(uint8_t output[],
                            const std::string& input,
                            bool ignore_ws = true);

/**
* Perform hex decoding
* @param input some hex input
* @param input_length the length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
std::vector<uint8_t> BOTAN_DLL
hex_decode(const char input[],
           size_t input_length,
           bool ignore_ws = true);

/**
* Perform hex decoding
* @param input some hex input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
std::vector<uint8_t> BOTAN_DLL
hex_decode(const std::string& input,
           bool ignore_ws = true);


/**
* Perform hex decoding
* @param input some hex input
* @param input_length the length of input in bytes
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
secure_vector<uint8_t> BOTAN_DLL
hex_decode_locked(const char input[],
                  size_t input_length,
                  bool ignore_ws = true);

/**
* Perform hex decoding
* @param input some hex input
* @param ignore_ws ignore whitespace on input; if false, throw an
                   exception if whitespace is encountered
* @return decoded hex output
*/
secure_vector<uint8_t> BOTAN_DLL
hex_decode_locked(const std::string& input,
                  bool ignore_ws = true);

}


namespace Botan {

/**
* HKDF from RFC 5869.
*/
class BOTAN_DLL HKDF final : public KDF
   {
   public:
      /**
      * @param prf MAC algorithm to use
      */
      explicit HKDF(MessageAuthenticationCode* prf) : m_prf(prf) {}

      KDF* clone() const override { return new HKDF(m_prf->clone()); }

      std::string name() const override { return "HKDF(" + m_prf->name() + ")"; }

      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

   private:
      std::unique_ptr<MessageAuthenticationCode> m_prf;
   };

/**
* HKDF Extraction Step from RFC 5869.
*/
class BOTAN_DLL HKDF_Extract final : public KDF
   {
   public:
      /**
      * @param prf MAC algorithm to use
      */
      explicit HKDF_Extract(MessageAuthenticationCode* prf) : m_prf(prf) {}

      KDF* clone() const override { return new HKDF_Extract(m_prf->clone()); }

      std::string name() const override { return "HKDF-Extract(" + m_prf->name() + ")"; }

      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

   private:
      std::unique_ptr<MessageAuthenticationCode> m_prf;
   };

/**
* HKDF Expansion Step from RFC 5869.
*/
class BOTAN_DLL HKDF_Expand final : public KDF
   {
   public:
      /**
      * @param prf MAC algorithm to use
      */
      explicit HKDF_Expand(MessageAuthenticationCode* prf) : m_prf(prf) {}

      KDF* clone() const override { return new HKDF_Expand(m_prf->clone()); }

      std::string name() const override { return "HKDF-Expand(" + m_prf->name() + ")"; }

      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

   private:
      std::unique_ptr<MessageAuthenticationCode> m_prf;
   };

}


namespace Botan {

/**
* HMAC
*/
class BOTAN_DLL HMAC final : public MessageAuthenticationCode
   {
   public:
      void clear() override;
      std::string name() const override;
      MessageAuthenticationCode* clone() const override;

      size_t output_length() const override { return m_hash->output_length(); }

      Key_Length_Specification key_spec() const override
         {
         // Absurd max length here is to support PBKDF2
         return Key_Length_Specification(0, 512);
         }

      /**
      * @param hash the hash to use for HMACing
      */
      explicit HMAC(HashFunction* hash);

      HMAC(const HMAC&) = delete;
      HMAC& operator=(const HMAC&) = delete;
   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      void key_schedule(const uint8_t[], size_t) override;

      std::unique_ptr<HashFunction> m_hash;
      secure_vector<uint8_t> m_ikey, m_okey;
   };

}


namespace Botan {

/**
* Inherited by RNGs which maintain in-process state, like HMAC_DRBG.
* On Unix these RNGs are vulnerable to problems with fork, where the
* RNG state is duplicated, and the parent and child process RNGs will
* produce identical output until one of them reseeds. Stateful_RNG
* reseeds itself whenever a fork is detected, or after a set number of
* bytes have been output.
*
* Not implemented by RNGs which access an external RNG, such as the
* system PRNG or a hardware RNG.
*/
class BOTAN_DLL Stateful_RNG : public RandomNumberGenerator
   {
   public:
      /**
      * @param rng is a reference to some RNG which will be used
      * to perform the periodic reseeding
      * @param entropy_sources will be polled to perform reseeding periodically
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      */
      Stateful_RNG(RandomNumberGenerator& rng,
                   Entropy_Sources& entropy_sources,
                   size_t reseed_interval) :
         m_underlying_rng(&rng),
         m_entropy_sources(&entropy_sources),
         m_reseed_interval(reseed_interval)
         {}

      /**
      * @param rng is a reference to some RNG which will be used
      * to perform the periodic reseeding
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      */
      Stateful_RNG(RandomNumberGenerator& rng, size_t reseed_interval) :
         m_underlying_rng(&rng),
         m_reseed_interval(reseed_interval)
         {}

      /**
      * @param entropy_sources will be polled to perform reseeding periodically
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      */
      Stateful_RNG(Entropy_Sources& entropy_sources, size_t reseed_interval) :
         m_entropy_sources(&entropy_sources),
         m_reseed_interval(reseed_interval)
         {}

      /**
      * In this case, automatic reseeding is impossible
      */
      Stateful_RNG() : m_reseed_interval(0) {}

      /**
      * Consume this input and mark the RNG as initialized regardless
      * of the length of the input or the current seeded state of
      * the RNG.
      */
      void initialize_with(const uint8_t input[], size_t length);

      bool is_seeded() const override final;

      /**
      * Mark state as requiring a reseed on next use
      */
      void force_reseed();

      void reseed_from_rng(RandomNumberGenerator& rng,
                           size_t poll_bits = BOTAN_RNG_RESEED_POLL_BITS) override final;

      /**
      * Overrides default implementation and also includes the current
      * process ID and the reseed counter.
      */
      void randomize_with_ts_input(uint8_t output[], size_t output_len) override final;

      /**
      * Poll provided sources for up to poll_bits bits of entropy
      * or until the timeout expires. Returns estimate of the number
      * of bits collected.
      */
      size_t reseed(Entropy_Sources& srcs,
                    size_t poll_bits = BOTAN_RNG_RESEED_POLL_BITS,
                    std::chrono::milliseconds poll_timeout = BOTAN_RNG_RESEED_DEFAULT_TIMEOUT) override;

      /**
      * @return intended security level of this DRBG
      */
      virtual size_t security_level() const = 0;

      void clear() override;

   protected:
      /**
      * Called with lock held
      */
      void reseed_check();

      uint32_t last_pid() const { return m_last_pid; }

   private:
      // A non-owned and possibly null pointer to shared RNG
      RandomNumberGenerator* m_underlying_rng = nullptr;

      // A non-owned and possibly null pointer to a shared Entropy_Source
      Entropy_Sources* m_entropy_sources = nullptr;

      const size_t m_reseed_interval;

      /*
      * Set to 1 after a sucessful seeding, then incremented.  Reset
      * to 0 by clear() or a fork. This logic is used even if
      * automatic reseeding is disabled (via m_reseed_interval = 0)
      */
      size_t m_reseed_counter = 0;
      uint32_t m_last_pid = 0;
   };

}


namespace Botan {

class Entropy_Sources;

/**
* HMAC_DRBG from NIST SP800-90A
*/
class BOTAN_DLL HMAC_DRBG final : public Stateful_RNG
   {
   public:
      /**
      * Initialize an HMAC_DRBG instance with the given MAC as PRF (normally HMAC)
      *
      * Automatic reseeding is disabled completely, as it has no access to
      * any source for seed material.
      *
      * If a fork is detected, the RNG will be unable to reseed itself
      * in response. In this case, an exception will be thrown rather
      * than generating duplicated output.
      */
      HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf);

      /**
      * Initialize an HMAC_DRBG instance with the given MAC as PRF (normally HMAC)
      *
      * Automatic reseeding from @p underlying_rng will take place after
      * @p reseed_interval many requests or after a fork was detected.
      *
      * @param prf MAC to use as a PRF
      * @param underlying_rng is a reference to some RNG which will be used
      * to perform the periodic reseeding
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed
      * @param max_number_of_bytes_per_request requests that are in size higher 
      * than max_number_of_bytes_per_request are treated as if multiple single 
      * requests of max_number_of_bytes_per_request size had been made.
      * In theory SP 800-90A requires that we reject any request for a DRBG
      * output longer than max_number_of_bytes_per_request. To avoid inconveniencing
      * the caller who wants an output larger than max_number_of_bytes_per_request, 
      * instead treat these requests as if multiple requests of 
      * max_number_of_bytes_per_request size had been made. NIST requires for 
      * HMAC_DRBG that every implementation set a value no more than 2**19 bits 
      * (or 64 KiB). Together with @p reseed_interval = 1 you can enforce that for 
      * example every 512 bit automatic reseeding occurs.
      */
      HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf,
                RandomNumberGenerator& underlying_rng,
                size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL,
                size_t max_number_of_bytes_per_request = 64 * 1024);

      /**
      * Initialize an HMAC_DRBG instance with the given MAC as PRF (normally HMAC)
      *
      * Automatic reseeding from @p entropy_sources will take place after
      * @p reseed_interval many requests or after a fork was detected.
      *
      * @param prf MAC to use as a PRF
      * @param entropy_sources will be polled to perform reseeding periodically
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed.
      * @param max_number_of_bytes_per_request requests that are in size higher 
      * than max_number_of_bytes_per_request are treated as if multiple single 
      * requests of max_number_of_bytes_per_request size had been made.
      * In theory SP 800-90A requires that we reject any request for a DRBG
      * output longer than max_number_of_bytes_per_request. To avoid inconveniencing
      * the caller who wants an output larger than max_number_of_bytes_per_request, 
      * instead treat these requests as if multiple requests of 
      * max_number_of_bytes_per_request size had been made. NIST requires for 
      * HMAC_DRBG that every implementation set a value no more than 2**19 bits 
      * (or 64 KiB). Together with @p reseed_interval = 1 you can enforce that for 
      * example every 512 bit automatic reseeding occurs.
      */
      HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf,
                Entropy_Sources& entropy_sources,
                size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL,
                size_t max_number_of_bytes_per_request = 64 * 1024);

      /**
      * Initialize an HMAC_DRBG instance with the given MAC as PRF (normally HMAC)
      *
      * Automatic reseeding from @p underlying_rng and @p entropy_sources
      * will take place after @p reseed_interval many requests or after
      * a fork was detected.
      *
      * @param prf MAC to use as a PRF
      * @param underlying_rng is a reference to some RNG which will be used
      * to perform the periodic reseeding
      * @param entropy_sources will be polled to perform reseeding periodically
      * @param reseed_interval specifies a limit of how many times
      * the RNG will be called before automatic reseeding is performed.
      * @param max_number_of_bytes_per_request requests that are in size higher 
      * than max_number_of_bytes_per_request are treated as if multiple single 
      * requests of max_number_of_bytes_per_request size had been made.
      * In theory SP 800-90A requires that we reject any request for a DRBG
      * output longer than max_number_of_bytes_per_request. To avoid inconveniencing
      * the caller who wants an output larger than max_number_of_bytes_per_request, 
      * instead treat these requests as if multiple requests of 
      * max_number_of_bytes_per_request size had been made. NIST requires for 
      * HMAC_DRBG that every implementation set a value no more than 2**19 bits 
      * (or 64 KiB). Together with @p reseed_interval = 1 you can enforce that for 
      * example every 512 bit automatic reseeding occurs.
      */
      HMAC_DRBG(std::unique_ptr<MessageAuthenticationCode> prf,
                RandomNumberGenerator& underlying_rng,
                Entropy_Sources& entropy_sources,
                size_t reseed_interval = BOTAN_RNG_DEFAULT_RESEED_INTERVAL,
                size_t max_number_of_bytes_per_request = 64 * 1024);

      /**
      * Constructor taking a string for the hash
      */
      HMAC_DRBG(const std::string& hmac_hash) :
         Stateful_RNG(),
         m_mac(MessageAuthenticationCode::create_or_throw("HMAC(" + hmac_hash + ")")),
         m_max_number_of_bytes_per_request(64 * 1024)
         {
         clear();
         }

      std::string name() const override;

      void clear() override;

      void randomize(uint8_t output[], size_t output_len) override;

      void randomize_with_input(uint8_t output[], size_t output_len,
                                const uint8_t input[], size_t input_len) override;

      void add_entropy(const uint8_t input[], size_t input_len) override;

      size_t security_level() const override;

   private:
      void update(const uint8_t input[], size_t input_len);

      std::unique_ptr<MessageAuthenticationCode> m_mac;
      secure_vector<uint8_t> m_V;
      const size_t m_max_number_of_bytes_per_request;
   };

}


namespace Botan {

namespace HTTP {

struct Response
   {
   public:
      Response() : m_status_code(0), m_status_message("Uninitialized") {}

      Response(unsigned int status_code, const std::string& status_message,
               const std::vector<uint8_t>& body,
               const std::map<std::string, std::string>& headers) :
         m_status_code(status_code),
         m_status_message(status_message),
         m_body(body),
         m_headers(headers) {}

      unsigned int status_code() const { return m_status_code; }

      const std::vector<uint8_t>& body() const { return m_body; }

      const std::map<std::string, std::string>& headers() const { return m_headers; }

      std::string status_message() const { return m_status_message; }

      void throw_unless_ok()
         {
         if(status_code() != 200)
            throw Exception("HTTP error: " + status_message());
         }

   private:
      unsigned int m_status_code;
      std::string m_status_message;
      std::vector<uint8_t> m_body;
      std::map<std::string, std::string> m_headers;
   };

/**
* HTTP_Error Exception
*/
struct BOTAN_DLL HTTP_Error : public Exception
   {
   explicit HTTP_Error(const std::string& msg) :
      Exception("HTTP error " + msg)
      {}
   };

BOTAN_DLL std::ostream& operator<<(std::ostream& o, const Response& resp);

typedef std::function<std::string (const std::string&, const std::string&)> http_exch_fn;

BOTAN_DLL Response http_sync(http_exch_fn fn,
                             const std::string& verb,
                             const std::string& url,
                             const std::string& content_type,
                             const std::vector<uint8_t>& body,
                             size_t allowable_redirects);

BOTAN_DLL Response http_sync(const std::string& verb,
                             const std::string& url,
                             const std::string& content_type,
                             const std::vector<uint8_t>& body,
                             size_t allowable_redirects);

BOTAN_DLL Response GET_sync(const std::string& url,
                            size_t allowable_redirects = 1);

BOTAN_DLL Response POST_sync(const std::string& url,
                             const std::string& content_type,
                             const std::vector<uint8_t>& body,
                             size_t allowable_redirects = 1);

BOTAN_DLL std::string url_encode(const std::string& url);

}

}


namespace Botan {

/**
* IDEA
*/
class BOTAN_DLL IDEA final : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;

      std::string provider() const override;
      std::string name() const override { return "IDEA"; }
      BlockCipher* clone() const override { return new IDEA; }
   private:
#if defined(BOTAN_HAS_IDEA_SSE2)
      void sse2_idea_op_8(const uint8_t in[64], uint8_t out[64], const uint16_t EK[52]) const;
#endif

      void key_schedule(const uint8_t[], size_t) override;

      secure_vector<uint16_t> m_EK, m_DK;
   };

}


namespace Botan {

/*
* Previously botan had state whose lifetime had to be explicitly
* managed by the application. As of 1.11.14 this is no longer the
* case, and this class is no longer needed and kept only for backwards
* compatibility.
*/
class BOTAN_DLL BOTAN_DEPRECATED("LibraryInitializer is no longer required") LibraryInitializer
   {
   public:
      explicit LibraryInitializer(const std::string& /*ignored*/ = "") { }

      static void initialize(const std::string& /*ignored*/ = "") {}
      static void deinitialize() {}
   };

}


namespace Botan {

/**
* ISO-9796-2 - Digital signature scheme 2 (probabilistic)
*/
class BOTAN_DLL ISO_9796_DS2 final : public EMSA
   {
   public:
      /**
       * @param hash function to use
       * @param implicit whether or not the trailer is implicit
       */
      explicit ISO_9796_DS2(HashFunction* hash, bool implicit = false) : m_hash(hash), m_implicit(implicit),
         m_SALT_SIZE(hash->output_length()) {}

      /**
       * @param hash function to use
       * @param implicit whether or not the trailer is implicit
       * @param salt_size size of the salt to use in bytes
       */
      ISO_9796_DS2(HashFunction* hash, bool implicit, size_t salt_size) : m_hash(hash), m_implicit(implicit),
         m_SALT_SIZE(salt_size) {}

      EMSA* clone() override
         {return new ISO_9796_DS2(m_hash->clone(), m_implicit, m_SALT_SIZE);}
   private:
      void update(const uint8_t input[], size_t length) override;

      secure_vector<uint8_t> raw_data() override;

      secure_vector<uint8_t> encoding_of(const secure_vector<uint8_t>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<uint8_t>& coded,
                  const secure_vector<uint8_t>& raw,
                  size_t key_bits) override;

      std::unique_ptr<HashFunction> m_hash;
      bool m_implicit;
      size_t m_SALT_SIZE;
      secure_vector<uint8_t> m_msg_buffer;
   };

/**
* ISO-9796-2 - Digital signature scheme 3 (deterministic)
*/
class BOTAN_DLL ISO_9796_DS3 final : public EMSA
   {
   public:
      /**
       * @param hash function to use
       * @param implicit whether or not the trailer is implicit
       */
      ISO_9796_DS3(HashFunction* hash, bool implicit = false) : m_hash(hash), m_implicit(implicit)
         {}

      EMSA* clone() override
         {return new ISO_9796_DS3(m_hash->clone(), m_implicit);}
   private:
      void update(const uint8_t input[], size_t length) override;

      secure_vector<uint8_t> raw_data() override;

      secure_vector<uint8_t> encoding_of(const secure_vector<uint8_t>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<uint8_t>& coded,
                  const secure_vector<uint8_t>& raw,
                  size_t key_bits) override;

      std::unique_ptr<HashFunction> m_hash;
      bool m_implicit;
      secure_vector<uint8_t> m_msg_buffer;
   };

}


namespace Botan {

/**
* KASUMI, the block cipher used in 3G telephony
*/
class BOTAN_DLL KASUMI final : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "KASUMI"; }
      BlockCipher* clone() const override { return new KASUMI; }
   private:
      void key_schedule(const uint8_t[], size_t) override;

      secure_vector<uint16_t> m_EK;
   };

}


namespace Botan {

/**
* KDF1, from IEEE 1363
*/
class BOTAN_DLL KDF1 final : public KDF
   {
   public:
      std::string name() const override { return "KDF1(" + m_hash->name() + ")"; }

      KDF* clone() const override { return new KDF1(m_hash->clone()); }

      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

      /**
      * @param h hash function to use
      */
      explicit KDF1(HashFunction* h) : m_hash(h) {}
   private:
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

/**
* KDF1, from ISO 18033-2
*/
class BOTAN_DLL KDF1_18033 : public KDF
   {
   public:
      std::string name() const override { return "KDF1-18033(" + m_hash->name() + ")"; }

      KDF* clone() const override { return new KDF1_18033(m_hash->clone()); }

      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

      /**
      * @param h hash function to use
      */
      KDF1_18033(HashFunction* h) : m_hash(h) {}
   private:
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

/**
* KDF2, from IEEE 1363
*/
class BOTAN_DLL KDF2 final : public KDF
   {
   public:
      std::string name() const override { return "KDF2(" + m_hash->name() + ")"; }

      KDF* clone() const override { return new KDF2(m_hash->clone()); }

      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

      /**
      * @param h hash function to use
      */
      explicit KDF2(HashFunction* h) : m_hash(h) {}
   private:
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

/**
* Keccak[1600], a SHA-3 candidate
*/
class BOTAN_DLL Keccak_1600 final : public HashFunction
   {
   public:

      /**
      * @param output_bits the size of the hash output; must be one of
      *                    224, 256, 384, or 512
      */
      explicit Keccak_1600(size_t output_bits = 512);

      size_t hash_block_size() const override { return m_bitrate / 8; }
      size_t output_length() const override { return m_output_bits / 8; }

      HashFunction* clone() const override;
      std::string name() const override;
      void clear() override;

   private:
      void add_data(const uint8_t input[], size_t length) override;
      void final_result(uint8_t out[]) override;

      size_t m_output_bits, m_bitrate;
      secure_vector<uint64_t> m_S;
      size_t m_S_pos;
   };

}


namespace Botan {

namespace KeyPair {

/**
* Tests whether the key is consistent for encryption; whether
* encrypting and then decrypting gives to the original plaintext.
* @param rng the rng to use
* @param private_key the key to test
* @param public_key the key to test
* @param padding the encryption padding method to use
* @return true if consistent otherwise false
*/
BOTAN_DLL bool
encryption_consistency_check(RandomNumberGenerator& rng,
                             const Private_Key& private_key,
                             const Public_Key& public_key,
                             const std::string& padding);

/**
* Tests whether the key is consistent for signatures; whether a
* signature can be created and then verified
* @param rng the rng to use
* @param private_key the key to test
* @param public_key the key to test
* @param padding the signature padding method to use
* @return true if consistent otherwise false
*/
BOTAN_DLL bool
signature_consistency_check(RandomNumberGenerator& rng,
                            const Private_Key& private_key,
                            const Public_Key& public_key,
                            const std::string& padding);

/**
* Tests whether the key is consistent for encryption; whether
* encrypting and then decrypting gives to the original plaintext.
* @param rng the rng to use
* @param key the key to test
* @param padding the encryption padding method to use
* @return true if consistent otherwise false
*/
inline bool
encryption_consistency_check(RandomNumberGenerator& rng,
                             const Private_Key& key,
                             const std::string& padding)
   {
   return encryption_consistency_check(rng, key, key, padding);
   }

/**
* Tests whether the key is consistent for signatures; whether a
* signature can be created and then verified
* @param rng the rng to use
* @param key the key to test
* @param padding the signature padding method to use
* @return true if consistent otherwise false
*/
inline bool
signature_consistency_check(RandomNumberGenerator& rng,
                            const Private_Key& key,
                            const std::string& padding)
   {
   return signature_consistency_check(rng, key, key, padding);
   }

}

}


namespace Botan {

/**
* Lion is a block cipher construction designed by Ross Anderson and
* Eli Biham, described in "Two Practical and Provably Secure Block
* Ciphers: BEAR and LION". It has a variable block size and is
* designed to encrypt very large blocks (up to a megabyte)

* http://www.cl.cam.ac.uk/~rja14/Papers/bear-lion.pdf
*/
class BOTAN_DLL Lion final : public BlockCipher
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      size_t block_size() const override { return m_block_size; }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(2, 2*m_hash->output_length(), 2);
         }

      void clear() override;
      std::string name() const override;
      BlockCipher* clone() const override;

      /**
      * @param hash the hash to use internally
      * @param cipher the stream cipher to use internally
      * @param block_size the size of the block to use
      */
      Lion(HashFunction* hash,
           StreamCipher* cipher,
           size_t block_size);
   private:
      void key_schedule(const uint8_t[], size_t) override;

      size_t left_size() const { return m_hash->output_length(); }
      size_t right_size() const { return m_block_size - left_size(); }

      const size_t m_block_size;
      std::unique_ptr<HashFunction> m_hash;
      std::unique_ptr<StreamCipher> m_cipher;
      secure_vector<uint8_t> m_key1, m_key2;
   };

}


namespace Botan {

class McEliece_PublicKey;
class McEliece_PrivateKey;

/**
* McEliece Integrated Encryption System
* Derive a shared key using MCE KEM and encrypt/authenticate the
* plaintext and AD using AES-256 in OCB mode.
*/
secure_vector<uint8_t>
BOTAN_DLL mceies_encrypt(const McEliece_PublicKey& pubkey,
                         const uint8_t pt[], size_t pt_len,
                         const uint8_t ad[], size_t ad_len,
                         RandomNumberGenerator& rng,
                         const std::string& aead = "AES-256/OCB");

/**
* McEliece Integrated Encryption System
* Derive a shared key using MCE KEM and decrypt/authenticate the
* ciphertext and AD using AES-256 in OCB mode.
*/
secure_vector<uint8_t>
BOTAN_DLL mceies_decrypt(const McEliece_PrivateKey& privkey,
                         const uint8_t ct[], size_t ct_len,
                         const uint8_t ad[], size_t ad_len,
                         const std::string& aead = "AES-256/OCB");


}


namespace Botan {

class RandomNumberGenerator;

struct polyn_gf2m
   {
   public:
      /**
      * create a zero polynomial:
      */
      explicit polyn_gf2m( std::shared_ptr<GF2m_Field> sp_field );

      polyn_gf2m()
         :m_deg(-1)
         {};

      polyn_gf2m(const secure_vector<uint8_t>& encoded, std::shared_ptr<GF2m_Field> sp_field );

      polyn_gf2m& operator=(const polyn_gf2m&) = default;

      bool operator==(const polyn_gf2m & other) const ;

      bool operator!=(const polyn_gf2m & other) const { return !(*this == other); };

      polyn_gf2m(polyn_gf2m&& other)
         {
         this->swap(other);
         };

      polyn_gf2m & operator=(polyn_gf2m&& other)
         {
         if(this != &other)
            {
            this->swap(other);
            }
         return *this;
         }

      void swap(polyn_gf2m& other);

      secure_vector<uint8_t> encode() const;
      /**
      * create zero polynomial with reservation of space for a degree d polynomial
      */
      polyn_gf2m(int d, std::shared_ptr<GF2m_Field> sp_field);

      polyn_gf2m(polyn_gf2m const& other);
      /**
      * create zero polynomial with allocated size determined by specified degree d:
      */

      /**
      * random irreducible polynomial of degree t
      */
      polyn_gf2m(int t, RandomNumberGenerator& rng, std::shared_ptr<GF2m_Field> sp_field);

      std::shared_ptr<GF2m_Field> get_sp_field() const
         { return msp_field; };

      gf2m& operator[](size_t i) { return coeff[i]; };

      gf2m operator[](size_t i) const { return coeff[i]; }

      gf2m get_lead_coef() const { return coeff[m_deg]; }

      gf2m get_coef(uint32_t i) const { return coeff[i]; }

      inline void set_coef(uint32_t i, gf2m v)
         {
         coeff[i] = v;
         };

      inline void add_to_coef(uint32_t i, gf2m v)
         {
         coeff[i] = coeff[i] ^ v;
         }

      std::string to_string() const;

      /** decode a polynomial from memory: **/
      polyn_gf2m(const uint8_t* mem, uint32_t mem_len, std::shared_ptr<GF2m_Field> sp_field);
      // remove one! ^v!
      /**
      *  create a polynomial from memory area (encoded)
      */
      polyn_gf2m(int degree, const unsigned  char* mem, uint32_t mem_byte_len, std::shared_ptr<GF2m_Field> sp_field);

      void encode(uint32_t min_numo_coeffs, uint8_t* mem, uint32_t mem_len) const;

      int get_degree() const;

      /**
      * determine the degree in a timing secure manner. the timing of this function
      * only depends on the number of allocated coefficients, not on the actual
      * degree
      */
      int calc_degree_secure() const;

      void degppf(const polyn_gf2m & g, int* p_result);

      static std::vector<polyn_gf2m> sqmod_init(const polyn_gf2m & g);

      static std::vector<polyn_gf2m> sqrt_mod_init(const polyn_gf2m & g);


      polyn_gf2m sqmod(const std::vector<polyn_gf2m> & sq, int d);
      void set_to_zero();
      gf2m eval(gf2m a);

      static std::pair<polyn_gf2m, polyn_gf2m> eea_with_coefficients(const polyn_gf2m & p,
                                                                     const polyn_gf2m & g,
                                                                     int break_deg);

      void patchup_deg_secure( uint32_t trgt_deg, volatile gf2m patch_elem);

   private:

      void set_degree(int d) { m_deg = d; }

      void poly_shiftmod( const polyn_gf2m & g);
      void realloc(uint32_t new_size);
      static polyn_gf2m gcd(polyn_gf2m const& p1, polyn_gf2m const& p2);

      /**
      * destructive:
      */
      static void remainder(polyn_gf2m & p, const polyn_gf2m & g);

      static polyn_gf2m gcd_aux(polyn_gf2m& p1, polyn_gf2m& p2);
   public:
      // public member variable:
      int m_deg;

      // public member variable:
      secure_vector<gf2m> coeff;

      // public member variable:
      std::shared_ptr<GF2m_Field> msp_field;
   };

gf2m random_gf2m(RandomNumberGenerator& rng);
gf2m random_code_element(unsigned code_length, RandomNumberGenerator& rng);

std::vector<polyn_gf2m> syndrome_init(polyn_gf2m const& generator, std::vector<gf2m> const& support, int n);

/**
* Find the roots of a polynomial over GF(2^m) using the method by Federenko
* et al.
*/
secure_vector<gf2m> find_roots_gf2m_decomp(const polyn_gf2m & polyn, uint32_t code_length);

}


namespace Botan {

class BOTAN_DLL McEliece_PublicKey : public virtual Public_Key
   {
   public:
      explicit McEliece_PublicKey(const std::vector<uint8_t>& key_bits);

      McEliece_PublicKey(std::vector<uint8_t> const& pub_matrix, uint32_t the_t, uint32_t the_code_length) :
         m_public_matrix(pub_matrix),
         m_t(the_t),
         m_code_length(the_code_length)
            {}

      McEliece_PublicKey(const McEliece_PublicKey& other);

      secure_vector<uint8_t> random_plaintext_element(RandomNumberGenerator& rng) const;

      std::string algo_name() const override { return "McEliece"; }

      AlgorithmIdentifier algorithm_identifier() const override;

      size_t key_length() const override;
      size_t estimated_strength() const override;

      std::vector<uint8_t> public_key_bits() const override;

      bool check_key(RandomNumberGenerator&, bool) const override
         { return true; }

      uint32_t get_t() const { return m_t; }
      uint32_t get_code_length() const { return m_code_length; }
      uint32_t get_message_word_bit_length() const;
      const std::vector<uint8_t>& get_public_matrix() const { return m_public_matrix; }

      bool operator==(const McEliece_PublicKey& other) const;
      bool operator!=(const McEliece_PublicKey& other) const { return !(*this == other); }

      std::unique_ptr<PK_Ops::KEM_Encryption>
         create_kem_encryption_op(RandomNumberGenerator& rng,
                                  const std::string& params,
                                  const std::string& provider) const override;

   protected:
      McEliece_PublicKey() : m_t(0), m_code_length(0) {}

      std::vector<uint8_t> m_public_matrix;
      uint32_t m_t;
      uint32_t m_code_length;
   };

class BOTAN_DLL McEliece_PrivateKey : public virtual McEliece_PublicKey,
                                      public virtual Private_Key
   {
   public:

      /**
      Generate a McEliece key pair

      Suggested parameters for a given security level (SL)

      SL=80 n=1632 t=33 - 59 KB pubkey 140 KB privkey
      SL=107 n=2480 t=45 - 128 KB pubkey 300 KB privkey
      SL=128 n=2960 t=57 - 195 KB pubkey 459 KB privkey
      SL=147 n=3408 t=67 - 265 KB pubkey 622 KB privkey
      SL=191 n=4624 t=95 - 516 KB pubkey 1234 KB privkey
      SL=256 n=6624 t=115 - 942 KB pubkey 2184 KB privkey
      */
      McEliece_PrivateKey(RandomNumberGenerator& rng, size_t code_length, size_t t);

      explicit McEliece_PrivateKey(const secure_vector<uint8_t>& key_bits);

      McEliece_PrivateKey(polyn_gf2m const& goppa_polyn,
                          std::vector<uint32_t> const& parity_check_matrix_coeffs,
                          std::vector<polyn_gf2m> const& square_root_matrix,
                          std::vector<gf2m> const& inverse_support,
                          std::vector<uint8_t> const& public_matrix );

      bool check_key(RandomNumberGenerator& rng, bool strong) const override;

      polyn_gf2m const& get_goppa_polyn() const { return m_g; }
      std::vector<uint32_t> const& get_H_coeffs() const { return m_coeffs; }
      std::vector<gf2m> const& get_Linv() const { return m_Linv; }
      std::vector<polyn_gf2m> const& get_sqrtmod() const { return m_sqrtmod; }

      inline uint32_t get_dimension() const { return m_dimension; }

      inline uint32_t get_codimension() const { return m_codimension; }

      secure_vector<uint8_t> private_key_bits() const override;

      bool operator==(const McEliece_PrivateKey & other) const;

      bool operator!=(const McEliece_PrivateKey& other) const { return !(*this == other); }

      std::unique_ptr<PK_Ops::KEM_Decryption>
         create_kem_decryption_op(RandomNumberGenerator& rng,
                                  const std::string& params,
                                  const std::string& provider) const override;
   private:
      polyn_gf2m m_g;
      std::vector<polyn_gf2m> m_sqrtmod;
      std::vector<gf2m> m_Linv;
      std::vector<uint32_t> m_coeffs;

      uint32_t m_codimension;
      uint32_t m_dimension;
   };

/**
* Estimate work factor for McEliece
* @return estimated security level for these key parameters
*/
BOTAN_DLL size_t mceliece_work_factor(size_t code_size, size_t t);

}


namespace Botan {

/**
* MDx Hash Function Base Class
*/
class BOTAN_DLL MDx_HashFunction : public HashFunction
   {
   public:
      /**
      * @param block_length is the number of bytes per block
      * @param big_byte_endian specifies if the hash uses big-endian bytes
      * @param big_bit_endian specifies if the hash uses big-endian bits
      * @param counter_size specifies the size of the counter var in bytes
      */
      MDx_HashFunction(size_t block_length,
                       bool big_byte_endian,
                       bool big_bit_endian,
                       size_t counter_size = 8);

      size_t hash_block_size() const override { return m_buffer.size(); }
   protected:
      void add_data(const uint8_t input[], size_t length) override;
      void final_result(uint8_t output[]) override;

      /**
      * Run the hash's compression function over a set of blocks
      * @param blocks the input
      * @param block_n the number of blocks
      */
      virtual void compress_n(const uint8_t blocks[], size_t block_n) = 0;

      void clear() override;

      /**
      * Copy the output to the buffer
      * @param buffer to put the output into
      */
      virtual void copy_out(uint8_t buffer[]) = 0;

      /**
      * Write the count, if used, to this spot
      * @param out where to write the counter to
      */
      virtual void write_count(uint8_t out[]);
   private:
      secure_vector<uint8_t> m_buffer;
      uint64_t m_count;
      size_t m_position;

      const bool BIG_BYTE_ENDIAN, BIG_BIT_ENDIAN;
      const size_t COUNT_SIZE;
   };

}


namespace Botan {

/**
* MD4
*/
class BOTAN_DLL MD4 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "MD4"; }
      size_t output_length() const override { return 16; }
      HashFunction* clone() const override { return new MD4; }

      void clear() override;

      MD4() : MDx_HashFunction(64, false, true), m_M(16), m_digest(4)
         { clear(); }
   protected:
      void compress_n(const uint8_t input[], size_t blocks) override;
      void copy_out(uint8_t[]) override;
   private:

      /**
      * The message buffer
      */
      secure_vector<uint32_t> m_M;

      /**
      * The digest value
      */
      secure_vector<uint32_t> m_digest;
   };

}


namespace Botan {

/**
* MD5
*/
class BOTAN_DLL MD5 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "MD5"; }
      size_t output_length() const override { return 16; }
      HashFunction* clone() const override { return new MD5; }

      void clear() override;

      MD5() : MDx_HashFunction(64, false, true), m_M(16), m_digest(4)
         { clear(); }
   protected:
      void compress_n(const uint8_t[], size_t blocks) override;
      void copy_out(uint8_t[]) override;

   private:
      /**
      * The message buffer
      */
      secure_vector<uint32_t> m_M;

      /**
      * The digest value
      */
      secure_vector<uint32_t> m_digest;
   };

}


namespace Botan {

/**
* MGF1 from PKCS #1 v2.0
* @param hash hash function to use
* @param in input buffer
* @param in_len size of the input buffer in bytes
* @param out output buffer
* @param out_len size of the output buffer in bytes
*/
void BOTAN_DLL mgf1_mask(HashFunction& hash,
                         const uint8_t in[], size_t in_len,
                         uint8_t out[], size_t out_len);

}


namespace Botan {

/**
* MISTY1 with 8 rounds
*/
class BOTAN_DLL MISTY1 final : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "MISTY1"; }
      BlockCipher* clone() const override { return new MISTY1; }
   private:
      void key_schedule(const uint8_t[], size_t) override;

      secure_vector<uint16_t> m_EK, m_DK;
   };

}


namespace Botan {

#if defined(__SIZEOF_INT128__) && defined(BOTAN_TARGET_CPU_HAS_NATIVE_64BIT)
   #define BOTAN_TARGET_HAS_NATIVE_UINT128

   // Prefer TI mode over __int128 as GCC rejects the latter in pendantic mode
   #if defined(__GNUG__)
     typedef unsigned int uint128_t __attribute__((mode(TI)));
   #else
     typedef unsigned __int128 uint128_t;
   #endif
#endif

}

#if defined(BOTAN_TARGET_HAS_NATIVE_UINT128)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi)      \
   do {                                      \
      const uint128_t r = static_cast<uint128_t>(a) * b;   \
      *hi = (r >> 64) & 0xFFFFFFFFFFFFFFFF;  \
      *lo = (r      ) & 0xFFFFFFFFFFFFFFFF;  \
   } while(0)

#elif defined(BOTAN_BUILD_COMPILER_IS_MSVC) && defined(BOTAN_TARGET_CPU_HAS_NATIVE_64BIT)

#include <intrin.h>
#pragma intrinsic(_umul128)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) \
   do { *lo = _umul128(a, b, hi); } while(0)

#elif defined(BOTAN_USE_GCC_INLINE_ASM)

#if defined(BOTAN_TARGET_ARCH_IS_X86_64)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {                           \
   asm("mulq %3" : "=d" (*hi), "=a" (*lo) : "a" (a), "rm" (b) : "cc"); \
   } while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_ALPHA)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {              \
   asm("umulh %1,%2,%0" : "=r" (*hi) : "r" (a), "r" (b)); \
   *lo = a * b;                                           \
} while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_IA64)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {                \
   asm("xmpy.hu %0=%1,%2" : "=f" (*hi) : "f" (a), "f" (b)); \
   *lo = a * b;                                             \
} while(0)

#elif defined(BOTAN_TARGET_ARCH_IS_PPC64)

#define BOTAN_FAST_64X64_MUL(a,b,lo,hi) do {                      \
   asm("mulhdu %0,%1,%2" : "=r" (*hi) : "r" (a), "r" (b) : "cc"); \
   *lo = a * b;                                                   \
} while(0)

#endif

#endif

namespace Botan {

/**
* Perform a 64x64->128 bit multiplication
*/
inline void mul64x64_128(uint64_t a, uint64_t b, uint64_t* lo, uint64_t* hi)
   {
#if defined(BOTAN_FAST_64X64_MUL)
   BOTAN_FAST_64X64_MUL(a, b, lo, hi);
#else

   /*
   * Do a 64x64->128 multiply using four 32x32->64 multiplies plus
   * some adds and shifts. Last resort for CPUs like UltraSPARC (with
   * 64-bit registers/ALU, but no 64x64->128 multiply) or 32-bit CPUs.
   */
   const size_t HWORD_BITS = 32;
   const uint32_t HWORD_MASK = 0xFFFFFFFF;

   const uint32_t a_hi = (a >> HWORD_BITS);
   const uint32_t a_lo = (a  & HWORD_MASK);
   const uint32_t b_hi = (b >> HWORD_BITS);
   const uint32_t b_lo = (b  & HWORD_MASK);

   uint64_t x0 = static_cast<uint64_t>(a_hi) * b_hi;
   uint64_t x1 = static_cast<uint64_t>(a_lo) * b_hi;
   uint64_t x2 = static_cast<uint64_t>(a_hi) * b_lo;
   uint64_t x3 = static_cast<uint64_t>(a_lo) * b_lo;

   // this cannot overflow as (2^32-1)^2 + 2^32-1 < 2^64-1
   x2 += x3 >> HWORD_BITS;

   // this one can overflow
   x2 += x1;

   // propagate the carry if any
   x0 += static_cast<uint64_t>(static_cast<bool>(x2 < x1)) << HWORD_BITS;

   *hi = x0 + (x2 >> HWORD_BITS);
   *lo  = ((x2 & HWORD_MASK) << HWORD_BITS) + (x3 & HWORD_MASK);
#endif
   }

}


namespace Botan {

/**
* Noekeon
*/
class BOTAN_DLL Noekeon final : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      std::string provider() const override;
      void clear() override;
      std::string name() const override { return "Noekeon"; }
      BlockCipher* clone() const override { return new Noekeon; }
   private:
#if defined(BOTAN_HAS_NOEKEON_SIMD)
      void simd_encrypt_4(const uint8_t in[], uint8_t out[]) const;
      void simd_decrypt_4(const uint8_t in[], uint8_t out[]) const;
#endif

      /**
      * The Noekeon round constants
      */
      static const uint8_t RC[17];

      void key_schedule(const uint8_t[], size_t) override;
      secure_vector<uint32_t> m_EK, m_DK;
   };

}


namespace Botan {

/**
* OAEP (called EME1 in IEEE 1363 and in earlier versions of the library)
*/
class BOTAN_DLL OAEP final : public EME
   {
   public:
      size_t maximum_input_size(size_t) const override;

      /**
      * @param hash function to use for hashing (takes ownership)
      * @param P an optional label. Normally empty.
      */
      OAEP(HashFunction* hash, const std::string& P = "");
   private:
      secure_vector<uint8_t> pad(const uint8_t in[],
                              size_t in_length,
                              size_t key_length,
                              RandomNumberGenerator& rng) const override;

      secure_vector<uint8_t> unpad(uint8_t& valid_mask,
                                const uint8_t in[],
                                size_t in_len) const override;

      secure_vector<uint8_t> m_Phash;
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

class L_computer;

/**
* OCB Mode (base class for OCB_Encryption and OCB_Decryption). Note
* that OCB is patented, but is freely licensed in some circumstances.
*
* @see "The OCB Authenticated-Encryption Algorithm" internet draft
        http://tools.ietf.org/html/draft-irtf-cfrg-ocb-03
* @see Free Licenses http://www.cs.ucdavis.edu/~rogaway/ocb/license.htm
* @see OCB home page http://www.cs.ucdavis.edu/~rogaway/ocb
*/
class BOTAN_DLL OCB_Mode : public AEAD_Mode
   {
   public:
      void set_associated_data(const uint8_t ad[], size_t ad_len) override;

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      bool valid_nonce_length(size_t) const override;

      size_t tag_size() const override { return m_tag_size; }

      void clear() override;

      void reset() override;

      ~OCB_Mode();
   protected:
      /**
      * @param cipher the 128-bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      OCB_Mode(BlockCipher* cipher, size_t tag_size);

      // fixme make these private
      std::unique_ptr<BlockCipher> m_cipher;
      std::unique_ptr<L_computer> m_L;

      size_t m_block_index = 0;

      secure_vector<uint8_t> m_checksum;
      secure_vector<uint8_t> m_offset;
      secure_vector<uint8_t> m_ad_hash;
   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;

      void key_schedule(const uint8_t key[], size_t length) override;

      secure_vector<uint8_t> update_nonce(const uint8_t nonce[], size_t nonce_len);

      size_t m_tag_size = 0;
      secure_vector<uint8_t> m_last_nonce;
      secure_vector<uint8_t> m_stretch;
   };

class BOTAN_DLL OCB_Encryption final : public OCB_Mode
   {
   public:
      /**
      * @param cipher the 128-bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      OCB_Encryption(BlockCipher* cipher, size_t tag_size = 16) :
         OCB_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   private:
      void encrypt(uint8_t input[], size_t blocks);
   };

class BOTAN_DLL OCB_Decryption final : public OCB_Mode
   {
   public:
      /**
      * @param cipher the 128-bit block cipher to use
      * @param tag_size is how big the auth tag will be
      */
      OCB_Decryption(BlockCipher* cipher, size_t tag_size = 16) :
         OCB_Mode(cipher, tag_size) {}

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length >= tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;
   private:
      void decrypt(uint8_t input[], size_t blocks);
   };

}


namespace Botan {

namespace OCSP {

class BOTAN_DLL CertID final : public ASN1_Object
   {
   public:
      CertID() {}

      CertID(const X509_Certificate& issuer,
             const X509_Certificate& subject);

      bool is_id_for(const X509_Certificate& issuer,
                     const X509_Certificate& subject) const;

      void encode_into(class DER_Encoder& to) const override;

      void decode_from(class BER_Decoder& from) override;

      const std::vector<uint8_t>& issuer_key_hash() const { return m_issuer_key_hash; }

   private:
      AlgorithmIdentifier m_hash_id;
      std::vector<uint8_t> m_issuer_dn_hash;
      std::vector<uint8_t> m_issuer_key_hash;
      BigInt m_subject_serial;
   };

class BOTAN_DLL SingleResponse final : public ASN1_Object
   {
   public:
      const CertID& certid() const { return m_certid; }

      size_t cert_status() const { return m_cert_status; }

      X509_Time this_update() const { return m_thisupdate; }

      X509_Time next_update() const { return m_nextupdate; }

      void encode_into(class DER_Encoder& to) const override;

      void decode_from(class BER_Decoder& from) override;
   private:
      CertID m_certid;
      size_t m_cert_status = 2; // unknown
      X509_Time m_thisupdate;
      X509_Time m_nextupdate;
   };

}

}


namespace Botan {

class Certificate_Store;

namespace OCSP {

/**
* An OCSP request.
*/
class BOTAN_DLL Request
   {
   public:
      /**
      * Create an OCSP request.
      * @param issuer_cert issuer certificate
      * @param subject_cert subject certificate
      */
      Request(const X509_Certificate& issuer_cert,
              const X509_Certificate& subject_cert);

      /**
      * @return BER-encoded OCSP request
      */
      std::vector<uint8_t> BER_encode() const;

      /**
      * @return Base64-encoded OCSP request
      */
      std::string base64_encode() const;

      /**
      * @return issuer certificate
      */
      const X509_Certificate& issuer() const { return m_issuer; }

      /**
      * @return subject certificate
      */
      const X509_Certificate& subject() const { return m_subject; }

      const std::vector<uint8_t>& issuer_key_hash() const
         { return m_certid.issuer_key_hash(); }
   private:
      X509_Certificate m_issuer, m_subject;
      CertID m_certid;
   };

/**
* OCSP response.
*
* Note this class is only usable as an OCSP client
*/
class BOTAN_DLL Response
   {
   public:
      /**
      * Creates an empty OCSP response.
      */
      Response() {}

      /**
      * Parses an OCSP response.
      * @param response_bits response bits received
      */
      Response(const std::vector<uint8_t>& response_bits) :
         Response(response_bits.data(), response_bits.size())
         {}

      /**
      * Parses an OCSP response.
      * @param response_bits response bits received
      * @param response_bits_len length of response in bytes
      */
      Response(const uint8_t response_bits[],
               size_t response_bits_len);

      /**
      * Check signature and return status
      * The optional cert_path is the (already validated!) certificate path of
      * the end entity which is being inquired about
      * @param trust_roots list of certstores containing trusted roots
      * @param cert_path optionally, the (already verified!) certificate path for the certificate
      * this is an OCSP response for. This is necessary to find the correct intermediate CA in
      * some cases.
      */
      Certificate_Status_Code check_signature(const std::vector<Certificate_Store*>& trust_roots,
                                              const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path = {}) const;

      /**
      * Verify that issuer's key signed this response
      * @param issuer certificate of issuer
      * @return if signature valid OCSP_SIGNATURE_OK else an error code
      */
      Certificate_Status_Code verify_signature(const X509_Certificate& issuer) const;

      /**
      * @return the time this OCSP response was supposedly produced at
      */
      const X509_Time& produced_at() const { return m_produced_at; }

      /**
      * @return DN of signer, if provided in response (may be empty)
      */
      const X509_DN& signer_name() const { return m_signer_name; }

      /**
      * @return key hash, if provided in response (may be empty)
      */
      const std::vector<uint8_t>& signer_key_hash() const { return m_key_hash; }

      const std::vector<uint8_t>& raw_bits() const { return m_response_bits; }

      /**
       * Searches the OCSP response for issuer and subject certificate.
       * @param issuer issuer certificate
       * @param subject subject certificate
       * @param ref_time the reference time
       * @return OCSP status code, possible values:
       *         CERT_IS_REVOKED,
       *         OCSP_NOT_YET_VALID,
       *         OCSP_HAS_EXPIRED,
       *         OCSP_RESPONSE_GOOD,
       *         OCSP_BAD_STATUS,
       *         OCSP_CERT_NOT_LISTED
       */
      Certificate_Status_Code status_for(const X509_Certificate& issuer,
                                         const X509_Certificate& subject,
                                         std::chrono::system_clock::time_point ref_time = std::chrono::system_clock::now()) const;

   private:
      std::vector<uint8_t> m_response_bits;
      X509_Time m_produced_at;
      X509_DN m_signer_name;
      std::vector<uint8_t> m_key_hash;
      std::vector<uint8_t> m_tbs_bits;
      AlgorithmIdentifier m_sig_algo;
      std::vector<uint8_t> m_signature;
      std::vector<X509_Certificate> m_certs;

      std::vector<SingleResponse> m_responses;
   };

#if defined(BOTAN_HAS_HTTP_UTIL)

/**
* Makes an online OCSP request via HTTP and returns the OCSP response.
* @param issuer issuer certificate
* @param subject subject certificate
* @param trusted_roots trusted roots for the OCSP response
* @return OCSP response
*/
BOTAN_DLL Response online_check(const X509_Certificate& issuer,
                                const X509_Certificate& subject,
                                Certificate_Store* trusted_roots);

#endif

}

}


namespace Botan {

/**
* Output Feedback Mode
*/
class BOTAN_DLL OFB final : public StreamCipher
   {
   public:
      void cipher(const uint8_t in[], uint8_t out[], size_t length) override;

      void set_iv(const uint8_t iv[], size_t iv_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return (iv_len <= m_cipher->block_size()); }

      Key_Length_Specification key_spec() const override
         {
         return m_cipher->key_spec();
         }

      std::string name() const override;

      OFB* clone() const override
         { return new OFB(m_cipher->clone()); }

      void clear() override;

      /**
      * @param cipher the block cipher to use
      */
      explicit OFB(BlockCipher* cipher);

      void seek(uint64_t offset) override;
   private:
      void key_schedule(const uint8_t key[], size_t key_len) override;

      std::unique_ptr<BlockCipher> m_cipher;
      secure_vector<uint8_t> m_buffer;
      size_t m_buf_pos;
   };

}


namespace Botan {

namespace OIDS {

/**
* Resolve an OID
* @param oid the OID to look up
* @return name associated with this OID
*/
BOTAN_DLL std::string lookup(const OID& oid);

/**
* Find the OID to a name. The lookup will be performed in the
* general OID section of the configuration.
* @param name the name to resolve
* @return OID associated with the specified name
*/
BOTAN_DLL OID lookup(const std::string& name);

/**
* See if an OID exists in the internal table.
* @param oid the oid to check for
* @return true if the oid is registered
*/
inline bool have_oid(const std::string& oid)
   {
   return (lookup(oid).empty() == false);
   }

}

}


namespace Botan {

/**
* Rivest's Package Tranform
* @param rng the random number generator to use
* @param cipher the block cipher to use (aont_package takes ownership)
* @param input the input data buffer
* @param input_len the length of the input data in bytes
* @param output the output data buffer (must be at least
*        input_len + cipher->BLOCK_SIZE bytes long)
*/
void BOTAN_DLL aont_package(RandomNumberGenerator& rng,
                            BlockCipher* cipher,
                            const uint8_t input[], size_t input_len,
                            uint8_t output[]);

/**
* Rivest's Package Tranform (Inversion)
* @param cipher the block cipher to use (aont_package takes ownership)
* @param input the input data buffer
* @param input_len the length of the input data in bytes
* @param output the output data buffer (must be at least
*        input_len - cipher->BLOCK_SIZE bytes long)
*/
void BOTAN_DLL aont_unpackage(BlockCipher* cipher,
                              const uint8_t input[], size_t input_len,
                              uint8_t output[]);

}


namespace Botan {

/**
* Parallel Hashes
*/
class BOTAN_DLL Parallel final : public HashFunction
   {
   public:
      void clear() override;
      std::string name() const override;
      HashFunction* clone() const override;

      size_t output_length() const override;

      /**
      * @param hashes a set of hashes to compute in parallel
      * Takes ownership of all pointers
      */
      explicit Parallel(std::vector<std::unique_ptr<HashFunction>>& hashes);

      Parallel(const Parallel&) = delete;
      Parallel& operator=(const Parallel&) = delete;
   private:
      Parallel() {}

      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;

      std::vector<std::unique_ptr<HashFunction>> m_hashes;
   };

}


namespace Botan {

/**
* Create a password hash using PBKDF2
* @param password the password
* @param rng a random number generator
* @param work_factor how much work to do to slow down guessing attacks
* @param alg_id specifies which PRF to use with PBKDF2
*        0 is HMAC(SHA-1)
*        1 is HMAC(SHA-256)
*        2 is CMAC(Blowfish)
*        3 is HMAC(SHA-384)
*        4 is HMAC(SHA-512)
*        all other values are currently undefined
*/
std::string BOTAN_DLL generate_passhash9(const std::string& password,
                                         RandomNumberGenerator& rng,
                                         uint16_t work_factor = 10,
                                         uint8_t alg_id = 1);

/**
* Check a previously created password hash
* @param password the password to check against
* @param hash the stored hash to check against
*/
bool BOTAN_DLL check_passhash9(const std::string& password,
                               const std::string& hash);

}


namespace Botan {

/**
* Encrypt with PBES2 from PKCS #5 v2.0
* @param key_bits the input
* @param passphrase the passphrase to use for encryption
* @param msec how many milliseconds to run PBKDF2
* @param cipher specifies the block cipher to use to encrypt
* @param digest specifies the PRF to use with PBKDF2 (eg "HMAC(SHA-1)")
* @param rng a random number generator
*/
std::pair<AlgorithmIdentifier, std::vector<uint8_t>>
BOTAN_DLL pbes2_encrypt(const secure_vector<uint8_t>& key_bits,
                        const std::string& passphrase,
                        std::chrono::milliseconds msec,
                        const std::string& cipher,
                        const std::string& digest,
                        RandomNumberGenerator& rng);

/**
* Decrypt a PKCS #5 v2.0 encrypted stream
* @param key_bits the input
* @param passphrase the passphrase to use for decryption
* @param params the PBES2 parameters
*/
secure_vector<uint8_t>
BOTAN_DLL pbes2_decrypt(const secure_vector<uint8_t>& key_bits,
                        const std::string& passphrase,
                        const std::vector<uint8_t>& params);

}


namespace Botan {

/**
* Base class for PBKDF (password based key derivation function)
* implementations. Converts a password into a key using a salt
* and iterated hashing to make brute force attacks harder.
*/
class BOTAN_DLL PBKDF
   {
   public:
      /**
      * Create an instance based on a name
      * If provider is empty then best available is chosen.
      * @param algo_spec algorithm name
      * @param provider provider implementation to choose
      * @return a null pointer if the algo/provider combination cannot be found
      */
      static std::unique_ptr<PBKDF> create(const std::string& algo_spec,
                                           const std::string& provider = "");

      /**
      * @return list of available providers for this algorithm, empty if not available
      */
      static std::vector<std::string> providers(const std::string& algo_spec);

      /**
      * @return new instance of this same algorithm
      */
      virtual PBKDF* clone() const = 0;

      /**
      * @return name of this PBKDF
      */
      virtual std::string name() const = 0;

      virtual ~PBKDF();

      /**
      * Derive a key from a passphrase for a number of iterations
      * specified by either iterations or if iterations == 0 then
      * running until msec time has elapsed.
      *
      * @param out buffer to store the derived key, must be of out_len bytes
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param iterations the number of iterations to use (use 10K or more)
      * @param msec if iterations is zero, then instead the PBKDF is
      *        run until msec milliseconds has passed.
      * @return the number of iterations performed
      */
      virtual size_t pbkdf(uint8_t out[], size_t out_len,
                           const std::string& passphrase,
                           const uint8_t salt[], size_t salt_len,
                           size_t iterations,
                           std::chrono::milliseconds msec) const = 0;

      /**
      * Derive a key from a passphrase for a number of iterations.
      *
      * @param out buffer to store the derived key, must be of out_len bytes
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param iterations the number of iterations to use (use 10K or more)
      */
      void pbkdf_iterations(uint8_t out[], size_t out_len,
                            const std::string& passphrase,
                            const uint8_t salt[], size_t salt_len,
                            size_t iterations) const;

      /**
      * Derive a key from a passphrase, running until msec time has elapsed.
      *
      * @param out buffer to store the derived key, must be of out_len bytes
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param msec if iterations is zero, then instead the PBKDF is
      *        run until msec milliseconds has passed.
      * @param iterations set to the number iterations executed
      */
      void pbkdf_timed(uint8_t out[], size_t out_len,
                         const std::string& passphrase,
                         const uint8_t salt[], size_t salt_len,
                         std::chrono::milliseconds msec,
                         size_t& iterations) const;

      /**
      * Derive a key from a passphrase for a number of iterations.
      *
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param iterations the number of iterations to use (use 10K or more)
      * @return the derived key
      */
      secure_vector<uint8_t> pbkdf_iterations(size_t out_len,
                                           const std::string& passphrase,
                                           const uint8_t salt[], size_t salt_len,
                                           size_t iterations) const;

      /**
      * Derive a key from a passphrase, running until msec time has elapsed.
      *
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param msec if iterations is zero, then instead the PBKDF is
      *        run until msec milliseconds has passed.
      * @param iterations set to the number iterations executed
      * @return the derived key
      */
      secure_vector<uint8_t> pbkdf_timed(size_t out_len,
                                      const std::string& passphrase,
                                      const uint8_t salt[], size_t salt_len,
                                      std::chrono::milliseconds msec,
                                      size_t& iterations) const;

      // Following kept for compat with 1.10:

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param iterations the number of iterations to use (use 10K or more)
      */
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const uint8_t salt[], size_t salt_len,
                             size_t iterations) const
         {
         return pbkdf_iterations(out_len, passphrase, salt, salt_len, iterations);
         }

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param iterations the number of iterations to use (use 10K or more)
      */
      template<typename Alloc>
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const std::vector<uint8_t, Alloc>& salt,
                             size_t iterations) const
         {
         return pbkdf_iterations(out_len, passphrase, salt.data(), salt.size(), iterations);
         }

      /**
      * Derive a key from a passphrase
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param salt_len length of salt in bytes
      * @param msec is how long to run the PBKDF
      * @param iterations is set to the number of iterations used
      */
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const uint8_t salt[], size_t salt_len,
                             std::chrono::milliseconds msec,
                             size_t& iterations) const
         {
         return pbkdf_timed(out_len, passphrase, salt, salt_len, msec, iterations);
         }

      /**
      * Derive a key from a passphrase using a certain amount of time
      * @param out_len the desired length of the key to produce
      * @param passphrase the password to derive the key from
      * @param salt a randomly chosen salt
      * @param msec is how long to run the PBKDF
      * @param iterations is set to the number of iterations used
      */
      template<typename Alloc>
      OctetString derive_key(size_t out_len,
                             const std::string& passphrase,
                             const std::vector<uint8_t, Alloc>& salt,
                             std::chrono::milliseconds msec,
                             size_t& iterations) const
         {
         return pbkdf_timed(out_len, passphrase, salt.data(), salt.size(), msec, iterations);
         }
   };

/**
* Password based key derivation function factory method
* @param algo_spec the name of the desired PBKDF algorithm
* @param provider the provider to use
* @return pointer to newly allocated object of that type
*/
inline PBKDF* get_pbkdf(const std::string& algo_spec,
                           const std::string& provider = "")
   {
   std::unique_ptr<PBKDF> p(PBKDF::create(algo_spec, provider));
   if(p)
      return p.release();
   throw Algorithm_Not_Found(algo_spec);
   }

}


namespace Botan {

/**
* PKCS #5 v1 PBKDF, aka PBKDF1
* Can only generate a key up to the size of the hash output.
* Unless needed for backwards compatibility, use PKCS5_PBKDF2
*/
class BOTAN_DLL PKCS5_PBKDF1 final : public PBKDF
   {
   public:
      /**
      * Create a PKCS #5 instance using the specified hash function.
      * @param hash pointer to a hash function object to use
      */
      explicit PKCS5_PBKDF1(HashFunction* hash) : m_hash(hash) {}

      std::string name() const override
         {
         return "PBKDF1(" + m_hash->name() + ")";
         }

      PBKDF* clone() const override
         {
         return new PKCS5_PBKDF1(m_hash->clone());
         }

      size_t pbkdf(uint8_t output_buf[], size_t output_len,
                           const std::string& passphrase,
                           const uint8_t salt[], size_t salt_len,
                           size_t iterations,
                           std::chrono::milliseconds msec) const override;
   private:
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

BOTAN_DLL size_t pbkdf2(MessageAuthenticationCode& prf,
                        uint8_t out[],
                        size_t out_len,
                        const std::string& passphrase,
                        const uint8_t salt[], size_t salt_len,
                        size_t iterations,
                        std::chrono::milliseconds msec);

/**
* PKCS #5 PBKDF2
*/
class BOTAN_DLL PKCS5_PBKDF2 final : public PBKDF
   {
   public:
      std::string name() const override
         {
         return "PBKDF2(" + m_mac->name() + ")";
         }

      PBKDF* clone() const override
         {
         return new PKCS5_PBKDF2(m_mac->clone());
         }

      size_t pbkdf(uint8_t output_buf[], size_t output_len,
                   const std::string& passphrase,
                   const uint8_t salt[], size_t salt_len,
                   size_t iterations,
                   std::chrono::milliseconds msec) const override;

      /**
      * Create a PKCS #5 instance using the specified message auth code
      * @param mac_fn the MAC object to use as PRF
      */
      explicit PKCS5_PBKDF2(MessageAuthenticationCode* mac_fn) : m_mac(mac_fn) {}
   private:
      std::unique_ptr<MessageAuthenticationCode> m_mac;
   };

}


namespace Botan {

namespace PEM_Code {

/**
* Encode some binary data in PEM format
* @param data binary data to encode
* @param data_len length of binary data in bytes
* @param label PEM label put after BEGIN and END
* @param line_width after this many characters, a new line is inserted
*/
BOTAN_DLL std::string encode(const uint8_t data[],
                             size_t data_len,
                             const std::string& label,
                             size_t line_width = 64);

/**
* Encode some binary data in PEM format
* @param data binary data to encode
* @param label PEM label
* @param line_width after this many characters, a new line is inserted
*/
inline std::string encode(const std::vector<uint8_t>& data,
                          const std::string& label,
                          size_t line_width = 64)
   {
   return encode(data.data(), data.size(), label, line_width);
   }

/**
* Encode some binary data in PEM format
* @param data binary data to encode
* @param label PEM label put after BEGIN and END
* @param line_width after this many characters, a new line is inserted
*/
inline std::string encode(const secure_vector<uint8_t>& data,
                          const std::string& label,
                          size_t line_width = 64)
   {
   return encode(data.data(), data.size(), label, line_width);
   }

/**
* Decode PEM data
* @param pem a datasource containing PEM encoded data
* @param label is set to the PEM label found for later inspection
*/
BOTAN_DLL secure_vector<uint8_t> decode(DataSource& pem,
                                     std::string& label);

/**
* Decode PEM data
* @param pem a string containing PEM encoded data
* @param label is set to the PEM label found for later inspection
*/
BOTAN_DLL secure_vector<uint8_t> decode(const std::string& pem,
                                     std::string& label);

/**
* Decode PEM data
* @param pem a datasource containing PEM encoded data
* @param label is what we expect the label to be
*/
BOTAN_DLL secure_vector<uint8_t> decode_check_label(
   DataSource& pem,
   const std::string& label);

/**
* Decode PEM data
* @param pem a string containing PEM encoded data
* @param label is what we expect the label to be
*/
BOTAN_DLL secure_vector<uint8_t> decode_check_label(
   const std::string& pem,
   const std::string& label);

/**
* Heuristic test for PEM data.
*/
BOTAN_DLL bool matches(DataSource& source,
                       const std::string& extra = "",
                       size_t search_range = 4096);

}

}


namespace Botan {

BOTAN_DLL std::unique_ptr<Public_Key>
load_public_key(const AlgorithmIdentifier& alg_id,
                const std::vector<uint8_t>& key_bits);

BOTAN_DLL std::unique_ptr<Private_Key>
load_private_key(const AlgorithmIdentifier& alg_id,
                 const secure_vector<uint8_t>& key_bits);

/**
* Create a new key
* For ECC keys, algo_params specifies EC group (eg, "secp256r1")
* For DH/DSA/ElGamal keys, algo_params is DL group (eg, "modp/ietf/2048")
* For RSA, algo_params is integer keylength
* For McEliece, algo_params is n,t
* If algo_params is left empty, suitable default parameters are chosen.
*/
BOTAN_DLL std::unique_ptr<Private_Key>
create_private_key(const std::string& algo_name,
                   RandomNumberGenerator& rng,
                   const std::string& algo_params = "");

}

/**
* Ordinary applications should never need to include or use this
* header. It is exposed only for specialized applications which want
* to implement new versions of public key crypto without merging them
* as changes to the library. One actual example of such usage is an
* application which creates RSA signatures using a custom TPM library.
* Unless you're doing something like that, you don't need anything
* here. Instead use pubkey.h which wraps these types safely and
* provides a stable application-oriented API.
*/


namespace Botan {

class EME;
class KDF;
class EMSA;

namespace PK_Ops {

/**
* Public key encryption interface
*/
class BOTAN_DLL Encryption
   {
   public:
      virtual secure_vector<uint8_t> encrypt(const uint8_t msg[],
                                          size_t msg_len,
                                          RandomNumberGenerator& rng) = 0;

      virtual size_t max_input_bits() const = 0;

      virtual ~Encryption() {}
   };

/**
* Public key decryption interface
*/
class BOTAN_DLL Decryption
   {
   public:
      virtual secure_vector<uint8_t> decrypt(uint8_t& valid_mask,
                                          const uint8_t ciphertext[],
                                          size_t ciphertext_len) = 0;

      virtual ~Decryption() {}
   };

/**
* Public key signature verification interface
*/
class BOTAN_DLL Verification
   {
   public:
      /*
      * Add more data to the message currently being signed
      * @param msg the message
      * @param msg_len the length of msg in bytes
      */
      virtual void update(const uint8_t msg[], size_t msg_len) = 0;

      /*
      * Perform a verification operation
      * @param rng a random number generator
      */
      virtual bool is_valid_signature(const uint8_t sig[], size_t sig_len) = 0;

      virtual ~Verification() {}
   };

/**
* Public key signature creation interface
*/
class BOTAN_DLL Signature
   {
   public:
      /*
      * Add more data to the message currently being signed
      * @param msg the message
      * @param msg_len the length of msg in bytes
      */
      virtual void update(const uint8_t msg[], size_t msg_len) = 0;

      /*
      * Perform a signature operation
      * @param rng a random number generator
      */
      virtual secure_vector<uint8_t> sign(RandomNumberGenerator& rng) = 0;

      virtual ~Signature() {}
   };

/**
* A generic key agreement operation (eg DH or ECDH)
*/
class BOTAN_DLL Key_Agreement
   {
   public:
      virtual secure_vector<uint8_t> agree(size_t key_len,
                                        const uint8_t other_key[], size_t other_key_len,
                                        const uint8_t salt[], size_t salt_len) = 0;

      virtual ~Key_Agreement() {}
   };

/**
* KEM (key encapsulation)
*/
class BOTAN_DLL KEM_Encryption
   {
   public:
      virtual void kem_encrypt(secure_vector<uint8_t>& out_encapsulated_key,
                               secure_vector<uint8_t>& out_shared_key,
                               size_t desired_shared_key_len,
                               Botan::RandomNumberGenerator& rng,
                               const uint8_t salt[],
                               size_t salt_len) = 0;

      virtual ~KEM_Encryption() {}
   };

class BOTAN_DLL KEM_Decryption
   {
   public:
      virtual secure_vector<uint8_t> kem_decrypt(const uint8_t encap_key[],
                                              size_t len,
                                              size_t desired_shared_key_len,
                                              const uint8_t salt[],
                                              size_t salt_len) = 0;

      virtual ~KEM_Decryption() {}
   };

}

}


namespace Botan {

/**
* PKCS #10 Certificate Request.
*/
class BOTAN_DLL PKCS10_Request final : public X509_Object
   {
   public:
      /**
      * Get the subject public key.
      * @return subject public key
      */
      Public_Key* subject_public_key() const;

      /**
      * Get the raw DER encoded public key.
      * @return raw DER encoded public key
      */
      std::vector<uint8_t> raw_public_key() const;

      /**
      * Get the subject DN.
      * @return subject DN
      */
      X509_DN subject_dn() const;

      /**
      * Get the subject alternative name.
      * @return subject alternative name.
      */
      AlternativeName subject_alt_name() const;

      /**
      * Get the key constraints for the key associated with this
      * PKCS#10 object.
      * @return key constraints
      */
      Key_Constraints constraints() const;

      /**
      * Get the extendend key constraints (if any).
      * @return extended key constraints
      */
      std::vector<OID> ex_constraints() const;

      /**
      * Find out whether this is a CA request.
      * @result true if it is a CA request, false otherwise.
      */
      bool is_CA() const;

      /**
      * Return the constraint on the path length defined
      * in the BasicConstraints extension.
      * @return path limit
      */
      size_t path_limit() const;

      /**
      * Get the challenge password for this request
      * @return challenge password for this request
      */
      std::string challenge_password() const;

      /**
      * Get the X509v3 extensions.
      * @return X509v3 extensions
      */
      Extensions extensions() const;

      /**
      * Create a PKCS#10 Request from a data source.
      * @param source the data source providing the DER encoded request
      */
      explicit PKCS10_Request(DataSource& source);

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)
      /**
      * Create a PKCS#10 Request from a file.
      * @param filename the name of the file containing the DER or PEM
      * encoded request file
      */
      explicit PKCS10_Request(const std::string& filename);
#endif

      /**
      * Create a PKCS#10 Request from binary data.
      * @param vec a std::vector containing the DER value
      */
      explicit PKCS10_Request(const std::vector<uint8_t>& vec);
   private:
      void force_decode() override;
      void handle_attribute(const Attribute&);

      Data_Store m_info;
      Extensions m_extensions;
   };

}


namespace Botan {

/**
* PKCS #8 General Exception
*/
struct BOTAN_DLL PKCS8_Exception : public Decoding_Error
   {
   explicit PKCS8_Exception(const std::string& error) :
      Decoding_Error("PKCS #8: " + error) {}
   };

/**
* This namespace contains functions for handling PKCS #8 private keys
*/
namespace PKCS8 {

/**
* BER encode a private key
* @param key the private key to encode
* @return BER encoded key
*/
BOTAN_DLL secure_vector<uint8_t> BER_encode(const Private_Key& key);

/**
* Get a string containing a PEM encoded private key.
* @param key the key to encode
* @return encoded key
*/
BOTAN_DLL std::string PEM_encode(const Private_Key& key);

/**
* Encrypt a key using PKCS #8 encryption
* @param key the key to encode
* @param rng the rng to use
* @param pass the password to use for encryption
* @param msec number of milliseconds to run the password derivation
* @param pbe_algo the name of the desired password-based encryption
         algorithm; if empty ("") a reasonable (portable/secure)
         default will be chosen.
* @return encrypted key in binary BER form
*/
BOTAN_DLL std::vector<uint8_t>
BER_encode(const Private_Key& key,
           RandomNumberGenerator& rng,
           const std::string& pass,
           std::chrono::milliseconds msec = std::chrono::milliseconds(300),
           const std::string& pbe_algo = "");

/**
* Get a string containing a PEM encoded private key, encrypting it with a
* password.
* @param key the key to encode
* @param rng the rng to use
* @param pass the password to use for encryption
* @param msec number of milliseconds to run the password derivation
* @param pbe_algo the name of the desired password-based encryption
         algorithm; if empty ("") a reasonable (portable/secure)
         default will be chosen.
* @return encrypted key in PEM form
*/
BOTAN_DLL std::string
PEM_encode(const Private_Key& key,
           RandomNumberGenerator& rng,
           const std::string& pass,
           std::chrono::milliseconds msec = std::chrono::milliseconds(300),
           const std::string& pbe_algo = "");

/**
* Load an encrypted key from a data source.
* @param source the data source providing the encoded key
* @param rng ignored for compatability
* @param get_passphrase a function that returns passphrases
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(DataSource& source,
                                RandomNumberGenerator& rng,
                                std::function<std::string ()> get_passphrase);

/** Load an encrypted key from a data source.
* @param source the data source providing the encoded key
* @param rng ignored for compatability
* @param pass the passphrase to decrypt the key
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(DataSource& source,
                                RandomNumberGenerator& rng,
                                const std::string& pass);

/** Load an unencrypted key from a data source.
* @param source the data source providing the encoded key
* @param rng ignored for compatability
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(DataSource& source,
                                RandomNumberGenerator& rng);

#if defined(BOTAN_TARGET_OS_HAS_FILESYSTEM)
/**
* Load an encrypted key from a file.
* @param filename the path to the file containing the encoded key
* @param rng ignored for compatability
* @param get_passphrase a function that returns passphrases
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(const std::string& filename,
                                RandomNumberGenerator& rng,
                                std::function<std::string ()> get_passphrase);

/** Load an encrypted key from a file.
* @param filename the path to the file containing the encoded key
* @param rng ignored for compatability
* @param pass the passphrase to decrypt the key
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(const std::string& filename,
                                RandomNumberGenerator& rng,
                                const std::string& pass);

/** Load an unencrypted key from a file.
* @param filename the path to the file containing the encoded key
* @param rng ignored for compatability
* @return loaded private key object
*/
BOTAN_DLL Private_Key* load_key(const std::string& filename,
                                RandomNumberGenerator& rng);
#endif

/**
* Copy an existing encoded key object.
* @param key the key to copy
* @param rng ignored for compatability
* @return new copy of the key
*/
BOTAN_DLL Private_Key* copy_key(const Private_Key& key,
                                RandomNumberGenerator& rng);

}

}


namespace Botan {

/**
* DJB's Poly1305
* Important note: each key can only be used once
*/
class BOTAN_DLL Poly1305 final : public MessageAuthenticationCode
   {
   public:
      std::string name() const override { return "Poly1305"; }

      MessageAuthenticationCode* clone() const override { return new Poly1305; }

      void clear() override;

      size_t output_length() const override { return 16; }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(32);
         }

   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      void key_schedule(const uint8_t[], size_t) override;

      secure_vector<uint64_t> m_poly;
      secure_vector<uint8_t> m_buf;
      size_t m_buf_pos = 0;
   };

}


namespace Botan {

/**
* PRF used in TLS 1.0/1.1
*/
class BOTAN_DLL TLS_PRF final : public KDF
   {
   public:
      std::string name() const override { return "TLS-PRF"; }

      KDF* clone() const override { return new TLS_PRF; }

      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

      TLS_PRF();
   private:
      std::unique_ptr<MessageAuthenticationCode> m_hmac_md5;
      std::unique_ptr<MessageAuthenticationCode> m_hmac_sha1;
   };

/**
* PRF used in TLS 1.2
*/
class BOTAN_DLL TLS_12_PRF final : public KDF
   {
   public:
      std::string name() const override { return "TLS-12-PRF(" + m_mac->name() + ")"; }

      KDF* clone() const override { return new TLS_12_PRF(m_mac->clone()); }

      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

      /**
      * @param mac MAC algorithm to use
      */
      explicit TLS_12_PRF(MessageAuthenticationCode* mac) : m_mac(mac) {}
   private:
      std::unique_ptr<MessageAuthenticationCode> m_mac;
   };

}


namespace Botan {

/**
* PRF from ANSI X9.42
*/
class BOTAN_DLL X942_PRF final : public KDF
   {
   public:
      std::string name() const override { return "X9.42-PRF(" + m_key_wrap_oid + ")"; }

      KDF* clone() const override { return new X942_PRF(m_key_wrap_oid); }

      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

      explicit X942_PRF(const std::string& oid);
   private:
      std::string m_key_wrap_oid;
   };

}


namespace Botan {

/**
* PSSR (called EMSA4 in IEEE 1363 and in old versions of the library)
*/
class BOTAN_DLL PSSR final : public EMSA
   {
   public:

      /**
      * @param hash the hash function to use
      */
      explicit PSSR(HashFunction* hash);

      /**
      * @param hash the hash function to use
      * @param salt_size the size of the salt to use in bytes
      */
      PSSR(HashFunction* hash, size_t salt_size);

      EMSA* clone() override { return new PSSR(m_hash->clone(), m_SALT_SIZE); }
   private:
      void update(const uint8_t input[], size_t length) override;

      secure_vector<uint8_t> raw_data() override;

      secure_vector<uint8_t> encoding_of(const secure_vector<uint8_t>& msg,
                                      size_t output_bits,
                                      RandomNumberGenerator& rng) override;

      bool verify(const secure_vector<uint8_t>& coded,
                  const secure_vector<uint8_t>& raw,
                  size_t key_bits) override;

      size_t m_SALT_SIZE;
      std::unique_ptr<HashFunction> m_hash;
   };

}


namespace Botan {

/**
* RC4 stream cipher
*/
class BOTAN_DLL RC4 final : public StreamCipher
   {
   public:
      void cipher(const uint8_t in[], uint8_t out[], size_t length) override;

      void set_iv(const uint8_t iv[], size_t iv_len) override;

      void clear() override;
      std::string name() const override;

      StreamCipher* clone() const override { return new RC4(m_SKIP); }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(1, 256);
         }

      /**
      * @param skip skip this many initial bytes in the keystream
      */
      explicit RC4(size_t skip = 0);

      ~RC4() { clear(); }

      void seek(uint64_t offset) override;
   private:
      void key_schedule(const uint8_t[], size_t) override;
      void generate();

      const size_t m_SKIP;
      uint8_t m_X = 0;
      uint8_t m_Y = 0;
      secure_vector<uint8_t> m_state;
      secure_vector<uint8_t> m_buffer;
      size_t m_position = 0;
   };

}


namespace Botan {

class BOTAN_DLL RDRAND_RNG : public Hardware_RNG
   {
   public:
      /**
      * On correctly working hardware, RDRAND is always supposed to
      * succeed within a set number of retries. If after that many
      * retries RDRAND has still not suceeded, sets ok = false and
      * returns 0.
      */
      static uint32_t rdrand_status(bool& ok);

      /*
      * Calls RDRAND until it succeeds, this could hypothetically
      * loop forever on broken hardware.
      */
      static uint32_t rdrand();

      /**
      * Constructor will throw if CPU does not have RDRAND bit set
      */
      RDRAND_RNG();

      /**
      * Uses RDRAND to produce output
      */
      void randomize(uint8_t out[], size_t out_len) override;

      /*
      * No way to provide entropy to RDRAND generator, so add_entropy is ignored
      */
      void add_entropy(const uint8_t[], size_t) override
         { /* no op */ }

      /*
      * No way to reseed RDRAND generator, so reseed is ignored
      */
      size_t reseed(Entropy_Sources&, size_t, std::chrono::milliseconds) override
         { return 0; /* no op */ }

      std::string name() const override { return "RDRAND"; }

      bool is_seeded() const override { return true; }

      void clear() override {}
   };

}


namespace Botan {

/**
* Encrypt a key under a key encryption key using the algorithm
* described in RFC 3394
*
* @param key the plaintext key to encrypt
* @param kek the key encryption key
* @return key encrypted under kek
*/
secure_vector<uint8_t> BOTAN_DLL rfc3394_keywrap(const secure_vector<uint8_t>& key,
                                              const SymmetricKey& kek);

/**
* Decrypt a key under a key encryption key using the algorithm
* described in RFC 3394
*
* @param key the encrypted key to decrypt
* @param kek the key encryption key
* @return key decrypted under kek
*/
secure_vector<uint8_t> BOTAN_DLL rfc3394_keyunwrap(const secure_vector<uint8_t>& key,
                                                const SymmetricKey& kek);

}


namespace Botan {

class HMAC_DRBG;

class BOTAN_DLL RFC6979_Nonce_Generator
   {
   public:
      /**
      * Note: keeps persistent reference to order
      */
      RFC6979_Nonce_Generator(const std::string& hash,
                              const BigInt& order,
                              const BigInt& x);

      ~RFC6979_Nonce_Generator();

      const BigInt& nonce_for(const BigInt& m);
   private:
      const BigInt& m_order;
      BigInt m_k;
      size_t m_qlen, m_rlen;
      std::unique_ptr<HMAC_DRBG> m_hmac_drbg;
      secure_vector<uint8_t> m_rng_in, m_rng_out;
   };

/**
* @param x the secret (EC)DSA key
* @param q the group order
* @param h the message hash already reduced mod q
* @param hash the hash function used to generate h
*/
BigInt BOTAN_DLL generate_rfc6979_nonce(const BigInt& x,
                                        const BigInt& q,
                                        const BigInt& h,
                                        const std::string& hash);

}


namespace Botan {

/**
* RIPEMD-160
*/
class BOTAN_DLL RIPEMD_160 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "RIPEMD-160"; }
      size_t output_length() const override { return 20; }
      HashFunction* clone() const override { return new RIPEMD_160; }

      void clear() override;

      RIPEMD_160() : MDx_HashFunction(64, false, true), m_M(16), m_digest(5)
         { clear(); }
   private:
      void compress_n(const uint8_t[], size_t blocks) override;
      void copy_out(uint8_t[]) override;

      secure_vector<uint32_t> m_M, m_digest;
   };

}


namespace Botan {

/**
* RSA Public Key
*/
class BOTAN_DLL RSA_PublicKey : public virtual Public_Key
   {
   public:
      /**
      * Load a public key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits DER encoded public key bits
      */
      RSA_PublicKey(const AlgorithmIdentifier& alg_id,
                    const std::vector<uint8_t>& key_bits);

      /**
      * Create a public key.
      * @arg n the modulus
      * @arg e the exponent
      */
      RSA_PublicKey(const BigInt& n, const BigInt& e) :
         m_n(n), m_e(e) {}

      std::string algo_name() const override { return "RSA"; }

      bool check_key(RandomNumberGenerator& rng, bool) const override;

      AlgorithmIdentifier algorithm_identifier() const override;

      std::vector<uint8_t> public_key_bits() const override;

      /**
      * @return public modulus
      */
      const BigInt& get_n() const { return m_n; }

      /**
      * @return public exponent
      */
      const BigInt& get_e() const { return m_e; }

      size_t key_length() const override;
      size_t estimated_strength() const override;

      std::unique_ptr<PK_Ops::Encryption>
         create_encryption_op(RandomNumberGenerator& rng,
                              const std::string& params,
                              const std::string& provider) const override;

      std::unique_ptr<PK_Ops::KEM_Encryption>
         create_kem_encryption_op(RandomNumberGenerator& rng,
                                  const std::string& params,
                                  const std::string& provider) const override;

      std::unique_ptr<PK_Ops::Verification>
         create_verification_op(const std::string& params,
                                const std::string& provider) const override;

   protected:
      RSA_PublicKey() {}

      BigInt m_n, m_e;
   };

/**
* RSA Private Key
*/
class BOTAN_DLL RSA_PrivateKey : public Private_Key, public RSA_PublicKey
   {
   public:
      /**
      * Load a private key.
      * @param alg_id the X.509 algorithm identifier
      * @param key_bits PKCS #8 structure
      */
      RSA_PrivateKey(const AlgorithmIdentifier& alg_id,
                     const secure_vector<uint8_t>& key_bits);

      /**
      * Construct a private key from the specified parameters.
      * @param p the first prime
      * @param q the second prime
      * @param e the exponent
      * @param d if specified, this has to be d with
      * exp * d = 1 mod (p - 1, q - 1). Leave it as 0 if you wish to
      * the constructor to calculate it.
      * @param n if specified, this must be n = p * q. Leave it as 0
      * if you wish to the constructor to calculate it.
      */
      RSA_PrivateKey(const BigInt& p, const BigInt& q,
                     const BigInt& e, const BigInt& d = 0,
                     const BigInt& n = 0);

      /**
      * Create a new private key with the specified bit length
      * @param rng the random number generator to use
      * @param bits the desired bit length of the private key
      * @param exp the public exponent to be used
      */
      RSA_PrivateKey(RandomNumberGenerator& rng,
                     size_t bits, size_t exp = 65537);

      bool check_key(RandomNumberGenerator& rng, bool) const override;

      /**
      * Get the first prime p.
      * @return prime p
      */
      const BigInt& get_p() const { return m_p; }

      /**
      * Get the second prime q.
      * @return prime q
      */
      const BigInt& get_q() const { return m_q; }

      /**
      * Get d with exp * d = 1 mod (p - 1, q - 1).
      * @return d
      */
      const BigInt& get_d() const { return m_d; }

      const BigInt& get_c() const { return m_c; }
      const BigInt& get_d1() const { return m_d1; }
      const BigInt& get_d2() const { return m_d2; }

      secure_vector<uint8_t> private_key_bits() const override;

      std::unique_ptr<PK_Ops::Decryption>
         create_decryption_op(RandomNumberGenerator& rng,
                              const std::string& params,
                              const std::string& provider) const override;

      std::unique_ptr<PK_Ops::KEM_Decryption>
         create_kem_decryption_op(RandomNumberGenerator& rng,
                                  const std::string& params,
                                  const std::string& provider) const override;

      std::unique_ptr<PK_Ops::Signature>
         create_signature_op(RandomNumberGenerator& rng,
                             const std::string& params,
                             const std::string& provider) const override;

   private:
      BigInt m_d, m_p, m_q, m_d1, m_d2, m_c;
   };

}


namespace Botan {

/**
* DJB's Salsa20 (and XSalsa20)
*/
class BOTAN_DLL Salsa20 final : public StreamCipher
   {
   public:
      void cipher(const uint8_t in[], uint8_t out[], size_t length) override;

      void set_iv(const uint8_t iv[], size_t iv_len) override;

      bool valid_iv_length(size_t iv_len) const override
         { return (iv_len == 0 || iv_len == 8 || iv_len == 24); }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(16, 32, 16);
         }

      void clear() override;
      std::string name() const override;
      StreamCipher* clone() const override { return new Salsa20; }

      void seek(uint64_t offset) override;
   private:
      void key_schedule(const uint8_t key[], size_t key_len) override;

      secure_vector<uint32_t> m_state;
      secure_vector<uint8_t> m_buffer;
      size_t m_position = 0;
   };

}


namespace Botan {

/**
A class encapsulating a SCAN name (similar to JCE conventions)
http://www.users.zetnet.co.uk/hopwood/crypto/scan/
*/
class BOTAN_DLL SCAN_Name
   {
   public:
      /**
      * Create a SCAN_Name
      * @param algo_spec A SCAN-format name
      */
      explicit SCAN_Name(const char* algo_spec);

      /**
      * Create a SCAN_Name
      * @param algo_spec A SCAN-format name
      */
      explicit SCAN_Name(std::string algo_spec);

      /**
      * @return original input string
      */
      const std::string& as_string() const { return m_orig_algo_spec; }

      /**
      * @return algorithm name
      */
      const std::string& algo_name() const { return m_alg_name; }

      /**
      * @return number of arguments
      */
      size_t arg_count() const { return m_args.size(); }

      /**
      * @param lower is the lower bound
      * @param upper is the upper bound
      * @return if the number of arguments is between lower and upper
      */
      bool arg_count_between(size_t lower, size_t upper) const
         { return ((arg_count() >= lower) && (arg_count() <= upper)); }

      /**
      * @param i which argument
      * @return ith argument
      */
      std::string arg(size_t i) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument or the default value
      */
      std::string arg(size_t i, const std::string& def_value) const;

      /**
      * @param i which argument
      * @param def_value the default value
      * @return ith argument as an integer, or the default value
      */
      size_t arg_as_integer(size_t i, size_t def_value) const;

      /**
      * @return cipher mode (if any)
      */
      std::string cipher_mode() const
         { return (m_mode_info.size() >= 1) ? m_mode_info[0] : ""; }

      /**
      * @return cipher mode padding (if any)
      */
      std::string cipher_mode_pad() const
         { return (m_mode_info.size() >= 2) ? m_mode_info[1] : ""; }

   private:
      std::string m_orig_algo_spec;
      std::string m_alg_name;
      std::vector<std::string> m_args;
      std::vector<std::string> m_mode_info;
   };

// This is unrelated but it is convenient to stash it here
template<typename T>
std::vector<std::string> probe_providers_of(const std::string& algo_spec,
                                            const std::vector<std::string>& possible)
   {
   std::vector<std::string> providers;
   for(auto&& prov : possible)
      {
      std::unique_ptr<T> o(T::create(algo_spec, prov));
      if(o)
         {
         providers.push_back(prov); // available
         }
      }
   return providers;
   }

}


namespace Botan {

/**
* A queue that knows how to zeroize itself
*/
class BOTAN_DLL SecureQueue : public Fanout_Filter, public DataSource
   {
   public:
      std::string name() const override { return "Queue"; }

      void write(const uint8_t[], size_t) override;

      size_t read(uint8_t[], size_t) override;
      size_t peek(uint8_t[], size_t, size_t = 0) const override;
      size_t get_bytes_read() const override;

      bool end_of_data() const override;

      bool empty() const;

      bool check_available(size_t n) override { return n <= size(); }

      /**
      * @return number of bytes available in the queue
      */
      size_t size() const;

      bool attachable() override { return false; }

      /**
      * SecureQueue assignment
      * @param other the queue to copy
      */
      SecureQueue& operator=(const SecureQueue& other);

      /**
      * SecureQueue default constructor (creates empty queue)
      */
      SecureQueue();

      /**
      * SecureQueue copy constructor
      * @param other the queue to copy
      */
      SecureQueue(const SecureQueue& other);

      ~SecureQueue() { destroy(); }

   private:
      void destroy();
      size_t m_bytes_read;
      class SecureQueueNode* m_head;
      class SecureQueueNode* m_tail;
   };

}


namespace Botan {

/**
* SEED, a Korean block cipher
*/
class BOTAN_DLL SEED final : public Block_Cipher_Fixed_Params<16, 16>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "SEED"; }
      BlockCipher* clone() const override { return new SEED; }
   private:
      void key_schedule(const uint8_t[], size_t) override;

      secure_vector<uint32_t> m_K;
   };

}


namespace Botan {

/**
* Serpent is the most conservative of the AES finalists
* http://www.cl.cam.ac.uk/~rja14/serpent.html
*/
class BOTAN_DLL Serpent final : public Block_Cipher_Fixed_Params<16, 16, 32, 8>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string provider() const override;
      std::string name() const override { return "Serpent"; }
      BlockCipher* clone() const override { return new Serpent; }

      size_t parallelism() const override { return 4; }

   protected:
#if defined(BOTAN_HAS_SERPENT_SIMD)
      /**
      * Encrypt 4 blocks in parallel using SSE2 or AltiVec
      */
      void simd_encrypt_4(const uint8_t in[64], uint8_t out[64]) const;

      /**
      * Decrypt 4 blocks in parallel using SSE2 or AltiVec
      */
      void simd_decrypt_4(const uint8_t in[64], uint8_t out[64]) const;
#endif

      /**
      * For use by subclasses using SIMD, asm, etc
      * @return const reference to the key schedule
      */
      const secure_vector<uint32_t>& get_round_keys() const
         { return m_round_key; }

      /**
      * For use by subclasses that implement the key schedule
      * @param ks is the new key schedule value to set
      */
      void set_round_keys(const uint32_t ks[132])
         {
         m_round_key.assign(&ks[0], &ks[132]);
         }

   private:
      void key_schedule(const uint8_t key[], size_t length) override;
      secure_vector<uint32_t> m_round_key;
   };

}


namespace Botan {

/**
* NIST's SHA-160
*/
class BOTAN_DLL SHA_160 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-160"; }
      size_t output_length() const override { return 20; }
      HashFunction* clone() const override { return new SHA_160; }

      void clear() override;

      SHA_160() : MDx_HashFunction(64, true, true), m_digest(5)
         {
         clear();
         }

   private:
      void compress_n(const uint8_t[], size_t blocks) override;

#if defined(BOTAN_HAS_SHA1_SSE2)
      static void sse2_compress_n(secure_vector<uint32_t>& digest,
                                  const uint8_t blocks[],
                                  size_t block_count);
#endif


      void copy_out(uint8_t[]) override;

      /**
      * The digest value
      */
      secure_vector<uint32_t> m_digest;

      /**
      * The message buffer
      */
      secure_vector<uint32_t> m_W;
   };

typedef SHA_160 SHA_1;

}


namespace Botan {

/**
* SHA-224
*/
class BOTAN_DLL SHA_224 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-224"; }
      size_t output_length() const override { return 28; }
      HashFunction* clone() const override { return new SHA_224; }

      void clear() override;

      SHA_224() : MDx_HashFunction(64, true, true), m_digest(8)
         { clear(); }
   private:
      void compress_n(const uint8_t[], size_t blocks) override;
      void copy_out(uint8_t[]) override;

      secure_vector<uint32_t> m_digest;
   };

/**
* SHA-256
*/
class BOTAN_DLL SHA_256 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-256"; }
      size_t output_length() const override { return 32; }
      HashFunction* clone() const override { return new SHA_256; }

      void clear() override;

      SHA_256() : MDx_HashFunction(64, true, true), m_digest(8)
         { clear(); }
   private:
      void compress_n(const uint8_t[], size_t blocks) override;
      void copy_out(uint8_t[]) override;

      secure_vector<uint32_t> m_digest;
   };

}


namespace Botan {

/**
* SHA-384
*/
class BOTAN_DLL SHA_384 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-384"; }
      size_t output_length() const override { return 48; }
      HashFunction* clone() const override { return new SHA_384; }

      void clear() override;

      SHA_384() : MDx_HashFunction(128, true, true, 16), m_digest(8)
         { clear(); }
   private:
      void compress_n(const uint8_t[], size_t blocks) override;
      void copy_out(uint8_t[]) override;

      secure_vector<uint64_t> m_digest;
   };

/**
* SHA-512
*/
class BOTAN_DLL SHA_512 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-512"; }
      size_t output_length() const override { return 64; }
      HashFunction* clone() const override { return new SHA_512; }

      void clear() override;

      SHA_512() : MDx_HashFunction(128, true, true, 16), m_digest(8)
         { clear(); }
   private:
      void compress_n(const uint8_t[], size_t blocks) override;
      void copy_out(uint8_t[]) override;

      secure_vector<uint64_t> m_digest;
   };

/**
* SHA-512/256
*/
class BOTAN_DLL SHA_512_256 final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "SHA-512-256"; }
      size_t output_length() const override { return 32; }
      HashFunction* clone() const override { return new SHA_512_256; }

      void clear() override;

      SHA_512_256() : MDx_HashFunction(128, true, true, 16), m_digest(8) { clear(); }
   private:
      void compress_n(const uint8_t[], size_t blocks) override;
      void copy_out(uint8_t[]) override;

      secure_vector<uint64_t> m_digest;
   };

}


namespace Botan {

/**
* SHA-3
*/
class BOTAN_DLL SHA_3 : public HashFunction
   {
   public:

      /**
      * @param output_bits the size of the hash output; must be one of
      *                    224, 256, 384, or 512
      */
      SHA_3(size_t output_bits);

      size_t hash_block_size() const override { return m_bitrate / 8; }
      size_t output_length() const override { return m_output_bits / 8; }

      HashFunction* clone() const override;
      std::string name() const override;
      void clear() override;

      // Static functions for internal usage

      /**
      * Absorb data into the provided state
      * @param bitrate the bitrate to absorb into the sponge
      * @param S the sponge state
      * @param S_pos where to begin absorbing into S
      * @param input the input data
      * @param length size of input in bytes
      */
      static size_t absorb(size_t bitrate,
                           secure_vector<uint64_t>& S, size_t S_pos,
                           const uint8_t input[], size_t length);

      /**
      * Expand from provided state
      * @param bitrate sponge parameter
      * @param S the state
      * @param output the output buffer
      * @param output_length the size of output in bytes
      */
      static void expand(size_t bitrate,
                         secure_vector<uint64_t>& S,
                         uint8_t output[], size_t output_length);

      /**
      * The bare Keccak-1600 permutation
      */
      static void permute(uint64_t A[25]);

   private:
      void add_data(const uint8_t input[], size_t length) override;
      void final_result(uint8_t out[]) override;

      size_t m_output_bits, m_bitrate;
      secure_vector<uint64_t> m_S;
      size_t m_S_pos;
   };

/**
* SHA-3-224
*/
class BOTAN_DLL SHA_3_224 final : public SHA_3
   {
   public:
      SHA_3_224() : SHA_3(224) {}
   };

/**
* SHA-3-256
*/
class BOTAN_DLL SHA_3_256 final : public SHA_3
   {
   public:
      SHA_3_256() : SHA_3(256) {}
   };

/**
* SHA-3-384
*/
class BOTAN_DLL SHA_3_384 final : public SHA_3
   {
   public:
      SHA_3_384() : SHA_3(384) {}
   };

/**
* SHA-3-512
*/
class BOTAN_DLL SHA_3_512 final : public SHA_3
   {
   public:
      SHA_3_512() : SHA_3(512) {}
   };

}


namespace Botan {

/**
* SHAKE-128
*/
class BOTAN_DLL SHAKE_128 : public HashFunction
   {
   public:

      /**
      * @param output_bits the desired output size in bits
      * must be a multiple of 8
      */
      SHAKE_128(size_t output_bits);

      size_t hash_block_size() const override { return SHAKE_128_BITRATE / 8; }
      size_t output_length() const override { return m_output_bits / 8; }

      HashFunction* clone() const override;
      std::string name() const override;
      void clear() override;

   private:
      void add_data(const uint8_t input[], size_t length) override;
      void final_result(uint8_t out[]) override;

      static const size_t SHAKE_128_BITRATE = 1600 - 256;

      size_t m_output_bits;
      secure_vector<uint64_t> m_S;
      size_t m_S_pos;
   };

/**
* SHAKE-256
*/
class BOTAN_DLL SHAKE_256 : public HashFunction
   {
   public:

      /**
      * @param output_bits the desired output size in bits
      * must be a multiple of 8
      */
      SHAKE_256(size_t output_bits);

      size_t hash_block_size() const override { return SHAKE_256_BITRATE / 8; }
      size_t output_length() const override { return m_output_bits / 8; }

      HashFunction* clone() const override;
      std::string name() const override;
      void clear() override;

   private:
      void add_data(const uint8_t input[], size_t length) override;
      void final_result(uint8_t out[]) override;

      static const size_t SHAKE_256_BITRATE = 1600 - 512;

      size_t m_output_bits;
      secure_vector<uint64_t> m_S;
      size_t m_S_pos;
   };

}


namespace Botan {

/**
* SHAKE-128 XOF presented as a stream cipher
*/
class BOTAN_DLL SHAKE_128_Cipher final : public StreamCipher
   {
   public:
      SHAKE_128_Cipher();

      /**
      * Produce more XOF output
      */
      void cipher(const uint8_t in[], uint8_t out[], size_t length) override;

      /**
      * Seeking is not supported, this function will throw
      */
      void seek(uint64_t offset) override;

      /**
      * IV not supported, this function will throw unless iv_len == 0
      */
      void set_iv(const uint8_t iv[], size_t iv_len) override;

      bool valid_iv_length(size_t iv_len) const override { return (iv_len == 0); }

      /**
      * In principle SHAKE can accept arbitrary length inputs, but this
      * does not seem required for a stream cipher.
      */
      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(16, 160, 8);
         }

      void clear() override;
      std::string name() const override { return "SHAKE-128"; }
      StreamCipher* clone() const override { return new SHAKE_128_Cipher; }

   private:
      void key_schedule(const uint8_t key[], size_t key_len) override;

      secure_vector<uint64_t> m_state; // internal state
      secure_vector<uint8_t> m_buffer; // ciphertext buffer
      size_t m_buf_pos; // position in m_buffer
   };

}


namespace Botan {

class BOTAN_DLL SipHash final : public MessageAuthenticationCode
   {
   public:
      SipHash(size_t c = 2, size_t d = 4) : m_C(c), m_D(d) {}

      void clear() override;
      std::string name() const override;

      MessageAuthenticationCode* clone() const override;

      size_t output_length() const override { return 8; }

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(16);
         }
   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      void key_schedule(const uint8_t[], size_t) override;

      const size_t m_C, m_D;
      secure_vector<uint64_t> m_V;
      uint64_t m_mbuf = 0;
      size_t m_mbuf_pos = 0;
      uint8_t m_words = 0;
   };

}


namespace Botan {

/**
* Base class for SIV encryption and decryption (@see RFC 5297)
*/
class BOTAN_DLL SIV_Mode : public AEAD_Mode
   {
   public:
      size_t process(uint8_t buf[], size_t size) override;

      /**
      * Sets the nth element of the vector of associated data
      * @param n index into the AD vector
      * @param ad associated data
      * @param ad_len length of associated data in bytes
      */
      void set_associated_data_n(size_t n, const uint8_t ad[], size_t ad_len);

      void set_associated_data(const uint8_t ad[], size_t ad_len) override
         {
         set_associated_data_n(0, ad, ad_len);
         }

      std::string name() const override;

      size_t update_granularity() const override;

      Key_Length_Specification key_spec() const override;

      bool valid_nonce_length(size_t) const override;

      void clear() override;

      void reset() override;

      size_t tag_size() const override { return 16; }

   protected:
      explicit SIV_Mode(BlockCipher* cipher);

      StreamCipher& ctr() { return *m_ctr; }

      void set_ctr_iv(secure_vector<uint8_t> V);

      secure_vector<uint8_t>& msg_buf() { return m_msg_buf; }

      secure_vector<uint8_t> S2V(const uint8_t text[], size_t text_len);
   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;

      void key_schedule(const uint8_t key[], size_t length) override;

      const std::string m_name;
      std::unique_ptr<StreamCipher> m_ctr;
      std::unique_ptr<MessageAuthenticationCode> m_cmac;
      secure_vector<uint8_t> m_nonce, m_msg_buf;
      std::vector<secure_vector<uint8_t>> m_ad_macs;
   };

/**
* SIV Encryption
*/
class BOTAN_DLL SIV_Encryption final : public SIV_Mode
   {
   public:
      /**
      * @param cipher a block cipher
      */
      explicit SIV_Encryption(BlockCipher* cipher) : SIV_Mode(cipher) {}

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override
         { return input_length + tag_size(); }

      size_t minimum_final_size() const override { return 0; }
   };

/**
* SIV Decryption
*/
class BOTAN_DLL SIV_Decryption final : public SIV_Mode
   {
   public:
      /**
      * @param cipher a 128-bit block cipher
      */
      explicit SIV_Decryption(BlockCipher* cipher) : SIV_Mode(cipher) {}

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override
         {
         BOTAN_ASSERT(input_length >= tag_size(), "Sufficient input");
         return input_length - tag_size();
         }

      size_t minimum_final_size() const override { return tag_size(); }
   };

}


namespace Botan {

/**
* Threefish-512
*/
class BOTAN_DLL Threefish_512 final : public Block_Cipher_Fixed_Params<64, 64>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void set_tweak(const uint8_t tweak[], size_t len);

      void clear() override;
      std::string provider() const override;
      std::string name() const override { return "Threefish-512"; }
      BlockCipher* clone() const override { return new Threefish_512; }
   protected:
      const secure_vector<uint64_t>& get_T() const { return m_T; }
      const secure_vector<uint64_t>& get_K() const { return m_K; }
   private:

#if defined(BOTAN_HAS_THREEFISH_512_AVX2)
      void avx2_encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
      void avx2_decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const;
#endif

      void key_schedule(const uint8_t key[], size_t key_len) override;

      // Interface for Skein
      friend class Skein_512;

      virtual void skein_feedfwd(const secure_vector<uint64_t>& M,
                                 const secure_vector<uint64_t>& T);

      // Private data
      secure_vector<uint64_t> m_T;
      secure_vector<uint64_t> m_K;
   };

}


namespace Botan {

/**
* Skein-512, a SHA-3 candidate
*/
class BOTAN_DLL Skein_512 final : public HashFunction
   {
   public:
      /**
      * @param output_bits the output size of Skein in bits
      * @param personalization is a string that will parameterize the
      * hash output
      */
      Skein_512(size_t output_bits = 512,
                const std::string& personalization = "");

      size_t hash_block_size() const override { return 64; }
      size_t output_length() const override { return m_output_bits / 8; }

      HashFunction* clone() const override;
      std::string name() const override;
      void clear() override;
   private:
      enum type_code {
         SKEIN_KEY = 0,
         SKEIN_CONFIG = 4,
         SKEIN_PERSONALIZATION = 8,
         SKEIN_PUBLIC_KEY = 12,
         SKEIN_KEY_IDENTIFIER = 16,
         SKEIN_NONCE = 20,
         SKEIN_MSG = 48,
         SKEIN_OUTPUT = 63
      };

      void add_data(const uint8_t input[], size_t length) override;
      void final_result(uint8_t out[]) override;

      void ubi_512(const uint8_t msg[], size_t msg_len);

      void initial_block();
      void reset_tweak(type_code type, bool is_final);

      std::string m_personalization;
      size_t m_output_bits;

      std::unique_ptr<Threefish_512> m_threefish;
      secure_vector<uint64_t> m_T;
      secure_vector<uint8_t> m_buffer;
      size_t m_buf_pos;
   };

}


namespace Botan {

/**
 * NIST SP 800-108 KDF in Counter Mode (5.1)
 */
class BOTAN_DLL SP800_108_Counter : public KDF
   {
   public:
      std::string name() const override { return "SP800-108-Counter(" + m_prf->name() + ")"; }

      KDF* clone() const override { return new SP800_108_Counter(m_prf->clone()); }

      /**
      * Derive a key using the SP800-108 KDF in Counter mode.
      *
      * The implementation hard codes the length of [L]_2
      * and [i]_2 (the value r) to 32 bits.
      *
      * @param key resulting keying material
      * @param key_len the desired output length in bytes
      * @param secret K_I
      * @param secret_len size of K_I in bytes
      * @param salt Context
      * @param salt_len size of Context in bytes
      * @param label Label
      * @param label_len size of Label in bytes
      *
      * @throws Invalid_Argument key_len > 2^32
      */
      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

      /**
      * @param mac MAC algorithm to use
      */
      SP800_108_Counter(MessageAuthenticationCode* mac) : m_prf(mac) {}
   private:
      std::unique_ptr<MessageAuthenticationCode> m_prf;
   };

/**
 * NIST SP 800-108 KDF in Feedback Mode (5.2)
 */
class BOTAN_DLL SP800_108_Feedback : public KDF
   {
   public:
      std::string name() const override { return "SP800-108-Feedback(" + m_prf->name() + ")"; }

      KDF* clone() const override { return new SP800_108_Feedback(m_prf->clone()); }

      /**
      * Derive a key using the SP800-108 KDF in Feedback mode.
      *
      * The implementation uses the optional counter i and hard
      * codes the length of [L]_2 and [i]_2 (the value r) to 32 bits.
      *
      * @param key resulting keying material
      * @param key_len the desired output length in bytes
      * @param secret K_I
      * @param secret_len size of K_I in bytes
      * @param salt IV || Context
      * @param salt_len size of Context plus IV in bytes
      * @param label Label
      * @param label_len size of Label in bytes
      *
      * @throws Invalid_Argument key_len > 2^32
      */
      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

      SP800_108_Feedback(MessageAuthenticationCode* mac) : m_prf(mac) {}
   private:
      std::unique_ptr<MessageAuthenticationCode> m_prf;
   };

/**
 * NIST SP 800-108 KDF in Double Pipeline Mode (5.3)
 */
class BOTAN_DLL SP800_108_Pipeline : public KDF
   {
   public:
      std::string name() const override { return "SP800-108-Pipeline(" + m_prf->name() + ")"; }

      KDF* clone() const override { return new SP800_108_Pipeline(m_prf->clone()); }

      /**
      * Derive a key using the SP800-108 KDF in Double Pipeline mode.
      *
      * The implementation uses the optional counter i and hard
      * codes the length of [L]_2 and [i]_2 (the value r) to 32 bits.
      *
      * @param key resulting keying material
      * @param key_len the desired output length in bytes
      * @param secret K_I
      * @param secret_len size of K_I in bytes
      * @param salt Context
      * @param salt_len size of Context in bytes
      * @param label Label
      * @param label_len size of Label in bytes
      *
      * @throws Invalid_Argument key_len > 2^32
      */
      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

      SP800_108_Pipeline(MessageAuthenticationCode* mac) : m_prf(mac) {}

   private:
      std::unique_ptr<MessageAuthenticationCode> m_prf;
   };

}


namespace Botan {

/**
 * NIST SP 800-56C KDF
 */
class BOTAN_DLL SP800_56C : public KDF
   {
   public:
      std::string name() const override { return "SP800-56C(" + m_prf->name() + ")"; }

      KDF* clone() const override { return new SP800_56C(m_prf->clone(), m_exp->clone()); }

      /**
      * Derive a key using the SP800-56C KDF.
      *
      * The implementation hard codes the context value for the
      * expansion step to the empty string.
      *
      * @param key derived keying material K_M
      * @param key_len the desired output length in bytes
      * @param secret shared secret Z
      * @param secret_len size of Z in bytes
      * @param salt salt s of the extraction step
      * @param salt_len size of s in bytes
      * @param label label for the expansion step
      * @param label_len size of label in bytes
      *
      * @throws Invalid_Argument key_len > 2^32
      */
      size_t kdf(uint8_t key[], size_t key_len,
                 const uint8_t secret[], size_t secret_len,
                 const uint8_t salt[], size_t salt_len,
                 const uint8_t label[], size_t label_len) const override;

      /**
      * @param mac MAC algorithm used for randomness extraction
      * @param exp KDF used for key expansion
      */
      SP800_56C(MessageAuthenticationCode* mac, KDF* exp) : m_prf(mac), m_exp(exp) {}
   private:
      std::unique_ptr<MessageAuthenticationCode> m_prf;
      std::unique_ptr<KDF> m_exp;
   };
}


namespace Botan {

/**
* SRP6a Client side
* @param username the username we are attempting login for
* @param password the password we are attempting to use
* @param group_id specifies the shared SRP group
* @param hash_id specifies a secure hash function
* @param salt is the salt value sent by the server
* @param B is the server's public value
* @param rng is a random number generator
*
* @return (A,K) the client public key and the shared secret key
*/
std::pair<BigInt,SymmetricKey>
BOTAN_DLL srp6_client_agree(const std::string& username,
                            const std::string& password,
                            const std::string& group_id,
                            const std::string& hash_id,
                            const std::vector<uint8_t>& salt,
                            const BigInt& B,
                            RandomNumberGenerator& rng);

/**
* Generate a new SRP-6 verifier
* @param identifier a username or other client identifier
* @param password the secret used to authenticate user
* @param salt a randomly chosen value, at least 128 bits long
* @param group_id specifies the shared SRP group
* @param hash_id specifies a secure hash function
*/
BigInt BOTAN_DLL generate_srp6_verifier(const std::string& identifier,
                                        const std::string& password,
                                        const std::vector<uint8_t>& salt,
                                        const std::string& group_id,
                                        const std::string& hash_id);

/**
* Return the group id for this SRP param set, or else thrown an
* exception
* @param N the group modulus
* @param g the group generator
* @return group identifier
*/
std::string BOTAN_DLL srp6_group_identifier(const BigInt& N, const BigInt& g);

/**
* Represents a SRP-6a server session
*/
class BOTAN_DLL SRP6_Server_Session
   {
   public:
      /**
      * Server side step 1
      * @param v the verification value saved from client registration
      * @param group_id the SRP group id
      * @param hash_id the SRP hash in use
      * @param rng a random number generator
      * @return SRP-6 B value
      */
      BigInt step1(const BigInt& v,
                   const std::string& group_id,
                   const std::string& hash_id,
                   RandomNumberGenerator& rng);

      /**
      * Server side step 2
      * @param A the client's value
      * @return shared symmetric key
      */
      SymmetricKey step2(const BigInt& A);

   private:
      std::string m_hash_id;
      BigInt m_B, m_b, m_v, m_S, m_p;
      size_t m_p_bytes = 0;
   };

}


namespace Botan {

class BOTAN_DLL Stream_Cipher_Mode : public Cipher_Mode
   {
   public:
      /**
      * @param cipher underyling stream cipher
      */
      explicit Stream_Cipher_Mode(StreamCipher* cipher) : m_cipher(cipher) {}

      size_t process(uint8_t buf[], size_t sz) override
         {
         m_cipher->cipher1(buf, sz);
         return sz;
         }

      void finish(secure_vector<uint8_t>& buf, size_t offset) override
         { return update(buf, offset); }

      size_t output_length(size_t input_length) const override { return input_length; }

      size_t update_granularity() const override { return 1; }

      size_t minimum_final_size() const override { return 0; }

      size_t default_nonce_length() const override { return 0; }

      bool valid_nonce_length(size_t nonce_len) const override
         { return m_cipher->valid_iv_length(nonce_len); }

      Key_Length_Specification key_spec() const override { return m_cipher->key_spec(); }

      std::string name() const override { return m_cipher->name(); }

      void clear() override
         {
         m_cipher->clear();
         reset();
         }

      void reset() override { /* no msg state */ return; }

   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override
         {
         m_cipher->set_iv(nonce, nonce_len);
         }

      void key_schedule(const uint8_t key[], size_t length) override
         {
         m_cipher->set_key(key, length);
         }

      std::unique_ptr<StreamCipher> m_cipher;
   };

}


namespace Botan {

/**
* Tiger
*/
class BOTAN_DLL Tiger final : public MDx_HashFunction
   {
   public:
      std::string name() const override;
      size_t output_length() const override { return m_hash_len; }

      HashFunction* clone() const override
         {
         return new Tiger(output_length(), m_passes);
         }

      void clear() override;

      /**
      * @param out_size specifies the output length; can be 16, 20, or 24
      * @param passes to make in the algorithm
      */
      Tiger(size_t out_size = 24, size_t passes = 3);
   private:
      void compress_n(const uint8_t[], size_t block) override;
      void copy_out(uint8_t[]) override;

      static void pass(uint64_t& A, uint64_t& B, uint64_t& C,
                       const secure_vector<uint64_t>& M,
                       uint8_t mul);

      static const uint64_t SBOX1[256];
      static const uint64_t SBOX2[256];
      static const uint64_t SBOX3[256];
      static const uint64_t SBOX4[256];

      secure_vector<uint64_t> m_X, m_digest;
      const size_t m_hash_len, m_passes;
   };

}


namespace Botan {

namespace TLS {

/**
* SSL/TLS Alert Message
*/
class BOTAN_DLL Alert
   {
   public:
      /**
      * Type codes for TLS alerts
      */
      enum Type {
         CLOSE_NOTIFY                    = 0,
         UNEXPECTED_MESSAGE              = 10,
         BAD_RECORD_MAC                  = 20,
         DECRYPTION_FAILED               = 21,
         RECORD_OVERFLOW                 = 22,
         DECOMPRESSION_FAILURE           = 30,
         HANDSHAKE_FAILURE               = 40,
         NO_CERTIFICATE                  = 41, // SSLv3 only
         BAD_CERTIFICATE                 = 42,
         UNSUPPORTED_CERTIFICATE         = 43,
         CERTIFICATE_REVOKED             = 44,
         CERTIFICATE_EXPIRED             = 45,
         CERTIFICATE_UNKNOWN             = 46,
         ILLEGAL_PARAMETER               = 47,
         UNKNOWN_CA                      = 48,
         ACCESS_DENIED                   = 49,
         DECODE_ERROR                    = 50,
         DECRYPT_ERROR                   = 51,
         EXPORT_RESTRICTION              = 60,
         PROTOCOL_VERSION                = 70,
         INSUFFICIENT_SECURITY           = 71,
         INTERNAL_ERROR                  = 80,
         INAPPROPRIATE_FALLBACK          = 86,
         USER_CANCELED                   = 90,
         NO_RENEGOTIATION                = 100,
         UNSUPPORTED_EXTENSION           = 110,
         CERTIFICATE_UNOBTAINABLE        = 111,
         UNRECOGNIZED_NAME               = 112,
         BAD_CERTIFICATE_STATUS_RESPONSE = 113,
         BAD_CERTIFICATE_HASH_VALUE      = 114,
         UNKNOWN_PSK_IDENTITY            = 115,

         NO_APPLICATION_PROTOCOL         = 120, // RFC 7301

         // pseudo alert values
         NULL_ALERT                      = 256
      };

      /**
      * @return true iff this alert is non-empty
      */
      bool is_valid() const { return (m_type_code != NULL_ALERT); }

      /**
      * @return if this alert is a fatal one or not
      */
      bool is_fatal() const { return m_fatal; }

      /**
      * @return type of alert
      */
      Type type() const { return m_type_code; }

      /**
      * @return type of alert
      */
      std::string type_string() const;

      /**
      * Serialize an alert
      */
      std::vector<uint8_t> serialize() const;

      /**
      * Deserialize an Alert message
      * @param buf the serialized alert
      */
      explicit Alert(const secure_vector<uint8_t>& buf);

      /**
      * Create a new Alert
      * @param type_code the type of alert
      * @param fatal specifies if this is a fatal alert
      */
      Alert(Type type_code, bool fatal = false) :
         m_fatal(fatal), m_type_code(type_code) {}

      Alert() : m_fatal(false), m_type_code(NULL_ALERT) {}
   private:
      bool m_fatal;
      Type m_type_code;
   };

}

}


namespace Botan {

namespace TLS {

/**
* TLS Protocol Version
*/
class BOTAN_DLL Protocol_Version
   {
   public:
      enum Version_Code {
         TLS_V10            = 0x0301,
         TLS_V11            = 0x0302,
         TLS_V12            = 0x0303,

         DTLS_V10           = 0xFEFF,
         DTLS_V12           = 0xFEFD
      };

      /**
      * @return latest known TLS version
      */
      static Protocol_Version latest_tls_version()
         {
         return Protocol_Version(TLS_V12);
         }

      /**
      * @return latest known DTLS version
      */
      static Protocol_Version latest_dtls_version()
         {
         return Protocol_Version(DTLS_V12);
         }

      Protocol_Version() : m_version(0) {}

      /**
      * @param named_version a specific named version of the protocol
      */
      Protocol_Version(Version_Code named_version) :
         m_version(static_cast<uint16_t>(named_version)) {}

      /**
      * @param major the major version
      * @param minor the minor version
      */
      Protocol_Version(uint8_t major, uint8_t minor) :
         m_version((static_cast<uint16_t>(major) << 8) | minor) {}

      /**
      * @return true if this is a valid protocol version
      */
      bool valid() const { return (m_version != 0); }

      /**
      * @return true if this is a protocol version we know about
      */
      bool known_version() const;

      /**
      * @return major version of the protocol version
      */
      uint8_t major_version() const { return get_byte(0, m_version); }

      /**
      * @return minor version of the protocol version
      */
      uint8_t minor_version() const { return get_byte(1, m_version); }

      /**
      * @return human-readable description of this version
      */
      std::string to_string() const;

      /**
      * @return true iff this is a DTLS version
      */
      bool is_datagram_protocol() const;

      /**
      * @return true if this version supports negotiable signature algorithms
      */
      bool supports_negotiable_signature_algorithms() const;

      /**
      * @return true if this version uses explicit IVs for block ciphers
      */
      bool supports_explicit_cbc_ivs() const;

      /**
      * @return true if this version uses a ciphersuite specific PRF
      */
      bool supports_ciphersuite_specific_prf() const;

      bool supports_aead_modes() const;

      /**
      * @return if this version is equal to other
      */
      bool operator==(const Protocol_Version& other) const
         {
         return (m_version == other.m_version);
         }

      /**
      * @return if this version is not equal to other
      */
      bool operator!=(const Protocol_Version& other) const
         {
         return (m_version != other.m_version);
         }

      /**
      * @return if this version is later than other
      */
      bool operator>(const Protocol_Version& other) const;

      /**
      * @return if this version is later than or equal to other
      */
      bool operator>=(const Protocol_Version& other) const
         {
         return (*this == other || *this > other);
         }

   private:
      uint16_t m_version;
   };

}

}


namespace Botan {

namespace TLS {

/**
* Ciphersuite Information
*/
class BOTAN_DLL Ciphersuite
   {
   public:
      /**
      * Convert an SSL/TLS ciphersuite to algorithm fields
      * @param suite the ciphersuite code number
      * @return ciphersuite object
      */
      static Ciphersuite by_id(uint16_t suite);

      /**
      * Returns true iff this suite is a known SCSV
      */
      static bool is_scsv(uint16_t suite);

      /**
      * Generate a static list of all known ciphersuites and return it.
      *
      * @return list of all known ciphersuites
      */
      static const std::vector<Ciphersuite>& all_known_ciphersuites();

      /**
      * Formats the ciphersuite back to an RFC-style ciphersuite string
      * @return RFC ciphersuite string identifier
      */
      std::string to_string() const { return m_iana_id; }

      /**
      * @return ciphersuite number
      */
      uint16_t ciphersuite_code() const { return m_ciphersuite_code; }

      /**
      * @return true if this is a PSK ciphersuite
      */
      bool psk_ciphersuite() const;

      /**
      * @return true if this is an ECC ciphersuite
      */
      bool ecc_ciphersuite() const;

      /**
       * @return true if this suite uses a CBC cipher
       */
      bool cbc_ciphersuite() const;

      /**
      * @return key exchange algorithm used by this ciphersuite
      */
      std::string kex_algo() const { return m_kex_algo; }

      /**
      * @return signature algorithm used by this ciphersuite
      */
      std::string sig_algo() const { return m_sig_algo; }

      /**
      * @return symmetric cipher algorithm used by this ciphersuite
      */
      std::string cipher_algo() const { return m_cipher_algo; }

      /**
      * @return message authentication algorithm used by this ciphersuite
      */
      std::string mac_algo() const { return m_mac_algo; }

      std::string prf_algo() const
         {
         if(m_prf_algo && *m_prf_algo)
            return m_prf_algo;
         return m_mac_algo;
         }

      /**
      * @return cipher key length used by this ciphersuite
      */
      size_t cipher_keylen() const { return m_cipher_keylen; }

      size_t nonce_bytes_from_record() const { return m_nonce_bytes_from_record; }

      size_t nonce_bytes_from_handshake() const { return m_nonce_bytes_from_handshake; }

      size_t mac_keylen() const { return m_mac_keylen; }

      /**
      * @return true if this is a valid/known ciphersuite
      */
      bool valid() const { return m_usable; }

      bool operator<(const Ciphersuite& o) const { return ciphersuite_code() < o.ciphersuite_code(); }
      bool operator<(const uint16_t c) const { return ciphersuite_code() < c; }

      Ciphersuite() {}

   private:

      bool is_usable() const;

      Ciphersuite(uint16_t ciphersuite_code,
                  const char* iana_id,
                  const char* sig_algo,
                  const char* kex_algo,
                  const char* cipher_algo,
                  size_t cipher_keylen,
                  size_t nonce_bytes_from_handshake,
                  size_t nonce_bytes_from_record,
                  const char* mac_algo,
                  size_t mac_keylen,
                  const char* prf_algo) :
         m_ciphersuite_code(ciphersuite_code),
         m_iana_id(iana_id),
         m_sig_algo(sig_algo),
         m_kex_algo(kex_algo),
         m_prf_algo(prf_algo),
         m_cipher_algo(cipher_algo),
         m_mac_algo(mac_algo),
         m_cipher_keylen(cipher_keylen),
         m_nonce_bytes_from_handshake(nonce_bytes_from_handshake),
         m_nonce_bytes_from_record(nonce_bytes_from_record),
         m_mac_keylen(mac_keylen)
         {
         m_usable = is_usable();
         }

      uint16_t m_ciphersuite_code = 0;

      /*
      All of these const char* strings are references to compile time
      constants in tls_suite_info.cpp
      */
      const char* m_iana_id = nullptr;

      const char* m_sig_algo = nullptr;
      const char* m_kex_algo = nullptr;
      const char* m_prf_algo = nullptr;

      const char* m_cipher_algo = nullptr;
      const char* m_mac_algo = nullptr;

      size_t m_cipher_keylen = 0;
      size_t m_nonce_bytes_from_handshake = 0;
      size_t m_nonce_bytes_from_record = 0;
      size_t m_mac_keylen = 0;

      bool m_usable = false;
   };

}

}


namespace Botan {

namespace TLS {

/**
* TLS Policy Base Class
* Inherit and overload as desired to suit local policy concerns
*/
class BOTAN_DLL Policy
   {
   public:

      /**
      * Returns a list of ciphers we are willing to negotiate, in
      * order of preference.
      */
      virtual std::vector<std::string> allowed_ciphers() const;

      /**
      * Returns a list of hash algorithms we are willing to use for
      * signatures, in order of preference.
      */
      virtual std::vector<std::string> allowed_signature_hashes() const;

      /**
      * Returns a list of MAC algorithms we are willing to use.
      */
      virtual std::vector<std::string> allowed_macs() const;

      /**
      * Returns a list of key exchange algorithms we are willing to
      * use, in order of preference. Allowed values: DH, empty string
      * (representing RSA using server certificate key)
      */
      virtual std::vector<std::string> allowed_key_exchange_methods() const;

      /**
      * Returns a list of signature algorithms we are willing to
      * use, in order of preference. Allowed values RSA and DSA.
      */
      virtual std::vector<std::string> allowed_signature_methods() const;

      /**
      * The minimum signature strength we will accept
      * Returning 80 allows RSA 1024 and SHA-1. Values larger than 80 disable SHA-1 support.
      * Returning 110 allows RSA 2048.
      * Return 128 to force ECC (P-256) or large (~3000 bit) RSA keys.
      * Default is 110
      */
      virtual size_t minimum_signature_strength() const;

      /**
      * Return if cert revocation info (CRL/OCSP) is required
      * If true, validation will fail unless a valid CRL or OCSP response
      * was examined.
      */
      virtual bool require_cert_revocation_info() const;

      bool allowed_signature_method(const std::string& sig_method) const;

      /**
      * Return list of ECC curves we are willing to use in order of preference
      */
      virtual std::vector<std::string> allowed_ecc_curves() const;

      bool allowed_ecc_curve(const std::string& curve) const;

      /**
      * Request that ECC curve points are sent compressed
      */
      virtual bool use_ecc_point_compression() const;

      /**
      * Returns a list of compression algorithms we are willing to use,
      * in order of preference. Allowed values any value of
      * Compression_Method.
      *
      * @note Compression is not currently supported
      */
      virtual std::vector<uint8_t> compression() const;

      /**
      * Choose an elliptic curve to use
      */
      virtual std::string choose_curve(const std::vector<std::string>& curve_names) const;

      /**
      * Allow renegotiation even if the counterparty doesn't
      * support the secure renegotiation extension.
      *
      * @warning Changing this to true exposes you to injected
      * plaintext attacks. Read RFC 5746 for background.
      */
      virtual bool allow_insecure_renegotiation() const;

      /**
      * The protocol dictates that the first 32 bits of the random
      * field are the current time in seconds. However this allows
      * client fingerprinting attacks. Set to false to disable, in
      * which case random bytes will be used instead.
      */
      virtual bool include_time_in_hello_random() const;

      /**
      * Allow servers to initiate a new handshake
      */
      virtual bool allow_server_initiated_renegotiation() const;
      
      /**
      * Allow TLS v1.0
      */
      virtual bool allow_tls10() const;
      
      /**
      * Allow TLS v1.1
      */
      virtual bool allow_tls11() const;
      
      /**
      * Allow TLS v1.2
      */
      virtual bool allow_tls12() const;
      
      /**
      * Allow DTLS v1.0
      */
      virtual bool allow_dtls10() const;
      
      /**
      * Allow DTLS v1.2
      */
      virtual bool allow_dtls12() const;

      virtual std::string dh_group() const;

      /**
      * Return the minimum DH group size we're willing to use
      * Default is currently 1024 (insecure), should be 2048
      */
      virtual size_t minimum_dh_group_size() const;
      
      /**
      * For ECDSA authenticated ciphersuites, the smallest key size the
      * client will accept.
      * This policy is currently only enforced on the server by the client.
      */
      virtual size_t minimum_ecdsa_group_size() const;
      
      /**
      * Return the minimum ECDH group size we're willing to use
      * for key exchange
      *
      * Default 255, allowing x25519 and larger
      * x25519 is the smallest curve we will negotiate
      * P-521 is the largest
      */
      virtual size_t minimum_ecdh_group_size() const;

      /**
      * Return the minimum bit size we're willing to accept for RSA
      * key exchange or server signatures.
      *
      * It does not place any requirements on the size of any RSA signature(s)
      * which were used to check the server certificate. This is only
      * concerned with the server's public key.
      *
      * Default is 2048 which is smallest RSA key size still secure
      * for medium term security.
      */
      virtual size_t minimum_rsa_bits() const;

      /**
      * Minimum DSA group size, default 2048 bits
      */
      virtual size_t minimum_dsa_group_size() const;

      /**
      * Throw an exception if you don't like the peer's key.
      * Default impl checks the key size against minimum_rsa_bits, minimum_ecdsa_group_size,
      * or minimum_ecdh_group_size depending on the key's type.
      * Override if you'd like to perform some other kind of test on
      * (or logging of) the peer's keys.
      */
      virtual void check_peer_key_acceptable(const Public_Key& public_key) const;
      
      /**
      * If this function returns false, unknown SRP/PSK identifiers
      * will be rejected with an unknown_psk_identifier alert as soon
      * as the non-existence is identified. Otherwise, a false
      * identifier value will be used and the protocol allowed to
      * proceed, causing the handshake to eventually fail without
      * revealing that the username does not exist on this system.
      */
      virtual bool hide_unknown_users() const;

      /**
      * Return the allowed lifetime of a session ticket. If 0, session
      * tickets do not expire until the session ticket key rolls over.
      * Expired session tickets cannot be used to resume a session.
      */
      virtual uint32_t session_ticket_lifetime() const;

      /**
      * If this returns a non-empty vector, and DTLS is negotiated,
      * then we will also attempt to negotiate the SRTP extension from
      * RFC 5764 using the returned values as the profile ids.
      */
      virtual std::vector<uint16_t> srtp_profiles() const;

      /**
      * @return true if and only if we are willing to accept this version
      * Default accepts TLS v1.0 and later or DTLS v1.2 or later.
      */
      virtual bool acceptable_protocol_version(Protocol_Version version) const;

      /**
      * Returns the more recent protocol version we are willing to
      * use, for either TLS or DTLS depending on datagram param.
      * Shouldn't ever need to override this unless you want to allow
      * a user to disable use of TLS v1.2 (which is *not recommended*)
      */
      virtual Protocol_Version latest_supported_version(bool datagram) const;

      /**
      * When offering this version, should we send a fallback SCSV?
      * Default returns true iff version is not the latest version the
      * policy allows, exists to allow override in case of interop problems.
      */
      virtual bool send_fallback_scsv(Protocol_Version version) const;

      /**
      * Allows policy to reject any ciphersuites which are undesirable
      * for whatever reason without having to reimplement ciphersuite_list
      */
      virtual bool acceptable_ciphersuite(const Ciphersuite& suite) const;

      /**
      * @return true if servers should choose the ciphersuite matching
      *         their highest preference, rather than the clients.
      *         Has no effect on client side.
      */
      virtual bool server_uses_own_ciphersuite_preferences() const;

      /**
      * Indicates whether the encrypt-then-MAC extension should be negotiated
      * (RFC 7366)
      */
      virtual bool negotiate_encrypt_then_mac() const;

      /**
      * Return allowed ciphersuites, in order of preference
      */
      virtual std::vector<uint16_t> ciphersuite_list(Protocol_Version version,
                                                   bool have_srp) const;

      /**
      * @return the default MTU for DTLS
      */
      virtual size_t dtls_default_mtu() const;

      /**
      * @return the initial timeout for DTLS
      */
      virtual size_t dtls_initial_timeout() const;

      /**
      * @return the maximum timeout for DTLS
      */
      virtual size_t dtls_maximum_timeout() const;

      /**
      * Convert this policy to a printable format.
      * @param o stream to be printed to
      */
      virtual void print(std::ostream& o) const;

      /**
      * Convert this policy to a printable format.
      * Same as calling `print` on a ostringstream and reading o.str()
      */
      std::string to_string() const;

      virtual ~Policy() {}
   };

/**
* NSA Suite B 128-bit security level (RFC 6460)
*/
class BOTAN_DLL NSA_Suite_B_128 : public Policy
   {
   public:
      std::vector<std::string> allowed_ciphers() const override
         { return std::vector<std::string>({"AES-128/GCM"}); }

      std::vector<std::string> allowed_signature_hashes() const override
         { return std::vector<std::string>({"SHA-256"}); }

      std::vector<std::string> allowed_macs() const override
         { return std::vector<std::string>({"AEAD"}); }

      std::vector<std::string> allowed_key_exchange_methods() const override
         { return std::vector<std::string>({"ECDH"}); }

      std::vector<std::string> allowed_signature_methods() const override
         { return std::vector<std::string>({"ECDSA"}); }

      std::vector<std::string> allowed_ecc_curves() const override
         { return std::vector<std::string>({"secp256r1"}); }

      size_t minimum_signature_strength() const override { return 128; }

      bool allow_tls10()  const override { return false; }
      bool allow_tls11()  const override { return false; }
      bool allow_tls12()  const override { return true;  }
      bool allow_dtls10() const override { return false; }
      bool allow_dtls12() const override { return false; }
   };

/**
* BSI TR-02102-2 Policy
*/
class BOTAN_DLL BSI_TR_02102_2 : public Policy
   {
   public:
      std::vector<std::string> allowed_ciphers() const override
         {
         return std::vector<std::string>({"AES-256/GCM", "AES-128/GCM", "AES-256", "AES-128" });
         }

      std::vector<std::string> allowed_signature_hashes() const override
         {
         return std::vector<std::string>({"SHA-384", "SHA-256"});
         }

      std::vector<std::string> allowed_macs() const override
         {
         return std::vector<std::string>({"AEAD", "SHA-384", "SHA-256"});
         }

      std::vector<std::string> allowed_key_exchange_methods() const override
         {
         return std::vector<std::string>({"ECDH", "DH", "PSK", "ECDHE_PSK", "DHE_PSK"});
         }

      std::vector<std::string> allowed_signature_methods() const override
         {
         return std::vector<std::string>({"ECDSA", "RSA", "DSA"});
         }

      std::vector<std::string> allowed_ecc_curves() const override
         {
         return std::vector<std::string>({"brainpool512r1", "brainpool384r1", "brainpool256r1", "secp384r1", "secp256r1"});
         }

      bool allow_insecure_renegotiation() const override { return false; }
      bool allow_server_initiated_renegotiation() const override { return true; }
      bool server_uses_own_ciphersuite_preferences() const override { return true; }
      bool negotiate_encrypt_then_mac() const override { return true; }

      size_t minimum_rsa_bits() const override { return 2000; }
      size_t minimum_dh_group_size() const override { return 2000; }
      size_t minimum_dsa_group_size() const override { return 2000; }

      size_t minimum_ecdh_group_size() const override { return 250; }
      size_t minimum_ecdsa_group_size() const override { return 250; }

      bool allow_tls10()  const override { return false; }
      bool allow_tls11()  const override { return false; }
      bool allow_tls12()  const override { return true;  }
      bool allow_dtls10() const override { return false; }
      bool allow_dtls12() const override { return false; }
   };

/**
* Policy for DTLS. We require DTLS v1.2 and an AEAD mode.
*/
class BOTAN_DLL Datagram_Policy : public Policy
   {
   public:
      std::vector<std::string> allowed_macs() const override
         { return std::vector<std::string>({"AEAD"}); }
            
      bool allow_tls10()  const override { return false; }
      bool allow_tls11()  const override { return false; }
      bool allow_tls12()  const override { return false; }
      bool allow_dtls10() const override { return false; }
      bool allow_dtls12() const override { return true;  }
   };

/*
* This policy requires a secure version of TLS and disables all insecure
* algorithms. It is compatible with other botan TLSes (including those using the
* default policy) and with many other recent implementations. It is a great idea
* to use if you control both sides of the protocol and don't have to worry
* about ancient and/or bizarre TLS implementations.
*/
class BOTAN_DLL Strict_Policy : public Policy
   {
   public:
      std::vector<std::string> allowed_ciphers() const override;

      std::vector<std::string> allowed_signature_hashes() const override;

      std::vector<std::string> allowed_macs() const override;

      std::vector<std::string> allowed_key_exchange_methods() const override;

      bool allow_tls10()  const override;
      bool allow_tls11()  const override;
      bool allow_tls12()  const override;
      bool allow_dtls10() const override;
      bool allow_dtls12() const override;
   };

class BOTAN_DLL Text_Policy : public Policy
   {
   public:

      std::vector<std::string> allowed_ciphers() const override
         { return get_list("ciphers", Policy::allowed_ciphers()); }

      std::vector<std::string> allowed_signature_hashes() const override
         { return get_list("signature_hashes", Policy::allowed_signature_hashes()); }

      std::vector<std::string> allowed_macs() const override
         { return get_list("macs", Policy::allowed_macs()); }

      std::vector<std::string> allowed_key_exchange_methods() const override
         { return get_list("key_exchange_methods", Policy::allowed_key_exchange_methods()); }

      std::vector<std::string> allowed_signature_methods() const override
         { return get_list("signature_methods", Policy::allowed_signature_methods()); }

      std::vector<std::string> allowed_ecc_curves() const override
         { return get_list("ecc_curves", Policy::allowed_ecc_curves()); }
      
      bool use_ecc_point_compression() const override
         { return get_bool("use_ecc_point_compression", Policy::use_ecc_point_compression()); }

      bool allow_tls10() const override
         { return get_bool("allow_tls10", Policy::allow_tls10()); }
      
      bool allow_tls11() const override
         { return get_bool("allow_tls11", Policy::allow_tls11()); }
      
      bool allow_tls12() const override
         { return get_bool("allow_tls12", Policy::allow_tls12()); }
      
      bool allow_dtls10() const override
         { return get_bool("allow_dtls10", Policy::allow_dtls10()); }
      
      bool allow_dtls12() const override
         { return get_bool("allow_dtls12", Policy::allow_dtls12()); }

      bool allow_insecure_renegotiation() const override
         { return get_bool("allow_insecure_renegotiation", Policy::allow_insecure_renegotiation()); }

      bool include_time_in_hello_random() const override
         { return get_bool("include_time_in_hello_random", Policy::include_time_in_hello_random()); }

      bool allow_server_initiated_renegotiation() const override
         { return get_bool("allow_server_initiated_renegotiation", Policy::allow_server_initiated_renegotiation()); }

      bool server_uses_own_ciphersuite_preferences() const override
         { return get_bool("server_uses_own_ciphersuite_preferences", Policy::server_uses_own_ciphersuite_preferences()); }

      bool negotiate_encrypt_then_mac() const override
         { return get_bool("negotiate_encrypt_then_mac", Policy::negotiate_encrypt_then_mac()); }

      std::string dh_group() const override
         { return get_str("dh_group", Policy::dh_group()); }

      size_t minimum_ecdh_group_size() const override
         { return get_len("minimum_ecdh_group_size", Policy::minimum_ecdh_group_size()); }

      size_t minimum_ecdsa_group_size() const override
         { return get_len("minimum_ecdsa_group_size", Policy::minimum_ecdsa_group_size()); }

      size_t minimum_dh_group_size() const override
         { return get_len("minimum_dh_group_size", Policy::minimum_dh_group_size()); }

      size_t minimum_rsa_bits() const override
         { return get_len("minimum_rsa_bits", Policy::minimum_rsa_bits()); }

      size_t minimum_signature_strength() const override
         { return get_len("minimum_signature_strength", Policy::minimum_signature_strength()); }

      bool hide_unknown_users() const override
         { return get_bool("hide_unknown_users", Policy::hide_unknown_users()); }

      uint32_t session_ticket_lifetime() const override
         { return static_cast<uint32_t>(get_len("session_ticket_lifetime", Policy::session_ticket_lifetime())); }

      bool send_fallback_scsv(Protocol_Version version) const override
         { return get_bool("send_fallback_scsv", false) ? Policy::send_fallback_scsv(version) : false; }

      std::vector<uint16_t> srtp_profiles() const override
         {
         std::vector<uint16_t> r;
         for(auto&& p : get_list("srtp_profiles", std::vector<std::string>()))
            {
            r.push_back(to_u32bit(p));
            }
         return r;
         }

      void set(const std::string& k, const std::string& v) { m_kv[k] = v; }

      explicit Text_Policy(const std::string& s)
         {
         std::istringstream iss(s);
         m_kv = read_cfg(iss);
         }

      explicit Text_Policy(std::istream& in) : m_kv(read_cfg(in))
         {}

   private:

      std::vector<std::string> get_list(const std::string& key,
                                        const std::vector<std::string>& def) const
         {
         const std::string v = get_str(key);

         if(v.empty())
            return def;

         return split_on(v, ' ');
         }

      size_t get_len(const std::string& key, size_t def) const
         {
         const std::string v = get_str(key);

         if(v.empty())
            return def;

         return to_u32bit(v);
         }

      bool get_bool(const std::string& key, bool def) const
         {
         const std::string v = get_str(key);

         if(v.empty())
            return def;

         if(v == "true" || v == "True")
            return true;
         else if(v == "false" || v == "False")
            return false;
         else
            throw Exception("Invalid boolean '" + v + "'");
         }

      std::string get_str(const std::string& key, const std::string& def = "") const
         {
         auto i = m_kv.find(key);
         if(i == m_kv.end())
            return def;

         return i->second;
         }

      std::map<std::string, std::string> m_kv;
   };

}

}

namespace Botan {

namespace TLS {

/**
* Protocol Constants for SSL/TLS
*/
enum Size_Limits {
   TLS_HEADER_SIZE    = 5,
   DTLS_HEADER_SIZE   = TLS_HEADER_SIZE + 8,

   MAX_PLAINTEXT_SIZE = 16*1024,
   MAX_COMPRESSED_SIZE = MAX_PLAINTEXT_SIZE + 1024,
   MAX_CIPHERTEXT_SIZE = MAX_COMPRESSED_SIZE + 1024,
};

enum Connection_Side { CLIENT = 1, SERVER = 2 };

enum Record_Type {
   CHANGE_CIPHER_SPEC = 20,
   ALERT              = 21,
   HANDSHAKE          = 22,
   APPLICATION_DATA   = 23,

   NO_RECORD          = 256
};

enum Handshake_Type {
   HELLO_REQUEST        = 0,
   CLIENT_HELLO         = 1,
   SERVER_HELLO         = 2,
   HELLO_VERIFY_REQUEST = 3,
   NEW_SESSION_TICKET   = 4, // RFC 5077
   CERTIFICATE          = 11,
   SERVER_KEX           = 12,
   CERTIFICATE_REQUEST  = 13,
   SERVER_HELLO_DONE    = 14,
   CERTIFICATE_VERIFY   = 15,
   CLIENT_KEX           = 16,
   FINISHED             = 20,

   CERTIFICATE_URL      = 21,
   CERTIFICATE_STATUS   = 22,

   HANDSHAKE_CCS        = 254, // Not a wire value
   HANDSHAKE_NONE       = 255  // Null value
};

const char* handshake_type_to_string(Handshake_Type t);

enum Compression_Method {
   NO_COMPRESSION       = 0x00,
   DEFLATE_COMPRESSION  = 0x01
};

}

}


namespace Botan {

namespace TLS {

/**
* Represents information known about a TLS server.
*/
class BOTAN_DLL Server_Information
   {
   public:
      /**
      * An empty server info - nothing known
      */
      Server_Information() : m_hostname(""), m_service(""), m_port(0) {}

      /**
      * @param hostname the host's DNS name, if known
      * @param port specifies the protocol port of the server (eg for
      *        TCP/UDP). Zero represents unknown.
      */
      Server_Information(const std::string& hostname,
                        uint16_t port = 0) :
         m_hostname(hostname), m_service(""), m_port(port) {}

      /**
      * @param hostname the host's DNS name, if known
      * @param service is a text string of the service type
      *        (eg "https", "tor", or "git")
      * @param port specifies the protocol port of the server (eg for
      *        TCP/UDP). Zero represents unknown.
      */
      Server_Information(const std::string& hostname,
                        const std::string& service,
                        uint16_t port = 0) :
         m_hostname(hostname), m_service(service), m_port(port) {}

      /**
      * @return the host's DNS name, if known
      */
      std::string hostname() const { return m_hostname; }

      /**
      * @return text string of the service type, e.g.,
      * "https", "tor", or "git"
      */
      std::string service() const { return m_service; }

      /**
      * @return the protocol port of the server, or zero if unknown
      */
      uint16_t port() const { return m_port; }

      /**
      * @return whether the hostname is known
      */
      bool empty() const { return m_hostname.empty(); }

   private:
      std::string m_hostname, m_service;
      uint16_t m_port;
   };

inline bool operator==(const Server_Information& a, const Server_Information& b)
   {
   return (a.hostname() == b.hostname()) &&
          (a.service() == b.service()) &&
          (a.port() == b.port());

   }

inline bool operator!=(const Server_Information& a, const Server_Information& b)
   {
   return !(a == b);
   }

inline bool operator<(const Server_Information& a, const Server_Information& b)
   {
   if(a.hostname() != b.hostname())
      return (a.hostname() < b.hostname());
   if(a.service() != b.service())
      return (a.service() < b.service());
   if(a.port() != b.port())
      return (a.port() < b.port());
   return false; // equal
   }

}

}


namespace Botan {

namespace TLS {

/**
* Class representing a TLS session state
*/
class BOTAN_DLL Session
   {
   public:

      /**
      * Uninitialized session
      */
      Session() :
         m_start_time(std::chrono::system_clock::time_point::min()),
         m_version(),
         m_ciphersuite(0),
         m_compression_method(0),
         m_connection_side(static_cast<Connection_Side>(0)),
         m_srtp_profile(0),
         m_extended_master_secret(false),
         m_encrypt_then_mac(false)
            {}

      /**
      * New session (sets session start time)
      */
      Session(const std::vector<uint8_t>& session_id,
              const secure_vector<uint8_t>& master_secret,
              Protocol_Version version,
              uint16_t ciphersuite,
              uint8_t compression_method,
              Connection_Side side,
              bool supports_extended_master_secret,
              bool supports_encrypt_then_mac,
              const std::vector<X509_Certificate>& peer_certs,
              const std::vector<uint8_t>& session_ticket,
              const Server_Information& server_info,
              const std::string& srp_identifier,
              uint16_t srtp_profile);

      /**
      * Load a session from DER representation (created by DER_encode)
      * @param ber DER representation buffer
      * @param ber_len size of buffer in bytes
      */
      Session(const uint8_t ber[], size_t ber_len);

      /**
      * Load a session from PEM representation (created by PEM_encode)
      * @param pem PEM representation
      */
      explicit Session(const std::string& pem);

      /**
      * Encode this session data for storage
      * @warning if the master secret is compromised so is the
      * session traffic
      */
      secure_vector<uint8_t> DER_encode() const;

      /**
      * Encrypt a session (useful for serialization or session tickets)
      */
      std::vector<uint8_t> encrypt(const SymmetricKey& key,
                                RandomNumberGenerator& rng) const;


      /**
      * Decrypt a session created by encrypt
      * @param ctext the ciphertext returned by encrypt
      * @param ctext_size the size of ctext in bytes
      * @param key the same key used by the encrypting side
      */
      static Session decrypt(const uint8_t ctext[],
                             size_t ctext_size,
                             const SymmetricKey& key);

      /**
      * Decrypt a session created by encrypt
      * @param ctext the ciphertext returned by encrypt
      * @param key the same key used by the encrypting side
      */
      static inline Session decrypt(const std::vector<uint8_t>& ctext,
                                    const SymmetricKey& key)
         {
         return Session::decrypt(ctext.data(), ctext.size(), key);
         }

      /**
      * Encode this session data for storage
      * @warning if the master secret is compromised so is the
      * session traffic
      */
      std::string PEM_encode() const;

      /**
      * Get the version of the saved session
      */
      Protocol_Version version() const { return m_version; }

      /**
      * Get the ciphersuite code of the saved session
      */
      uint16_t ciphersuite_code() const { return m_ciphersuite; }

      /**
      * Get the ciphersuite info of the saved session
      */
      Ciphersuite ciphersuite() const { return Ciphersuite::by_id(m_ciphersuite); }

      /**
      * Get the compression method used in the saved session
      */
      uint8_t compression_method() const { return m_compression_method; }

      /**
      * Get which side of the connection the resumed session we are/were
      * acting as.
      */
      Connection_Side side() const { return m_connection_side; }

      /**
      * Get the SRP identity (if sent by the client in the initial handshake)
      */
      const std::string& srp_identifier() const { return m_srp_identifier; }

      /**
      * Get the saved master secret
      */
      const secure_vector<uint8_t>& master_secret() const { return m_master_secret; }

      /**
      * Get the session identifier
      */
      const std::vector<uint8_t>& session_id() const { return m_identifier; }

      /**
      * Get the negotiated DTLS-SRTP algorithm (RFC 5764)
      */
      uint16_t dtls_srtp_profile() const { return m_srtp_profile; }

      bool supports_extended_master_secret() const { return m_extended_master_secret; }

      bool supports_encrypt_then_mac() const { return m_encrypt_then_mac; }

      /**
      * Return the certificate chain of the peer (possibly empty)
      */
      const std::vector<X509_Certificate>& peer_certs() const { return m_peer_certs; }

      /**
      * Get the wall clock time this session began
      */
      std::chrono::system_clock::time_point start_time() const { return m_start_time; }

      /**
      * Return how long this session has existed (in seconds)
      */
      std::chrono::seconds session_age() const;

      /**
      * Return the session ticket the server gave us
      */
      const std::vector<uint8_t>& session_ticket() const { return m_session_ticket; }

      /**
      * @return information about the TLS server
      */
      const Server_Information& server_info() const { return m_server_info; }

   private:
      enum { TLS_SESSION_PARAM_STRUCT_VERSION = 20160812};

      std::chrono::system_clock::time_point m_start_time;

      std::vector<uint8_t> m_identifier;
      std::vector<uint8_t> m_session_ticket; // only used by client side
      secure_vector<uint8_t> m_master_secret;

      Protocol_Version m_version;
      uint16_t m_ciphersuite;
      uint8_t m_compression_method;
      Connection_Side m_connection_side;
      uint16_t m_srtp_profile;
      bool m_extended_master_secret;
      bool m_encrypt_then_mac;

      std::vector<X509_Certificate> m_peer_certs;
      Server_Information m_server_info; // optional
      std::string m_srp_identifier; // optional
   };

}

}


namespace Botan {

namespace TLS {

/**
* Session_Manager is an interface to systems which can save
* session parameters for supporting session resumption.
*
* Saving sessions is done on a best-effort basis; an implementation is
* allowed to drop sessions due to space constraints.
*
* Implementations should strive to be thread safe
*/
class BOTAN_DLL Session_Manager
   {
   public:
      /**
      * Try to load a saved session (using session ID)
      * @param session_id the session identifier we are trying to resume
      * @param session will be set to the saved session data (if found),
               or not modified if not found
      * @return true if session was modified
      */
      virtual bool load_from_session_id(const std::vector<uint8_t>& session_id,
                                        Session& session) = 0;

      /**
      * Try to load a saved session (using info about server)
      * @param info the information about the server
      * @param session will be set to the saved session data (if found),
               or not modified if not found
      * @return true if session was modified
      */
      virtual bool load_from_server_info(const Server_Information& info,
                                         Session& session) = 0;

      /**
      * Remove this session id from the cache, if it exists
      */
      virtual void remove_entry(const std::vector<uint8_t>& session_id) = 0;

      /**
      * Remove all sessions from the cache, return number of sessions deleted
      */
      virtual size_t remove_all() = 0;

      /**
      * Save a session on a best effort basis; the manager may not in
      * fact be able to save the session for whatever reason; this is
      * not an error. Caller cannot assume that calling save followed
      * immediately by load_from_* will result in a successful lookup.
      *
      * @param session to save
      */
      virtual void save(const Session& session) = 0;

      /**
      * Return the allowed lifetime of a session; beyond this time,
      * sessions are not resumed. Returns 0 if unknown/no explicit
      * expiration policy.
      */
      virtual std::chrono::seconds session_lifetime() const = 0;

      virtual ~Session_Manager() {}
   };

/**
* An implementation of Session_Manager that does not save sessions at
* all, preventing session resumption.
*/
class BOTAN_DLL Session_Manager_Noop : public Session_Manager
   {
   public:
      bool load_from_session_id(const std::vector<uint8_t>&, Session&) override
         { return false; }

      bool load_from_server_info(const Server_Information&, Session&) override
         { return false; }

      void remove_entry(const std::vector<uint8_t>&) override {}

      size_t remove_all() override { return 0; }

      void save(const Session&) override {}

      std::chrono::seconds session_lifetime() const override
         { return std::chrono::seconds(0); }
   };

/**
* An implementation of Session_Manager that saves values in memory.
*/
class BOTAN_DLL Session_Manager_In_Memory : public Session_Manager
   {
   public:
      /**
      * @param rng a RNG used for generating session key and for
      *        session encryption
      * @param max_sessions a hint on the maximum number of sessions
      *        to keep in memory at any one time. (If zero, don't cap)
      * @param session_lifetime sessions are expired after this many
      *        seconds have elapsed from initial handshake.
      */
      Session_Manager_In_Memory(RandomNumberGenerator& rng,
                                size_t max_sessions = 1000,
                                std::chrono::seconds session_lifetime =
                                   std::chrono::seconds(7200));

      bool load_from_session_id(const std::vector<uint8_t>& session_id,
                                Session& session) override;

      bool load_from_server_info(const Server_Information& info,
                                 Session& session) override;

      void remove_entry(const std::vector<uint8_t>& session_id) override;

      size_t remove_all() override;

      void save(const Session& session_data) override;

      std::chrono::seconds session_lifetime() const override
         { return m_session_lifetime; }

   private:
      bool load_from_session_str(const std::string& session_str,
                                 Session& session);

      mutex_type m_mutex;

      size_t m_max_sessions;

      std::chrono::seconds m_session_lifetime;

      RandomNumberGenerator& m_rng;
      secure_vector<uint8_t> m_session_key;

      std::map<std::string, std::vector<uint8_t>> m_sessions; // hex(session_id) -> session
      std::map<Server_Information, std::string> m_info_sessions;
   };

}

}


namespace Botan {

class Certificate_Store;
class X509_Certificate;

namespace OCSP {

class Response;

}

namespace TLS {

class Handshake_Message;
class Policy;

/**
* Encapsulates the callbacks that a TLS channel will make which are due to
* channel specific operations.
*/
class BOTAN_DLL Callbacks
   {
   public:
       virtual ~Callbacks();

       /**
       * Mandatory callback: output function
       * The channel will call this with data which needs to be sent to the peer
       * (eg, over a socket or some other form of IPC). The array will be overwritten
       * when the function returns so a copy must be made if the data cannot be
       * sent immediately.
       *
       * @param data the vector of data to send
       *
       * @param size the number of bytes to send
       */
       virtual void tls_emit_data(const uint8_t data[], size_t size) = 0;

       /**
       * Mandatory callback: process application data
       * Called when application data record is received from the peer.
       * Again the array is overwritten immediately after the function returns.
       *
       * @param seq_no the underlying TLS/DTLS record sequence number
       *
       * @param data the vector containing the received record
       *
       * @param size the length of the received record, in bytes
       */
       virtual void tls_record_received(uint64_t seq_no, const uint8_t data[], size_t size) = 0;

       /**
       * Mandatory callback: alert received
       * Called when an alert is received from the peer
       * If fatal, the connection is closing. If not fatal, the connection may
       * still be closing (depending on the error and the peer).
       *
       * @param alert the source of the alert
       */
       virtual void tls_alert(Alert alert) = 0;

       /**
       * Mandatory callback: session established
       * Called when a session is established. Throw an exception to abort
       * the connection.
       *
       * @param session the session descriptor
       *
       * @return return false to prevent the session from being cached,
       * return true to cache the session in the configured session manager
       */
       virtual bool tls_session_established(const Session& session) = 0;

       /**
       * Optional callback: session activated
       * Called when a session is active and can be written to
       */
       virtual void tls_session_activated() {}

       /**
       * Optional callback with default impl: verify cert chain
       *
       * Default implementation performs a standard PKIX validation
       * and initiates network OCSP request for end-entity cert.
       * Override to provide different behavior.
       *
       * Check the certificate chain is valid up to a trusted root, and
       * optionally (if hostname != "") that the hostname given is
       * consistent with the leaf certificate.
       *
       * This function should throw an exception derived from
       * std::exception with an informative what() result if the
       * certificate chain cannot be verified.
       *
       * @param cert_chain specifies a certificate chain leading to a
       *        trusted root CA certificate.
       * @param ocsp_responses the server may have provided some
       * @param trusted_roots the list of trusted certificates
       * @param usage what this cert chain is being used for
       *        Usage_Type::TLS_SERVER_AUTH for server chains,
       *        Usage_Type::TLS_CLIENT_AUTH for client chains,
       *        Usage_Type::UNSPECIFIED for other uses
       * @param hostname when authenticating a server, this is the hostname
       *        the client requested (eg via SNI). When authenticating a client,
       *        this is the server name the client is authenticating *to*.
       *        Empty in other cases or if no hostname was used.
       * @param policy the TLS policy associated with the session being authenticated
       *        using the certificate chain
       */
       virtual void tls_verify_cert_chain(
          const std::vector<X509_Certificate>& cert_chain,
          const std::vector<std::shared_ptr<const OCSP::Response>>& ocsp_responses,
          const std::vector<Certificate_Store*>& trusted_roots,
          Usage_Type usage,
          const std::string& hostname,
          const TLS::Policy& policy);

       /**
       * Called by default `tls_verify_cert_chain` to get the timeout to use for OCSP
       * requests. Return 0 to disable online OCSP checks.
       */
       virtual std::chrono::milliseconds tls_verify_cert_chain_ocsp_timeout() const
          {
          return std::chrono::milliseconds(0);
          }

       /**
       * Optional callback: inspect handshake message
       * Throw an exception to abort the handshake.
       * Default simply ignores the message.
       *
       * @param message the handshake message
       */
       virtual void tls_inspect_handshake_msg(const Handshake_Message& message);

       /**
       * Optional callback for server: choose ALPN protocol
       * ALPN (RFC 7301) works by the client sending a list of application
       * protocols it is willing to negotiate. The server then selects which
       * protocol to use, which is not necessarily even on the list that
       * the client sent.
       *
       * @param client_protos the vector of protocols the client is willing to negotiate
       *
       * @return the protocol selected by the server, which need not be on the
       * list that the client sent; if this is the empty string, the server ignores the
       * client ALPN extension. Default return value is empty string.
       */
       virtual std::string tls_server_choose_app_protocol(const std::vector<std::string>& client_protos);

       /**
       * Optional callback: error logging. (not currently called)
       * @param err An error message related to this connection.
       */
       virtual void tls_log_error(const char* err)
          {
          BOTAN_UNUSED(err);
          }

       /**
       * Optional callback: debug logging. (not currently called)
       * @param what Some hopefully informative string
       */
       virtual void tls_log_debug(const char* what)
          {
          BOTAN_UNUSED(what);
          }

       /**
       * Optional callback: debug logging taking a buffer. (not currently called)
       * @param descr What this buffer is
       * @param val the bytes
       * @param val_len length of val
       */
       virtual void tls_log_debug_bin(const char* descr, const uint8_t val[], size_t val_len)
          {
          BOTAN_UNUSED(descr);
          BOTAN_UNUSED(val);
          BOTAN_UNUSED(val_len);
          }
   };

/**
* TLS::Callbacks using std::function for compatability with the old API signatures.
* This type is only provided for backward compatibility.
* New implementations should derive from TLS::Callbacks instead.
*/
class BOTAN_DLL Compat_Callbacks final : public Callbacks
   {
   public:
      typedef std::function<void (const uint8_t[], size_t)> output_fn;
      typedef std::function<void (const uint8_t[], size_t)> data_cb;
      typedef std::function<void (Alert, const uint8_t[], size_t)> alert_cb;
      typedef std::function<bool (const Session&)> handshake_cb;
      typedef std::function<void (const Handshake_Message&)> handshake_msg_cb;
      typedef std::function<std::string (std::vector<std::string>)> next_protocol_fn;

      /**
       * @param output_fn is called with data for the outbound socket
       *
       * @param app_data_cb is called when new application data is received
       *
       * @param alert_cb is called when a TLS alert is received
       *
       * @param hs_cb is called when a handshake is completed
       *
       * @param hs_msg_cb is called for each handshake message received
       *
       * @param next_proto is called with ALPN protocol data sent by the client
       */
       BOTAN_DEPRECATED("Use TLS::Callbacks (virtual interface).")
       Compat_Callbacks(output_fn output_fn, data_cb app_data_cb, alert_cb alert_cb,
                        handshake_cb hs_cb, handshake_msg_cb hs_msg_cb = nullptr,
                        next_protocol_fn next_proto = nullptr)
          : m_output_function(output_fn), m_app_data_cb(app_data_cb),
            m_alert_cb(std::bind(alert_cb, std::placeholders::_1, nullptr, 0)),
            m_hs_cb(hs_cb), m_hs_msg_cb(hs_msg_cb), m_next_proto(next_proto) {}

       BOTAN_DEPRECATED("Use TLS::Callbacks (virtual interface).")
       Compat_Callbacks(output_fn output_fn, data_cb app_data_cb,
                        std::function<void (Alert)> alert_cb,
                        handshake_cb hs_cb,
                        handshake_msg_cb hs_msg_cb = nullptr,
                        next_protocol_fn next_proto = nullptr)
          : m_output_function(output_fn), m_app_data_cb(app_data_cb),
            m_alert_cb(alert_cb),
            m_hs_cb(hs_cb), m_hs_msg_cb(hs_msg_cb), m_next_proto(next_proto) {}

       void tls_emit_data(const uint8_t data[], size_t size) override
          {
          BOTAN_ASSERT(m_output_function != nullptr,
                       "Invalid TLS output function callback.");
          m_output_function(data, size);
          }

       void tls_record_received(uint64_t /*seq_no*/, const uint8_t data[], size_t size) override
          {
          BOTAN_ASSERT(m_app_data_cb != nullptr,
                       "Invalid TLS app data callback.");
          m_app_data_cb(data, size);
          }

       void tls_alert(Alert alert) override
          {
          BOTAN_ASSERT(m_alert_cb != nullptr,
                       "Invalid TLS alert callback.");
          m_alert_cb(alert);
          }

       bool tls_session_established(const Session& session) override
          {
          BOTAN_ASSERT(m_hs_cb != nullptr,
                       "Invalid TLS handshake callback.");
          return m_hs_cb(session);
          }

       std::string tls_server_choose_app_protocol(const std::vector<std::string>& client_protos) override
          {
          if(m_next_proto != nullptr) { return m_next_proto(client_protos); }
          return "";
          }

       void tls_inspect_handshake_msg(const Handshake_Message& hmsg) override
          {
          // The handshake message callback is optional so we can
          // not assume it has been set.
          if(m_hs_msg_cb != nullptr) { m_hs_msg_cb(hmsg); }
          }

    private:
         const output_fn m_output_function;
         const data_cb m_app_data_cb;
         const std::function<void (Alert)> m_alert_cb;
         const handshake_cb m_hs_cb;
         const handshake_msg_cb m_hs_msg_cb;
         const next_protocol_fn m_next_proto;
   };

}

}


namespace Botan {

namespace TLS {

class Connection_Cipher_State;
class Connection_Sequence_Numbers;
class Handshake_State;
class Handshake_Message;
class Client_Hello;
class Server_Hello;

/**
* Generic interface for TLS endpoint
*/
class BOTAN_DLL Channel
   {
   public:
      typedef std::function<void (const uint8_t[], size_t)> output_fn;
      typedef std::function<void (const uint8_t[], size_t)> data_cb;
      typedef std::function<void (Alert, const uint8_t[], size_t)> alert_cb;
      typedef std::function<bool (const Session&)> handshake_cb;
      typedef std::function<void (const Handshake_Message&)> handshake_msg_cb;
      static size_t IO_BUF_DEFAULT_SIZE;

      /**
      * Set up a new TLS session
      *
      * @param callbacks contains a set of callback function references
      *        required by the TLS endpoint.
      *
      * @param session_manager manages session state
      *
      * @param rng a random number generator
      *
      * @param policy specifies other connection policy information
      *
      * @param is_datagram whether this is a DTLS session
      *
      * @param io_buf_sz This many bytes of memory will
      *        be preallocated for the read and write buffers. Smaller
      *        values just mean reallocations and copies are more likely.
      */
      Channel(Callbacks& callbacks,
              Session_Manager& session_manager,
              RandomNumberGenerator& rng,
              const Policy& policy,
              bool is_datagram,
              size_t io_buf_sz = IO_BUF_DEFAULT_SIZE);

      /**
       * DEPRECATED. This constructor is only provided for backward
       * compatibility and should not be used in new implementations.
       * (Not marked deprecated since it is only called internally, by
       * other deprecated constructors)
       */
      Channel(output_fn out,
              data_cb app_data_cb,
              alert_cb alert_cb,
              handshake_cb hs_cb,
              handshake_msg_cb hs_msg_cb,
              Session_Manager& session_manager,
              RandomNumberGenerator& rng,
              const Policy& policy,
              bool is_datagram,
              size_t io_buf_sz = IO_BUF_DEFAULT_SIZE);

      Channel(const Channel&) = delete;

      Channel& operator=(const Channel&) = delete;

      virtual ~Channel();

      /**
      * Inject TLS traffic received from counterparty
      * @return a hint as the how many more bytes we need to process the
      *         current record (this may be 0 if on a record boundary)
      */
      size_t received_data(const uint8_t buf[], size_t buf_size);

      /**
      * Inject TLS traffic received from counterparty
      * @return a hint as the how many more bytes we need to process the
      *         current record (this may be 0 if on a record boundary)
      */
      size_t received_data(const std::vector<uint8_t>& buf);

      /**
      * Inject plaintext intended for counterparty
      * Throws an exception if is_active() is false
      */
      void send(const uint8_t buf[], size_t buf_size);

      /**
      * Inject plaintext intended for counterparty
      * Throws an exception if is_active() is false
      */
      void send(const std::string& val);

      /**
      * Inject plaintext intended for counterparty
      * Throws an exception if is_active() is false
      */
      template<typename Alloc>
         void send(const std::vector<unsigned char, Alloc>& val)
         {
         send(val.data(), val.size());
         }

      /**
      * Send a TLS alert message. If the alert is fatal, the internal
      * state (keys, etc) will be reset.
      * @param alert the Alert to send
      */
      void send_alert(const Alert& alert);

      /**
      * Send a warning alert
      */
      void send_warning_alert(Alert::Type type) { send_alert(Alert(type, false)); }

      /**
      * Send a fatal alert
      */
      void send_fatal_alert(Alert::Type type) { send_alert(Alert(type, true)); }

      /**
      * Send a close notification alert
      */
      void close() { send_warning_alert(Alert::CLOSE_NOTIFY); }

      /**
      * @return true iff the connection is active for sending application data
      */
      bool is_active() const;

      /**
      * @return true iff the connection has been definitely closed
      */
      bool is_closed() const;


      /**
      * @return certificate chain of the peer (may be empty)
      */
      std::vector<X509_Certificate> peer_cert_chain() const;

      /**
      * Key material export (RFC 5705)
      * @param label a disambiguating label string
      * @param context a per-association context value
      * @param length the length of the desired key in bytes
      * @return key of length bytes
      */
      SymmetricKey key_material_export(const std::string& label,
                                       const std::string& context,
                                       size_t length) const;

      /**
      * Attempt to renegotiate the session
      * @param force_full_renegotiation if true, require a full renegotiation,
      * otherwise allow session resumption
      */
      void renegotiate(bool force_full_renegotiation = false);

      /**
      * @return true iff the counterparty supports the secure
      * renegotiation extensions.
      */
      bool secure_renegotiation_supported() const;

      /**
      * Perform a handshake timeout check. This does nothing unless
      * this is a DTLS channel with a pending handshake state, in
      * which case we check for timeout and potentially retransmit
      * handshake packets.
      */
      bool timeout_check();

   protected:

      virtual void process_handshake_msg(const Handshake_State* active_state,
                                         Handshake_State& pending_state,
                                         Handshake_Type type,
                                         const std::vector<uint8_t>& contents) = 0;

      virtual void initiate_handshake(Handshake_State& state,
                                      bool force_full_renegotiation) = 0;

      virtual std::vector<X509_Certificate>
         get_peer_cert_chain(const Handshake_State& state) const = 0;

      virtual Handshake_State* new_handshake_state(class Handshake_IO* io) = 0;

      Handshake_State& create_handshake_state(Protocol_Version version);

      void inspect_handshake_message(const Handshake_Message& msg);

      void activate_session();

      void change_cipher_spec_reader(Connection_Side side);

      void change_cipher_spec_writer(Connection_Side side);

      /* secure renegotiation handling */

      void secure_renegotiation_check(const Client_Hello* client_hello);
      void secure_renegotiation_check(const Server_Hello* server_hello);

      std::vector<uint8_t> secure_renegotiation_data_for_client_hello() const;
      std::vector<uint8_t> secure_renegotiation_data_for_server_hello() const;

      RandomNumberGenerator& rng() { return m_rng; }

      Session_Manager& session_manager() { return m_session_manager; }

      const Policy& policy() const { return m_policy; }

      bool save_session(const Session& session) const { return callbacks().tls_session_established(session); }

      Callbacks& callbacks() const { return m_callbacks; }
   private:
      void init(size_t io_buf_sze);

      void send_record(uint8_t record_type, const std::vector<uint8_t>& record);

      void send_record_under_epoch(uint16_t epoch, uint8_t record_type,
                                   const std::vector<uint8_t>& record);

      void send_record_array(uint16_t epoch, uint8_t record_type,
                             const uint8_t input[], size_t length);

      void write_record(Connection_Cipher_State* cipher_state,
                        uint16_t epoch, uint8_t type, const uint8_t input[], size_t length);

      Connection_Sequence_Numbers& sequence_numbers() const;

      std::shared_ptr<Connection_Cipher_State> read_cipher_state_epoch(uint16_t epoch) const;

      std::shared_ptr<Connection_Cipher_State> write_cipher_state_epoch(uint16_t epoch) const;

      void reset_state();

      const Handshake_State* active_state() const { return m_active_state.get(); }

      const Handshake_State* pending_state() const { return m_pending_state.get(); }

      /* methods to handle incoming traffic through Channel::receive_data. */
      void process_handshake_ccs(const secure_vector<uint8_t>& record,
                                 uint64_t record_sequence,
                                 Record_Type record_type,
                                 Protocol_Version record_version);

      void process_application_data(uint64_t req_no, const secure_vector<uint8_t>& record);

      void process_alert(const secure_vector<uint8_t>& record);

      bool m_is_datagram;

      /* callbacks */
      std::unique_ptr<Compat_Callbacks> m_compat_callbacks;
      Callbacks& m_callbacks;

      /* external state */
      Session_Manager& m_session_manager;
      const Policy& m_policy;
      RandomNumberGenerator& m_rng;

      /* sequence number state */
      std::unique_ptr<Connection_Sequence_Numbers> m_sequence_numbers;

      /* pending and active connection states */
      std::unique_ptr<Handshake_State> m_active_state;
      std::unique_ptr<Handshake_State> m_pending_state;

      /* cipher states for each epoch */
      std::map<uint16_t, std::shared_ptr<Connection_Cipher_State>> m_write_cipher_states;
      std::map<uint16_t, std::shared_ptr<Connection_Cipher_State>> m_read_cipher_states;

      /* I/O buffers */
      secure_vector<uint8_t> m_writebuf;
      secure_vector<uint8_t> m_readbuf;
   };

}

}


namespace Botan {

namespace TLS {

/**
* SSL/TLS Client
*/
class BOTAN_DLL Client final : public Channel
   {
   public:

      /**
      * Set up a new TLS client session
      *
      * @param callbacks contains a set of callback function references
      *        required by the TLS client.
      *
      * @param session_manager manages session state
      *
      * @param creds manages application/user credentials
      *
      * @param policy specifies other connection policy information
      *
      * @param rng a random number generator
      *
      * @param server_info is identifying information about the TLS server
      *
      * @param offer_version specifies which version we will offer
      *        to the TLS server.
      *
      * @param next_protocols specifies protocols to advertise with ALPN
      *
      * @param reserved_io_buffer_size This many bytes of memory will
      *        be preallocated for the read and write buffers. Smaller
      *        values just mean reallocations and copies are more likely.
      */
     Client(Callbacks& callbacks,
            Session_Manager& session_manager,
            Credentials_Manager& creds,
            const Policy& policy,
            RandomNumberGenerator& rng,
            const Server_Information& server_info = Server_Information(),
            const Protocol_Version& offer_version = Protocol_Version::latest_tls_version(),
            const std::vector<std::string>& next_protocols = {},
            size_t reserved_io_buffer_size = TLS::Client::IO_BUF_DEFAULT_SIZE
         );

      /**
      * DEPRECATED. This constructor is only provided for backward
      * compatibility and should not be used in new code.
      * 
      * Set up a new TLS client session
      *
      * @param output_fn is called with data for the outbound socket
      *
      * @param app_data_cb is called when new application data is received
      *
      * @param alert_cb is called when a TLS alert is received
      *
      * @param hs_cb is called when a handshake is completed
      *
      * @param session_manager manages session state
      *
      * @param creds manages application/user credentials
      *
      * @param policy specifies other connection policy information
      *
      * @param rng a random number generator
      *
      * @param server_info is identifying information about the TLS server
      *
      * @param offer_version specifies which version we will offer
      *        to the TLS server.
      *
      * @param next_protocols specifies protocols to advertise with ALPN
      *
      * @param reserved_io_buffer_size This many bytes of memory will
      *        be preallocated for the read and write buffers. Smaller
      *        values just mean reallocations and copies are more likely.
      */
      BOTAN_DEPRECATED("Use TLS::Client(TLS::Callbacks ...)")
      Client(output_fn output_fn,
             data_cb app_data_cb,
             alert_cb alert_cb,
             handshake_cb hs_cb,
             Session_Manager& session_manager,
             Credentials_Manager& creds,
             const Policy& policy,
             RandomNumberGenerator& rng,
             const Server_Information& server_info = Server_Information(),
             const Protocol_Version& offer_version = Protocol_Version::latest_tls_version(),
             const std::vector<std::string>& next_protocols = {},
             size_t reserved_io_buffer_size = TLS::Client::IO_BUF_DEFAULT_SIZE
         );

      /**
       * DEPRECATED. This constructor is only provided for backward
       * compatibility and should not be used in new implementations.
       */
      BOTAN_DEPRECATED("Use TLS::Client(TLS::Callbacks ...)")
      Client(output_fn out,
             data_cb app_data_cb,
             alert_cb alert_cb,
             handshake_cb hs_cb,
             handshake_msg_cb hs_msg_cb,
             Session_Manager& session_manager,
             Credentials_Manager& creds,
             const Policy& policy,
             RandomNumberGenerator& rng,
             const Server_Information& server_info = Server_Information(),
             const Protocol_Version& offer_version = Protocol_Version::latest_tls_version(),
             const std::vector<std::string>& next_protocols = {}
         );

      /**
      * @return network protocol as advertised by the TLS server, if server sent the ALPN extension
      */
      const std::string& application_protocol() const { return m_application_protocol; }
   private:
      void init(const Protocol_Version& protocol_version,
                const std::vector<std::string>& next_protocols);

      std::vector<X509_Certificate>
         get_peer_cert_chain(const Handshake_State& state) const override;

      void initiate_handshake(Handshake_State& state,
                              bool force_full_renegotiation) override;

      void send_client_hello(Handshake_State& state,
                             bool force_full_renegotiation,
                             Protocol_Version version,
                             const std::string& srp_identifier = "",
                             const std::vector<std::string>& next_protocols = {});

      void process_handshake_msg(const Handshake_State* active_state,
                                 Handshake_State& pending_state,
                                 Handshake_Type type,
                                 const std::vector<uint8_t>& contents) override;

      Handshake_State* new_handshake_state(Handshake_IO* io) override;

      Credentials_Manager& m_creds;
      const Server_Information m_info;
      std::string m_application_protocol;
   };

}

}



namespace Botan {

namespace TLS {

class Server_Handshake_State;

/**
* TLS Server
*/
class BOTAN_DLL Server final : public Channel
   {
   public:
      typedef std::function<std::string (std::vector<std::string>)> next_protocol_fn;

      /**
      * Server initialization
      *
      * @param callbacks contains a set of callback function references
      *        required by the TLS client.
      *
      * @param session_manager manages session state
      *
      * @param creds manages application/user credentials
      *
      * @param policy specifies other connection policy information
      *
      * @param rng a random number generator
      *
      * @param is_datagram set to true if this server should expect DTLS
      *        connections. Otherwise TLS connections are expected.
      *
      * @param reserved_io_buffer_size This many bytes of memory will
      *        be preallocated for the read and write buffers. Smaller
      *        values just mean reallocations and copies are more likely.
      */
      Server(Callbacks& callbacks,
             Session_Manager& session_manager,
             Credentials_Manager& creds,
             const Policy& policy,
             RandomNumberGenerator& rng,
             bool is_datagram = false,
             size_t reserved_io_buffer_size = TLS::Server::IO_BUF_DEFAULT_SIZE
         );

      /**
       * DEPRECATED. This constructor is only provided for backward
       * compatibility and should not be used in new implementations.
       */
      BOTAN_DEPRECATED("Use TLS::Server(TLS::Callbacks ...)")
      Server(output_fn output,
             data_cb data_cb,
             alert_cb alert_cb,
             handshake_cb handshake_cb,
             Session_Manager& session_manager,
             Credentials_Manager& creds,
             const Policy& policy,
             RandomNumberGenerator& rng,
             next_protocol_fn next_proto = next_protocol_fn(),
             bool is_datagram = false,
             size_t reserved_io_buffer_size = TLS::Server::IO_BUF_DEFAULT_SIZE
         );

      /**
       * DEPRECATED. This constructor is only provided for backward
       * compatibility and should not be used in new implementations.
       */
      BOTAN_DEPRECATED("Use TLS::Server(TLS::Callbacks ...)")
      Server(output_fn output,
             data_cb data_cb,
             alert_cb alert_cb,
             handshake_cb handshake_cb,
             handshake_msg_cb hs_msg_cb,
             Session_Manager& session_manager,
             Credentials_Manager& creds,
             const Policy& policy,
             RandomNumberGenerator& rng,
             next_protocol_fn next_proto = next_protocol_fn(),
             bool is_datagram = false
         );

      /**
      * Return the protocol notification set by the client (using the
      * NPN extension) for this connection, if any. This value is not
      * tied to the session and a later renegotiation of the same
      * session can choose a new protocol.
      */
      std::string next_protocol() const { return m_next_protocol; }

   private:
      std::vector<X509_Certificate>
         get_peer_cert_chain(const Handshake_State& state) const override;

      void initiate_handshake(Handshake_State& state,
                              bool force_full_renegotiation) override;

      void process_handshake_msg(const Handshake_State* active_state,
                                 Handshake_State& pending_state,
                                 Handshake_Type type,
                                 const std::vector<uint8_t>& contents) override;

      void process_client_hello_msg(const Handshake_State* active_state,
                                    Server_Handshake_State& pending_state,
                                    const std::vector<uint8_t>& contents);

      void process_certificate_msg(Server_Handshake_State& pending_state,
                                   const std::vector<uint8_t>& contents);

      void process_client_key_exchange_msg(Server_Handshake_State& pending_state,
                                           const std::vector<uint8_t>& contents);

      void process_change_cipher_spec_msg(Server_Handshake_State& pending_state);

      void process_certificate_verify_msg(Server_Handshake_State& pending_state,
                                          Handshake_Type type,
                                          const std::vector<uint8_t>& contents);

      void process_finished_msg(Server_Handshake_State& pending_state,
                                Handshake_Type type,
                                const std::vector<uint8_t>& contents);

      void session_resume(Server_Handshake_State& pending_state,
                          bool have_session_ticket_key,
                          Session& session_info);

      void session_create(Server_Handshake_State& pending_state,
                          bool have_session_ticket_key);

      Handshake_State* new_handshake_state(Handshake_IO* io) override;

      Credentials_Manager& m_creds;
      std::string m_next_protocol;

      // Set by deprecated constructor, Server calls both this fn and Callbacks version
      next_protocol_fn m_choose_next_protocol;
   };

}

}


namespace Botan {

//template<typename T> using secure_deque = std::vector<T, secure_allocator<T>>;

namespace TLS {

/**
* Blocking TLS Client
* Can be used directly, or subclass to get handshake and alert notifications
*/
class BOTAN_DLL Blocking_Client
   {
   public:
      /*
      * These functions are expected to block until completing entirely, or
      * fail by throwing an exception.
      */
      typedef std::function<size_t (uint8_t[], size_t)> read_fn;
      typedef std::function<void (const uint8_t[], size_t)> write_fn;

      BOTAN_DEPRECATED("Use the regular TLS::Client interface")
      Blocking_Client(read_fn reader,
                      write_fn writer,
                      Session_Manager& session_manager,
                      Credentials_Manager& creds,
                      const Policy& policy,
                      RandomNumberGenerator& rng,
                      const Server_Information& server_info = Server_Information(),
                      const Protocol_Version& offer_version = Protocol_Version::latest_tls_version(),
                      const std::vector<std::string>& next_protos = {});

      /**
      * Completes full handshake then returns
      */
      void do_handshake();

      /**
      * Number of bytes pending read in the plaintext buffer (bytes
      * readable without blocking)
      */
      size_t pending() const { return m_plaintext.size(); }

      /**
      * Blocking read, will return at least 1 byte (eventually) or else 0 if the connection
      * is closed.
      */
      size_t read(uint8_t buf[], size_t buf_len);

      void write(const uint8_t buf[], size_t buf_len) { m_channel.send(buf, buf_len); }

      const TLS::Channel& underlying_channel() const { return m_channel; }
      TLS::Channel& underlying_channel() { return m_channel; }

      void close() { m_channel.close(); }

      bool is_closed() const { return m_channel.is_closed(); }

      std::vector<X509_Certificate> peer_cert_chain() const
         { return m_channel.peer_cert_chain(); }

      virtual ~Blocking_Client() {}

   protected:
      /**
      * Application can override to get the handshake complete notification
      */
      virtual bool handshake_complete(const Session&) { return true; }

      /**
      * Application can override to get notification of alerts
      */
      virtual void alert_notification(const Alert&) {}

   private:

      bool handshake_cb(const Session&);

      void data_cb(const uint8_t data[], size_t data_len);

      void alert_cb(const Alert& alert);

      read_fn m_read;
      std::unique_ptr<Compat_Callbacks> m_callbacks;
      TLS::Client m_channel;
      secure_vector<uint8_t> m_plaintext;
   };

}

}


namespace Botan {

namespace TLS {

/**
* Exception Base Class
*/
class BOTAN_DLL TLS_Exception : public Exception
   {
   public:
      Alert::Type type() const { return m_alert_type; }

      TLS_Exception(Alert::Type type,
                    const std::string& err_msg = "Unknown error") :
         Exception(err_msg), m_alert_type(type) {}

   private:
      Alert::Type m_alert_type;
   };

/**
* Unexpected_Message Exception
*/
struct BOTAN_DLL Unexpected_Message : public TLS_Exception
   {
   explicit Unexpected_Message(const std::string& err) :
      TLS_Exception(Alert::UNEXPECTED_MESSAGE, err) {}
   };

}

}


namespace Botan {

namespace TLS {

class TLS_Data_Reader;

enum Handshake_Extension_Type {
   TLSEXT_SERVER_NAME_INDICATION = 0,
   TLSEXT_CERT_STATUS_REQUEST    = 5,

   TLSEXT_CERTIFICATE_TYPES      = 9,
   TLSEXT_USABLE_ELLIPTIC_CURVES = 10,
   TLSEXT_EC_POINT_FORMATS       = 11,
   TLSEXT_SRP_IDENTIFIER         = 12,
   TLSEXT_SIGNATURE_ALGORITHMS   = 13,
   TLSEXT_USE_SRTP               = 14,
   TLSEXT_ALPN                   = 16,

   TLSEXT_ENCRYPT_THEN_MAC       = 22,
   TLSEXT_EXTENDED_MASTER_SECRET = 23,

   TLSEXT_SESSION_TICKET         = 35,

   TLSEXT_SAFE_RENEGOTIATION     = 65281,
};

/**
* Base class representing a TLS extension of some kind
*/
class Extension
   {
   public:
      /**
      * @return code number of the extension
      */
      virtual Handshake_Extension_Type type() const = 0;

      /**
      * @return serialized binary for the extension
      */
      virtual std::vector<uint8_t> serialize() const = 0;

      /**
      * @return if we should encode this extension or not
      */
      virtual bool empty() const = 0;

      virtual ~Extension() {}
   };

/**
* Server Name Indicator extension (RFC 3546)
*/
class Server_Name_Indicator final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SERVER_NAME_INDICATION; }

      Handshake_Extension_Type type() const override { return static_type(); }

      explicit Server_Name_Indicator(const std::string& host_name) :
         m_sni_host_name(host_name) {}

      Server_Name_Indicator(TLS_Data_Reader& reader,
                            uint16_t extension_size);

      std::string host_name() const { return m_sni_host_name; }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return m_sni_host_name.empty(); }
   private:
      std::string m_sni_host_name;
   };

#if defined(BOTAN_HAS_SRP6)
/**
* SRP identifier extension (RFC 5054)
*/
class SRP_Identifier final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SRP_IDENTIFIER; }

      Handshake_Extension_Type type() const override { return static_type(); }

      explicit SRP_Identifier(const std::string& identifier) :
         m_srp_identifier(identifier) {}

      SRP_Identifier(TLS_Data_Reader& reader,
                     uint16_t extension_size);

      std::string identifier() const { return m_srp_identifier; }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return m_srp_identifier.empty(); }
   private:
      std::string m_srp_identifier;
   };
#endif

/**
* Renegotiation Indication Extension (RFC 5746)
*/
class Renegotiation_Extension final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SAFE_RENEGOTIATION; }

      Handshake_Extension_Type type() const override { return static_type(); }

      Renegotiation_Extension() {}

      explicit Renegotiation_Extension(const std::vector<uint8_t>& bits) :
         m_reneg_data(bits) {}

      Renegotiation_Extension(TLS_Data_Reader& reader,
                             uint16_t extension_size);

      const std::vector<uint8_t>& renegotiation_info() const
         { return m_reneg_data; }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return false; } // always send this
   private:
      std::vector<uint8_t> m_reneg_data;
   };

/**
* ALPN (RFC 7301)
*/
class Application_Layer_Protocol_Notification final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type() { return TLSEXT_ALPN; }

      Handshake_Extension_Type type() const override { return static_type(); }

      const std::vector<std::string>& protocols() const { return m_protocols; }

      const std::string& single_protocol() const;

      /**
      * Single protocol, used by server
      */
      explicit Application_Layer_Protocol_Notification(const std::string& protocol) :
         m_protocols(1, protocol) {}

      /**
      * List of protocols, used by client
      */
      explicit Application_Layer_Protocol_Notification(const std::vector<std::string>& protocols) :
         m_protocols(protocols) {}

      Application_Layer_Protocol_Notification(TLS_Data_Reader& reader,
                                              uint16_t extension_size);

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return m_protocols.empty(); }
   private:
      std::vector<std::string> m_protocols;
   };

/**
* Session Ticket Extension (RFC 5077)
*/
class Session_Ticket final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SESSION_TICKET; }

      Handshake_Extension_Type type() const override { return static_type(); }

      /**
      * @return contents of the session ticket
      */
      const std::vector<uint8_t>& contents() const { return m_ticket; }

      /**
      * Create empty extension, used by both client and server
      */
      Session_Ticket() {}

      /**
      * Extension with ticket, used by client
      */
      explicit Session_Ticket(const std::vector<uint8_t>& session_ticket) :
         m_ticket(session_ticket) {}

      /**
      * Deserialize a session ticket
      */
      Session_Ticket(TLS_Data_Reader& reader, uint16_t extension_size);

      std::vector<uint8_t> serialize() const override { return m_ticket; }

      bool empty() const override { return false; }
   private:
      std::vector<uint8_t> m_ticket;
   };

/**
* Supported Elliptic Curves Extension (RFC 4492)
*/
class Supported_Elliptic_Curves final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_USABLE_ELLIPTIC_CURVES; }

      Handshake_Extension_Type type() const override { return static_type(); }

      static std::string curve_id_to_name(uint16_t id);
      static uint16_t name_to_curve_id(const std::string& name);

      const std::vector<std::string>& curves() const { return m_curves; }

      std::vector<uint8_t> serialize() const override;

      explicit Supported_Elliptic_Curves(const std::vector<std::string>& curves) :
         m_curves(curves) {}

      Supported_Elliptic_Curves(TLS_Data_Reader& reader,
                                uint16_t extension_size);

      bool empty() const override { return m_curves.empty(); }
   private:
      std::vector<std::string> m_curves;
   };

/**
* Supported Point Formats Extension (RFC 4492)
*/
class Supported_Point_Formats final : public Extension
   {
   public:
      enum ECPointFormat : uint8_t {
         UNCOMPRESSED = 0,
         ANSIX962_COMPRESSED_PRIME = 1,
         ANSIX962_COMPRESSED_CHAR2 = 2, // don't support these curves
      };

      static Handshake_Extension_Type static_type()
         { return TLSEXT_EC_POINT_FORMATS; }

      Handshake_Extension_Type type() const override { return static_type(); }

      std::vector<uint8_t> serialize() const override;

      explicit Supported_Point_Formats(bool prefer_compressed) :
         m_prefers_compressed(prefer_compressed) {}

      Supported_Point_Formats(TLS_Data_Reader& reader,
                              uint16_t extension_size);

      bool empty() const override { return false; }

      bool prefers_compressed() { return m_prefers_compressed; }

   private:
      bool m_prefers_compressed = false;
   };

/**
* Signature Algorithms Extension for TLS 1.2 (RFC 5246)
*/
class Signature_Algorithms final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_SIGNATURE_ALGORITHMS; }

      Handshake_Extension_Type type() const override { return static_type(); }

      static std::string hash_algo_name(uint8_t code);
      static uint8_t hash_algo_code(const std::string& name);

      static std::string sig_algo_name(uint8_t code);
      static uint8_t sig_algo_code(const std::string& name);

      // [(hash,sig),(hash,sig),...]
      const std::vector<std::pair<std::string, std::string>>&
      supported_signature_algorthms() const
         {
         return m_supported_algos;
         }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return false; }

      Signature_Algorithms(const std::vector<std::string>& hashes,
                           const std::vector<std::string>& sig_algos);

      explicit Signature_Algorithms(const std::vector<std::pair<std::string, std::string>>& algos) :
         m_supported_algos(algos) {}

      Signature_Algorithms(TLS_Data_Reader& reader,
                           uint16_t extension_size);
   private:
      std::vector<std::pair<std::string, std::string>> m_supported_algos;
   };

/**
* Used to indicate SRTP algorithms for DTLS (RFC 5764)
*/
class SRTP_Protection_Profiles final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_USE_SRTP; }

      Handshake_Extension_Type type() const override { return static_type(); }

      const std::vector<uint16_t>& profiles() const { return m_pp; }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return m_pp.empty(); }

      explicit SRTP_Protection_Profiles(const std::vector<uint16_t>& pp) : m_pp(pp) {}

      explicit SRTP_Protection_Profiles(uint16_t pp) : m_pp(1, pp) {}

      SRTP_Protection_Profiles(TLS_Data_Reader& reader, uint16_t extension_size);
   private:
      std::vector<uint16_t> m_pp;
   };

/**
* Extended Master Secret Extension (RFC 7627)
*/
class Extended_Master_Secret final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_EXTENDED_MASTER_SECRET; }

      Handshake_Extension_Type type() const override { return static_type(); }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return false; }

      Extended_Master_Secret() {}

      Extended_Master_Secret(TLS_Data_Reader& reader, uint16_t extension_size);
   };

/**
* Encrypt-then-MAC Extension (RFC 7366)
*/
class Encrypt_then_MAC final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_ENCRYPT_THEN_MAC; }

      Handshake_Extension_Type type() const override { return static_type(); }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return false; }

      Encrypt_then_MAC() {}

      Encrypt_then_MAC(TLS_Data_Reader& reader, uint16_t extension_size);
   };

/**
* Certificate Status Request (RFC 6066)
*/
class Certificate_Status_Request final : public Extension
   {
   public:
      static Handshake_Extension_Type static_type()
         { return TLSEXT_CERT_STATUS_REQUEST; }

      Handshake_Extension_Type type() const override { return static_type(); }

      std::vector<uint8_t> serialize() const override;

      bool empty() const override { return false; }

      // Server generated version: empty
      Certificate_Status_Request();

      // Client version, both lists can be empty
      Certificate_Status_Request(const std::vector<X509_DN>& ocsp_responder_ids,
                                 const std::vector<std::vector<uint8_t>>& ocsp_key_ids);

      Certificate_Status_Request(TLS_Data_Reader& reader, uint16_t extension_size);
   private:
      std::vector<X509_DN> m_ocsp_names;
      std::vector<std::vector<uint8_t>> m_ocsp_keys;
      std::vector<uint8_t> m_extension_bytes;
      bool m_server_side;
   };

/**
* Represents a block of extensions in a hello message
*/
class BOTAN_DLL Extensions
   {
   public:
      std::set<Handshake_Extension_Type> extension_types() const;

      template<typename T>
      T* get() const
         {
         Handshake_Extension_Type type = T::static_type();

         auto i = m_extensions.find(type);

         if(i != m_extensions.end())
            return dynamic_cast<T*>(i->second.get());
         return nullptr;
         }

      template<typename T>
      bool has() const
         {
         return get<T>() != nullptr;
         }

      void add(Extension* extn)
         {
         m_extensions[extn->type()].reset(extn);
         }

      std::vector<uint8_t> serialize() const;

      void deserialize(TLS_Data_Reader& reader);

      Extensions() {}

      explicit Extensions(TLS_Data_Reader& reader) { deserialize(reader); }

   private:
      Extensions(const Extensions&) {}
      Extensions& operator=(const Extensions&) { return (*this); }

      std::map<Handshake_Extension_Type, std::unique_ptr<Extension>> m_extensions;
   };

}

}


namespace Botan {

namespace TLS {

class Handshake_IO;
class Handshake_Hash;

/**
* TLS Handshake Message Base Class
*/
class BOTAN_DLL Handshake_Message
   {
   public:
      /**
      * @return string representation of this message type
      */
      std::string type_string() const;

      /**
      * @return the message type
      */
      virtual Handshake_Type type() const = 0;

      /**
      * @return DER representation of this message
      */
      virtual std::vector<uint8_t> serialize() const = 0;

      virtual ~Handshake_Message() {}
   };

}

}


namespace Botan {

class Credentials_Manager;

#if defined(BOTAN_HAS_SRP6)
class SRP6_Server_Session;
#endif

#if defined(BOTAN_HAS_CECPQ1)
class CECPQ1_key;
#endif

namespace TLS {

class Session;
class Handshake_IO;
class Handshake_State;

std::vector<uint8_t> make_hello_random(RandomNumberGenerator& rng,
                                    const Policy& policy);

/**
* DTLS Hello Verify Request
*/
class BOTAN_DLL Hello_Verify_Request final : public Handshake_Message
   {
   public:
      std::vector<uint8_t> serialize() const override;
      Handshake_Type type() const override { return HELLO_VERIFY_REQUEST; }

      std::vector<uint8_t> cookie() const { return m_cookie; }

      explicit Hello_Verify_Request(const std::vector<uint8_t>& buf);

      Hello_Verify_Request(const std::vector<uint8_t>& client_hello_bits,
                           const std::string& client_identity,
                           const SymmetricKey& secret_key);
   private:
      std::vector<uint8_t> m_cookie;
   };

/**
* Client Hello Message
*/
class BOTAN_DLL Client_Hello final : public Handshake_Message
   {
   public:
      class Settings
      {
          public:
              Settings(const Protocol_Version version,
                       const std::string& hostname = "",
                       const std::string& srp_identifier = "")
                  : m_new_session_version(version),
                    m_hostname(hostname),
                    m_srp_identifier(srp_identifier) {};

              const Protocol_Version protocol_version() const { return m_new_session_version; };
              const std::string& hostname() const { return m_hostname; };
              const std::string& srp_identifier() const { return m_srp_identifier; }

          private:
              const Protocol_Version m_new_session_version;
              const std::string m_hostname;
              const std::string m_srp_identifier;
      };

      Handshake_Type type() const override { return CLIENT_HELLO; }

      Protocol_Version version() const { return m_version; }

      const std::vector<uint8_t>& random() const { return m_random; }

      const std::vector<uint8_t>& session_id() const { return m_session_id; }

      std::vector<uint16_t> ciphersuites() const { return m_suites; }

      std::vector<uint8_t> compression_methods() const { return m_comp_methods; }

      bool offered_suite(uint16_t ciphersuite) const;

      bool sent_fallback_scsv() const;

      std::vector<std::pair<std::string, std::string>> supported_algos() const
         {
         if(Signature_Algorithms* sigs = m_extensions.get<Signature_Algorithms>())
            return sigs->supported_signature_algorthms();
         return std::vector<std::pair<std::string, std::string>>();
         }

      std::set<std::string> supported_sig_algos() const
         {
         std::set<std::string> sig;
         for(auto&& hash_and_sig : supported_algos())
            sig.insert(hash_and_sig.second);
         return sig;
         }

      std::vector<std::string> supported_ecc_curves() const
         {
         if(Supported_Elliptic_Curves* ecc = m_extensions.get<Supported_Elliptic_Curves>())
            return ecc->curves();
         return std::vector<std::string>();
         }

      bool prefers_compressed_ec_points() const
         {
         if(Supported_Point_Formats* ecc_formats = m_extensions.get<Supported_Point_Formats>())
            {
            return ecc_formats->prefers_compressed();
            }
         return false;
         }

      std::string sni_hostname() const
         {
         if(Server_Name_Indicator* sni = m_extensions.get<Server_Name_Indicator>())
            return sni->host_name();
         return "";
         }

#if defined(BOTAN_HAS_SRP6)
      std::string srp_identifier() const
         {
         if(SRP_Identifier* srp = m_extensions.get<SRP_Identifier>())
            return srp->identifier();
         return "";
         }
#endif

      bool secure_renegotiation() const
         {
         return m_extensions.has<Renegotiation_Extension>();
         }

      std::vector<uint8_t> renegotiation_info() const
         {
         if(Renegotiation_Extension* reneg = m_extensions.get<Renegotiation_Extension>())
            return reneg->renegotiation_info();
         return std::vector<uint8_t>();
         }

      bool supports_session_ticket() const
         {
         return m_extensions.has<Session_Ticket>();
         }

      std::vector<uint8_t> session_ticket() const
         {
         if(Session_Ticket* ticket = m_extensions.get<Session_Ticket>())
            return ticket->contents();
         return std::vector<uint8_t>();
         }

      bool supports_alpn() const
         {
         return m_extensions.has<Application_Layer_Protocol_Notification>();
         }

      bool supports_extended_master_secret() const
         {
         return m_extensions.has<Extended_Master_Secret>();
         }

      bool supports_cert_status_message() const
         {
         return m_extensions.has<Certificate_Status_Request>();
         }

      bool supports_encrypt_then_mac() const
         {
         return m_extensions.has<Encrypt_then_MAC>();
         }

      bool sent_signature_algorithms() const
         {
         return m_extensions.has<Signature_Algorithms>();
         }

      std::vector<std::string> next_protocols() const
         {
         if(auto alpn = m_extensions.get<Application_Layer_Protocol_Notification>())
            return alpn->protocols();
         return std::vector<std::string>();
         }

      std::vector<uint16_t> srtp_profiles() const
         {
         if(SRTP_Protection_Profiles* srtp = m_extensions.get<SRTP_Protection_Profiles>())
            return srtp->profiles();
         return std::vector<uint16_t>();
         }

      void update_hello_cookie(const Hello_Verify_Request& hello_verify);

      std::set<Handshake_Extension_Type> extension_types() const
         { return m_extensions.extension_types(); }

      Client_Hello(Handshake_IO& io,
                   Handshake_Hash& hash,
                   const Policy& policy,
                   RandomNumberGenerator& rng,
                   const std::vector<uint8_t>& reneg_info,
                   const Client_Hello::Settings& client_settings,
                   const std::vector<std::string>& next_protocols);

      Client_Hello(Handshake_IO& io,
                   Handshake_Hash& hash,
                   const Policy& policy,
                   RandomNumberGenerator& rng,
                   const std::vector<uint8_t>& reneg_info,
                   const Session& resumed_session,
                   const std::vector<std::string>& next_protocols);

      explicit Client_Hello(const std::vector<uint8_t>& buf);

   private:
      std::vector<uint8_t> serialize() const override;

      Protocol_Version m_version;
      std::vector<uint8_t> m_session_id;
      std::vector<uint8_t> m_random;
      std::vector<uint16_t> m_suites;
      std::vector<uint8_t> m_comp_methods;
      std::vector<uint8_t> m_hello_cookie; // DTLS only

      Extensions m_extensions;
   };

/**
* Server Hello Message
*/
class BOTAN_DLL Server_Hello final : public Handshake_Message
   {
   public:
      class Settings
      {
          public:
              Settings(const std::vector<uint8_t> new_session_id,
                       Protocol_Version new_session_version,
                       uint16_t ciphersuite,
                       uint8_t compression,
                       bool offer_session_ticket)
                  : m_new_session_id(new_session_id),
                    m_new_session_version(new_session_version),
                    m_ciphersuite(ciphersuite),
                    m_compression(compression),
                    m_offer_session_ticket(offer_session_ticket) {};

              const std::vector<uint8_t>& session_id() const { return m_new_session_id; };
              Protocol_Version protocol_version() const { return m_new_session_version; };
              uint16_t ciphersuite() const { return m_ciphersuite; };
              uint8_t compression() const { return m_compression; }
              bool offer_session_ticket() const { return m_offer_session_ticket; }

          private:
              const std::vector<uint8_t> m_new_session_id;
              Protocol_Version m_new_session_version;
              uint16_t m_ciphersuite;
              uint8_t m_compression;
              bool m_offer_session_ticket;
      };


      Handshake_Type type() const override { return SERVER_HELLO; }

      Protocol_Version version() const { return m_version; }

      const std::vector<uint8_t>& random() const { return m_random; }

      const std::vector<uint8_t>& session_id() const { return m_session_id; }

      uint16_t ciphersuite() const { return m_ciphersuite; }

      uint8_t compression_method() const { return m_comp_method; }

      bool secure_renegotiation() const
         {
         return m_extensions.has<Renegotiation_Extension>();
         }

      std::vector<uint8_t> renegotiation_info() const
         {
         if(Renegotiation_Extension* reneg = m_extensions.get<Renegotiation_Extension>())
            return reneg->renegotiation_info();
         return std::vector<uint8_t>();
         }

      bool supports_extended_master_secret() const
         {
         return m_extensions.has<Extended_Master_Secret>();
         }

      bool supports_encrypt_then_mac() const
         {
         return m_extensions.has<Encrypt_then_MAC>();
         }

      bool supports_certificate_status_message() const
         {
         return m_extensions.has<Certificate_Status_Request>();
         }

      bool supports_session_ticket() const
         {
         return m_extensions.has<Session_Ticket>();
         }

      uint16_t srtp_profile() const
         {
         if(auto srtp = m_extensions.get<SRTP_Protection_Profiles>())
            {
            auto prof = srtp->profiles();
            if(prof.size() != 1 || prof[0] == 0)
               throw Decoding_Error("Server sent malformed DTLS-SRTP extension");
            return prof[0];
            }

         return 0;
         }

      std::string next_protocol() const
         {
         if(auto alpn = m_extensions.get<Application_Layer_Protocol_Notification>())
            return alpn->single_protocol();
         return "";
         }

      std::set<Handshake_Extension_Type> extension_types() const
         { return m_extensions.extension_types(); }

      bool prefers_compressed_ec_points() const
         {
         if(auto ecc_formats = m_extensions.get<Supported_Point_Formats>())
            {
            return ecc_formats->prefers_compressed();
            }
         return false;
         }

      Server_Hello(Handshake_IO& io,
                   Handshake_Hash& hash,
                   const Policy& policy,
                   RandomNumberGenerator& rng,
                   const std::vector<uint8_t>& secure_reneg_info,
                   const Client_Hello& client_hello,
                   const Server_Hello::Settings& settings,
                   const std::string next_protocol);

      Server_Hello(Handshake_IO& io,
                   Handshake_Hash& hash,
                   const Policy& policy,
                   RandomNumberGenerator& rng,
                   const std::vector<uint8_t>& secure_reneg_info,
                   const Client_Hello& client_hello,
                   Session& resumed_session,
                   bool offer_session_ticket,
                   const std::string& next_protocol);

      explicit Server_Hello(const std::vector<uint8_t>& buf);
   private:
      std::vector<uint8_t> serialize() const override;

      Protocol_Version m_version;
      std::vector<uint8_t> m_session_id, m_random;
      uint16_t m_ciphersuite;
      uint8_t m_comp_method;

      Extensions m_extensions;
   };

/**
* Client Key Exchange Message
*/
class BOTAN_DLL Client_Key_Exchange final : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return CLIENT_KEX; }

      const secure_vector<uint8_t>& pre_master_secret() const
         { return m_pre_master; }

      Client_Key_Exchange(Handshake_IO& io,
                          Handshake_State& state,
                          const Policy& policy,
                          Credentials_Manager& creds,
                          const Public_Key* server_public_key,
                          const std::string& hostname,
                          RandomNumberGenerator& rng);

      Client_Key_Exchange(const std::vector<uint8_t>& buf,
                          const Handshake_State& state,
                          const Private_Key* server_rsa_kex_key,
                          Credentials_Manager& creds,
                          const Policy& policy,
                          RandomNumberGenerator& rng);

   private:
      std::vector<uint8_t> serialize() const override
         { return m_key_material; }

      std::vector<uint8_t> m_key_material;
      secure_vector<uint8_t> m_pre_master;
   };

/**
* Certificate Message
*/
class BOTAN_DLL Certificate final : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return CERTIFICATE; }
      const std::vector<X509_Certificate>& cert_chain() const { return m_certs; }

      size_t count() const { return m_certs.size(); }
      bool empty() const { return m_certs.empty(); }

      Certificate(Handshake_IO& io,
                  Handshake_Hash& hash,
                  const std::vector<X509_Certificate>& certs);

      explicit Certificate(const std::vector<uint8_t>& buf, const Policy &policy);
   private:
      std::vector<uint8_t> serialize() const override;

      std::vector<X509_Certificate> m_certs;
   };

/**
* Certificate Status (RFC 6066)
*/
class BOTAN_DLL Certificate_Status final : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return CERTIFICATE_STATUS; }

      std::shared_ptr<const OCSP::Response> response() const { return m_response; }

      Certificate_Status(const std::vector<uint8_t>& buf);

      Certificate_Status(Handshake_IO& io,
                         Handshake_Hash& hash,
                         std::shared_ptr<const OCSP::Response> response);

   private:
      std::vector<uint8_t> serialize() const override;
      std::shared_ptr<const OCSP::Response> m_response;
   };

/**
* Certificate Request Message
*/
class BOTAN_DLL Certificate_Req final : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return CERTIFICATE_REQUEST; }

      const std::vector<std::string>& acceptable_cert_types() const
         { return m_cert_key_types; }

      std::vector<X509_DN> acceptable_CAs() const { return m_names; }

      std::vector<std::pair<std::string, std::string> > supported_algos() const
         { return m_supported_algos; }

      Certificate_Req(Handshake_IO& io,
                      Handshake_Hash& hash,
                      const Policy& policy,
                      const std::vector<X509_DN>& allowed_cas,
                      Protocol_Version version);

      Certificate_Req(const std::vector<uint8_t>& buf,
                      Protocol_Version version);
   private:
      std::vector<uint8_t> serialize() const override;

      std::vector<X509_DN> m_names;
      std::vector<std::string> m_cert_key_types;

      std::vector<std::pair<std::string, std::string> > m_supported_algos;
   };

/**
* Certificate Verify Message
*/
class BOTAN_DLL Certificate_Verify final : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return CERTIFICATE_VERIFY; }

      /**
      * Check the signature on a certificate verify message
      * @param cert the purported certificate
      * @param state the handshake state
      * @param policy the TLS policy
      */
      bool verify(const X509_Certificate& cert,
                  const Handshake_State& state,
                  const Policy& policy) const;

      Certificate_Verify(Handshake_IO& io,
                         Handshake_State& state,
                         const Policy& policy,
                         RandomNumberGenerator& rng,
                         const Private_Key* key);

      Certificate_Verify(const std::vector<uint8_t>& buf,
                         Protocol_Version version);
   private:
      std::vector<uint8_t> serialize() const override;

      std::string m_sig_algo; // sig algo used to create signature
      std::string m_hash_algo; // hash used to create signature
      std::vector<uint8_t> m_signature;
   };

/**
* Finished Message
*/
class BOTAN_DLL Finished final : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return FINISHED; }

      std::vector<uint8_t> verify_data() const
         { return m_verification_data; }

      bool verify(const Handshake_State& state,
                  Connection_Side side) const;

      Finished(Handshake_IO& io,
               Handshake_State& state,
               Connection_Side side);

      explicit Finished(const std::vector<uint8_t>& buf);
   private:
      std::vector<uint8_t> serialize() const override;

      std::vector<uint8_t> m_verification_data;
   };

/**
* Hello Request Message
*/
class BOTAN_DLL Hello_Request final : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return HELLO_REQUEST; }

      explicit Hello_Request(Handshake_IO& io);
      explicit Hello_Request(const std::vector<uint8_t>& buf);
   private:
      std::vector<uint8_t> serialize() const override;
   };

/**
* Server Key Exchange Message
*/
class BOTAN_DLL Server_Key_Exchange final : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return SERVER_KEX; }

      const std::vector<uint8_t>& params() const { return m_params; }

      bool verify(const Public_Key& server_key,
                  const Handshake_State& state,
                  const Policy& policy) const;

      // Only valid for certain kex types
      const Private_Key& server_kex_key() const;

#if defined(BOTAN_HAS_SRP6)
      // Only valid for SRP negotiation
      SRP6_Server_Session& server_srp_params() const
         {
         BOTAN_ASSERT_NONNULL(m_srp_params);
         return *m_srp_params;
         }
#endif

#if defined(BOTAN_HAS_CECPQ1)
      // Only valid for CECPQ1 negotiation
      const CECPQ1_key& cecpq1_key() const
         {
         BOTAN_ASSERT_NONNULL(m_cecpq1_key);
         return *m_cecpq1_key;
         }
#endif

      Server_Key_Exchange(Handshake_IO& io,
                          Handshake_State& state,
                          const Policy& policy,
                          Credentials_Manager& creds,
                          RandomNumberGenerator& rng,
                          const Private_Key* signing_key = nullptr);

      Server_Key_Exchange(const std::vector<uint8_t>& buf,
                          const std::string& kex_alg,
                          const std::string& sig_alg,
                          Protocol_Version version);

      ~Server_Key_Exchange();
   private:
      std::vector<uint8_t> serialize() const override;

#if defined(BOTAN_HAS_SRP6)
      std::unique_ptr<SRP6_Server_Session> m_srp_params;
#endif

#if defined(BOTAN_HAS_CECPQ1)
      std::unique_ptr<CECPQ1_key> m_cecpq1_key;
#endif

      std::unique_ptr<Private_Key> m_kex_key;

      std::vector<uint8_t> m_params;

      std::string m_sig_algo; // sig algo used to create signature
      std::string m_hash_algo; // hash used to create signature
      std::vector<uint8_t> m_signature;
   };

/**
* Server Hello Done Message
*/
class BOTAN_DLL Server_Hello_Done final : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return SERVER_HELLO_DONE; }

      Server_Hello_Done(Handshake_IO& io, Handshake_Hash& hash);
      explicit Server_Hello_Done(const std::vector<uint8_t>& buf);
   private:
      std::vector<uint8_t> serialize() const override;
   };

/**
* New Session Ticket Message
*/
class BOTAN_DLL New_Session_Ticket final : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return NEW_SESSION_TICKET; }

      uint32_t ticket_lifetime_hint() const { return m_ticket_lifetime_hint; }
      const std::vector<uint8_t>& ticket() const { return m_ticket; }

      New_Session_Ticket(Handshake_IO& io,
                         Handshake_Hash& hash,
                         const std::vector<uint8_t>& ticket,
                         uint32_t lifetime);

      New_Session_Ticket(Handshake_IO& io,
                         Handshake_Hash& hash);

      explicit New_Session_Ticket(const std::vector<uint8_t>& buf);
   private:
      std::vector<uint8_t> serialize() const override;

      uint32_t m_ticket_lifetime_hint = 0;
      std::vector<uint8_t> m_ticket;
   };

/**
* Change Cipher Spec
*/
class BOTAN_DLL Change_Cipher_Spec final : public Handshake_Message
   {
   public:
      Handshake_Type type() const override { return HANDSHAKE_CCS; }

      std::vector<uint8_t> serialize() const override
         { return std::vector<uint8_t>(1, 1); }
   };

}

}


namespace Botan {

namespace TLS {

/**
* An implementation of Session_Manager that saves values in a SQL
* database file, with the session data encrypted using a passphrase.
*
* @warning For clients, the hostnames associated with the saved
* sessions are stored in the database in plaintext. This may be a
* serious privacy risk in some situations.
*/
class BOTAN_DLL Session_Manager_SQL : public Session_Manager
   {
   public:
      /**
      * @param db A connection to the database to use
               The table names botan_tls_sessions and
               botan_tls_sessions_metadata will be used
      * @param passphrase used to encrypt the session data
      * @param rng a random number generator
      * @param max_sessions a hint on the maximum number of sessions
      *        to keep in memory at any one time. (If zero, don't cap)
      * @param session_lifetime sessions are expired after this many
      *        seconds have elapsed from initial handshake.
      */
      Session_Manager_SQL(std::shared_ptr<SQL_Database> db,
                          const std::string& passphrase,
                          RandomNumberGenerator& rng,
                          size_t max_sessions = 1000,
                          std::chrono::seconds session_lifetime = std::chrono::seconds(7200));

      Session_Manager_SQL(const Session_Manager_SQL&) = delete;

      Session_Manager_SQL& operator=(const Session_Manager_SQL&) = delete;

      bool load_from_session_id(const std::vector<uint8_t>& session_id,
                                Session& session) override;

      bool load_from_server_info(const Server_Information& info,
                                 Session& session) override;

      void remove_entry(const std::vector<uint8_t>& session_id) override;

      size_t remove_all() override;

      void save(const Session& session_data) override;

      std::chrono::seconds session_lifetime() const override
         { return m_session_lifetime; }

   private:
      void prune_session_cache();

      std::shared_ptr<SQL_Database> m_db;
      secure_vector<uint8_t> m_session_key;
      RandomNumberGenerator& m_rng;
      size_t m_max_sessions;
      std::chrono::seconds m_session_lifetime;
   };

}

}


namespace Botan {

/**
* A split secret, using the format from draft-mcgrew-tss-03
*/
class BOTAN_DLL RTSS_Share
   {
   public:
      /**
      * @param M the number of shares needed to reconstruct
      * @param N the number of shares generated
      * @param secret the secret to split
      * @param secret_len the length of the secret
      * @param identifier the 16 byte share identifier
      * @param rng the random number generator to use
      */
      static std::vector<RTSS_Share>
         split(uint8_t M, uint8_t N,
               const uint8_t secret[], uint16_t secret_len,
               const uint8_t identifier[16],
               RandomNumberGenerator& rng);

      /**
      * @param shares the list of shares
      */
      static secure_vector<uint8_t>
        reconstruct(const std::vector<RTSS_Share>& shares);

      RTSS_Share() {}

      /**
      * @param hex_input the share encoded in hexadecimal
      */
      explicit RTSS_Share(const std::string& hex_input);

      /**
      * @return hex representation
      */
      std::string to_string() const;

      /**
      * @return share identifier
      */
      uint8_t share_id() const;

      /**
      * @return size of this share in bytes
      */
      size_t size() const { return m_contents.size(); }

      /**
      * @return if this TSS share was initialized or not
      */
      bool initialized() const { return (m_contents.size() > 0); }
   private:
      secure_vector<uint8_t> m_contents;
   };

}


namespace Botan {

/**
* Twofish, an AES finalist
*/
class BOTAN_DLL Twofish final : public Block_Cipher_Fixed_Params<16, 16, 32, 8>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "Twofish"; }
      BlockCipher* clone() const override { return new Twofish; }
   private:
      void key_schedule(const uint8_t[], size_t) override;

      static const uint32_t MDS0[256];
      static const uint32_t MDS1[256];
      static const uint32_t MDS2[256];
      static const uint32_t MDS3[256];
      static const uint8_t Q0[256];
      static const uint8_t Q1[256];
      static const uint8_t RS[32];
      static const uint8_t EXP_TO_POLY[255];
      static const uint8_t POLY_TO_EXP[255];

      secure_vector<uint32_t> m_SB, m_RK;
   };

}


namespace Botan {

/**
* Whirlpool
*/
class BOTAN_DLL Whirlpool final : public MDx_HashFunction
   {
   public:
      std::string name() const override { return "Whirlpool"; }
      size_t output_length() const override { return 64; }
      HashFunction* clone() const override { return new Whirlpool; }

      void clear() override;

      Whirlpool() : MDx_HashFunction(64, true, true, 32), m_M(8), m_digest(8)
         { clear(); }
   private:
      void compress_n(const uint8_t[], size_t blocks) override;
      void copy_out(uint8_t[]) override;

      static const uint64_t C0[256];
      static const uint64_t C1[256];
      static const uint64_t C2[256];
      static const uint64_t C3[256];
      static const uint64_t C4[256];
      static const uint64_t C5[256];
      static const uint64_t C6[256];
      static const uint64_t C7[256];

      secure_vector<uint64_t> m_M, m_digest;
   };

}


namespace Botan {

/**
* Estimate work factor for discrete logarithm
* @param prime_group_size size of the group in bits
* @return estimated security level for this group
*/
BOTAN_DLL size_t dl_work_factor(size_t prime_group_size);

/**
* Return the appropriate exponent size to use for a particular prime
* group. This is twice the size of the estimated cost of breaking the
* key using an index calculus attack; the assumption is that if an
* arbitrary discrete log on a group of size bits would take about 2^n
* effort, and thus using an exponent of size 2^(2*n) implies that all
* available attacks are about as easy (as e.g Pollard's kangaroo
* algorithm can compute the DL in sqrt(x) operations) while minimizing
* the exponent size for performance reasons.
*/
BOTAN_DLL size_t dl_exponent_size(size_t prime_group_size);

/**
* Estimate work factor for integer factorization
* @param n_bits size of modulus in bits
* @return estimated security level for this modulus
*/
BOTAN_DLL size_t if_work_factor(size_t n_bits);

/**
* Estimate work factor for EC discrete logarithm
* @param prime_group_size size of the group in bits
* @return estimated security level for this group
*/
BOTAN_DLL size_t ecp_work_factor(size_t prime_group_size);

}


#if defined(BOTAN_HAS_SYSTEM_RNG)
#endif

namespace Botan {

class PK_Signer;

/**
* This class represents X.509 Certificate Authorities (CAs).
*/
class BOTAN_DLL X509_CA
   {
   public:
      /**
      * Sign a PKCS#10 Request.
      * @param req the request to sign
      * @param rng the rng to use
      * @param not_before the starting time for the certificate
      * @param not_after the expiration time for the certificate
      * @return resulting certificate
      */
      X509_Certificate sign_request(const PKCS10_Request& req,
                                    RandomNumberGenerator& rng,
                                    const X509_Time& not_before,
                                    const X509_Time& not_after);

      /**
      * Get the certificate of this CA.
      * @return CA certificate
      */
      X509_Certificate ca_certificate() const;

      /**
      * Create a new and empty CRL for this CA.
      * @param rng the random number generator to use
      * @param next_update the time to set in next update in seconds
      * as the offset from the current time
      * @return new CRL
      */
      X509_CRL new_crl(RandomNumberGenerator& rng,
                       uint32_t next_update = 0) const;

      /**
      * Create a new CRL by with additional entries.
      * @param last_crl the last CRL of this CA to add the new entries to
      * @param new_entries contains the new CRL entries to be added to the CRL
      * @param rng the random number generator to use
      * @param next_update the time to set in next update in seconds
      * as the offset from the current time
      */
      X509_CRL update_crl(const X509_CRL& last_crl,
                          const std::vector<CRL_Entry>& new_entries,
                          RandomNumberGenerator& rng,
                          uint32_t next_update = 0) const;

      /**
      * Interface for creating new certificates
      * @param signer a signing object
      * @param rng a random number generator
      * @param sig_algo the signature algorithm identifier
      * @param pub_key the serialized public key
      * @param not_before the start time of the certificate
      * @param not_after the end time of the certificate
      * @param issuer_dn the DN of the issuer
      * @param subject_dn the DN of the subject
      * @param extensions an optional list of certificate extensions
      * @returns newly minted certificate
      */
      static X509_Certificate make_cert(PK_Signer* signer,
                                        RandomNumberGenerator& rng,
                                        const AlgorithmIdentifier& sig_algo,
                                        const std::vector<uint8_t>& pub_key,
                                        const X509_Time& not_before,
                                        const X509_Time& not_after,
                                        const X509_DN& issuer_dn,
                                        const X509_DN& subject_dn,
                                        const Extensions& extensions);

      /**
      * Create a new CA object.
      * @param ca_certificate the certificate of the CA
      * @param key the private key of the CA
      * @param hash_fn name of a hash function to use for signing
      * @param rng the random generator to use
      */
      X509_CA(const X509_Certificate& ca_certificate,
              const Private_Key& key,
              const std::string& hash_fn,
              RandomNumberGenerator& rng);

#if defined(BOTAN_HAS_SYSTEM_RNG)
      BOTAN_DEPRECATED("Use version taking RNG object")
      X509_CA(const X509_Certificate& ca_certificate,
              const Private_Key& key,
              const std::string& hash_fn) :
         X509_CA(ca_certificate, key, hash_fn, system_rng())
         {}
#endif

      X509_CA(const X509_CA&) = delete;
      X509_CA& operator=(const X509_CA&) = delete;

      ~X509_CA();
   private:
      X509_CRL make_crl(const std::vector<CRL_Entry>& entries,
                        uint32_t crl_number, uint32_t next_update,
                        RandomNumberGenerator& rng) const;

      AlgorithmIdentifier m_ca_sig_algo;
      X509_Certificate m_cert;
      PK_Signer* m_signer;
   };

/**
* Choose the default signature format for a certain public key signature
* scheme.
* @param key will be the key to choose a padding scheme for
* @param rng the random generator to use
* @param hash_fn is the desired hash function
* @param alg_id will be set to the chosen scheme
* @return A PK_Signer object for generating signatures
*/
BOTAN_DLL PK_Signer* choose_sig_format(const Private_Key& key,
                                       RandomNumberGenerator& rng,
                                       const std::string& hash_fn,
                                       AlgorithmIdentifier& alg_id);

}


#if defined(BOTAN_TARGET_OS_HAS_THREADS) && defined(BOTAN_HAS_HTTP_UTIL)
  #define BOTAN_HAS_ONLINE_REVOCATION_CHECKS
#endif

namespace Botan {

/**
* This type represents the validation status of an entire certificate path.
* There is one set of status codes for each certificate in the path.
*/
typedef std::vector<std::set<Certificate_Status_Code>> CertificatePathStatusCodes;

/**
* Specifies restrictions on the PKIX path validation
*/
class BOTAN_DLL Path_Validation_Restrictions
   {
   public:
      /**
      * @param require_rev if true, revocation information is required

      * @param minimum_key_strength is the minimum strength (in terms of
      *    operations, eg 80 means 2^80) of a signature. Signatures weaker than
      *    this are rejected. If more than 80, SHA-1 signatures are also
      *    rejected. If possible use at least setting 110.
      *
      *        80 bit strength requires 1024 bit RSA
      *        110 bit strength requires 2k bit RSA
      *        128 bit strength requires ~3k bit RSA or P-256
      * @param ocsp_all_intermediates Make OCSP requests for all CAs as
      * well as end entity (if OCSP enabled in path validation request)
      */
      Path_Validation_Restrictions(bool require_rev = false,
                                   size_t minimum_key_strength = 110,
                                   bool ocsp_all_intermediates = false);

      /**
      * @param require_rev if true, revocation information is required
      * @param minimum_key_strength is the minimum strength (in terms of
      *        operations, eg 80 means 2^80) of a signature. Signatures
      *        weaker than this are rejected.
      * @param ocsp_all_intermediates Make OCSP requests for all CAs as
      * well as end entity (if OCSP enabled in path validation request)
      * @param trusted_hashes a set of trusted hashes. Any signatures
      *        created using a hash other than one of these will be
      *        rejected.
      */
      Path_Validation_Restrictions(bool require_rev,
                                   size_t minimum_key_strength,
                                   bool ocsp_all_intermediates,
                                   const std::set<std::string>& trusted_hashes) :
         m_require_revocation_information(require_rev),
         m_ocsp_all_intermediates(ocsp_all_intermediates),
         m_trusted_hashes(trusted_hashes),
         m_minimum_key_strength(minimum_key_strength) {}

      /**
      * @return whether revocation information is required
      */
      bool require_revocation_information() const
         { return m_require_revocation_information; }

      /**
      * @return whether all intermediate CAs should also be OCSPed. If false
      * then only end entity OCSP is required/requested.
      */
      bool ocsp_all_intermediates() const
         { return m_ocsp_all_intermediates; }

      /**
      * @return trusted signature hash functions
      */
      const std::set<std::string>& trusted_hashes() const
         { return m_trusted_hashes; }

      /**
      * @return minimum required key strength
      */
      size_t minimum_key_strength() const
         { return m_minimum_key_strength; }

   private:
      bool m_require_revocation_information;
      bool m_ocsp_all_intermediates;
      std::set<std::string> m_trusted_hashes;
      size_t m_minimum_key_strength;
   };

/**
* Represents the result of a PKIX path validation
*/
class BOTAN_DLL Path_Validation_Result
   {
   public:
      typedef Certificate_Status_Code Code;

      /**
      * @return the set of hash functions you are implicitly
      * trusting by trusting this result.
      */
      std::set<std::string> trusted_hashes() const;

      /**
      * @return the trust root of the validation if successful
      * throws an exception if the validation failed
      */
      const X509_Certificate& trust_root() const;

      /**
      * @return the full path from subject to trust root
      * This path may be empty
      */
      const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path() const { return m_cert_path; }

      /**
      * @return true iff the validation was successful
      */
      bool successful_validation() const;

      /**
      * @return overall validation result code
      */
      Certificate_Status_Code result() const { return m_overall; }

      /**
      * @return a set of status codes for each certificate in the chain
      */
      const CertificatePathStatusCodes& all_statuses() const
         { return m_all_status; }

      /**
      * @return string representation of the validation result
      */
      std::string result_string() const;

      /**
      * @param code validation status code
      * @return corresponding validation status message
      */
      static const char* status_string(Certificate_Status_Code code);

      /**
      * Create a Path_Validation_Result
      * @param status list of validation status codes
      * @param cert_chain the certificate chain that was validated
      */
      Path_Validation_Result(CertificatePathStatusCodes status,
                             std::vector<std::shared_ptr<const X509_Certificate>>&& cert_chain);

      /**
      * Create a Path_Validation_Result
      * @param status validation status code
      */
      explicit Path_Validation_Result(Certificate_Status_Code status) : m_overall(status) {}

   private:
      CertificatePathStatusCodes m_all_status;
      std::vector<std::shared_ptr<const X509_Certificate>> m_cert_path;
      Certificate_Status_Code m_overall;
   };

/**
* PKIX Path Validation
* @param end_certs certificate chain to validate
* @param restrictions path validation restrictions
* @param trusted_roots list of certificate stores that contain trusted certificates
* @param hostname if not empty, compared against the DNS name in end_certs[0]
* @param usage if not set to UNSPECIFIED, compared against the key usage in end_certs[0]
* @param validation_time what reference time to use for validation
* @param ocsp_timeout timeout for OCSP operations, 0 disables OCSP check
* @param ocsp_resp additional OCSP responses to consider (eg from peer)
* @return result of the path validation
*/
Path_Validation_Result BOTAN_DLL x509_path_validate(
   const std::vector<X509_Certificate>& end_certs,
   const Path_Validation_Restrictions& restrictions,
   const std::vector<Certificate_Store*>& trusted_roots,
   const std::string& hostname = "",
   Usage_Type usage = Usage_Type::UNSPECIFIED,
   std::chrono::system_clock::time_point validation_time = std::chrono::system_clock::now(),
   std::chrono::milliseconds ocsp_timeout = std::chrono::milliseconds(0),
   const std::vector<std::shared_ptr<const OCSP::Response>>& ocsp_resp = {});

/**
* PKIX Path Validation
* @param end_cert certificate to validate
* @param restrictions path validation restrictions
* @param trusted_roots list of stores that contain trusted certificates
* @param hostname if not empty, compared against the DNS name in end_cert
* @param usage if not set to UNSPECIFIED, compared against the key usage in end_cert
* @param validation_time what reference time to use for validation
* @param ocsp_timeout timeoutput for OCSP operations, 0 disables OCSP check
* @param ocsp_resp additional OCSP responses to consider (eg from peer)
* @return result of the path validation
*/
Path_Validation_Result BOTAN_DLL x509_path_validate(
   const X509_Certificate& end_cert,
   const Path_Validation_Restrictions& restrictions,
   const std::vector<Certificate_Store*>& trusted_roots,
   const std::string& hostname = "",
   Usage_Type usage = Usage_Type::UNSPECIFIED,
   std::chrono::system_clock::time_point validation_time = std::chrono::system_clock::now(),
   std::chrono::milliseconds ocsp_timeout = std::chrono::milliseconds(0),
   const std::vector<std::shared_ptr<const OCSP::Response>>& ocsp_resp = {});

/**
* PKIX Path Validation
* @param end_cert certificate to validate
* @param restrictions path validation restrictions
* @param store store that contains trusted certificates
* @param hostname if not empty, compared against the DNS name in end_cert
* @param usage if not set to UNSPECIFIED, compared against the key usage in end_cert
* @param validation_time what reference time to use for validation
* @param ocsp_timeout timeoutput for OCSP operations, 0 disables OCSP check
* @param ocsp_resp additional OCSP responses to consider (eg from peer)
* @return result of the path validation
*/
Path_Validation_Result BOTAN_DLL x509_path_validate(
   const X509_Certificate& end_cert,
   const Path_Validation_Restrictions& restrictions,
   const Certificate_Store& store,
   const std::string& hostname = "",
   Usage_Type usage = Usage_Type::UNSPECIFIED,
   std::chrono::system_clock::time_point validation_time = std::chrono::system_clock::now(),
   std::chrono::milliseconds ocsp_timeout = std::chrono::milliseconds(0),
   const std::vector<std::shared_ptr<const OCSP::Response>>& ocsp_resp = {});

/**
* PKIX Path Validation
* @param end_certs certificate chain to validate
* @param restrictions path validation restrictions
* @param store store that contains trusted certificates
* @param hostname if not empty, compared against the DNS name in end_certs[0]
* @param usage if not set to UNSPECIFIED, compared against the key usage in end_certs[0]
* @param validation_time what reference time to use for validation
* @param ocsp_timeout timeoutput for OCSP operations, 0 disables OCSP check
* @param ocsp_resp additional OCSP responses to consider (eg from peer)
* @return result of the path validation
*/
Path_Validation_Result BOTAN_DLL x509_path_validate(
   const std::vector<X509_Certificate>& end_certs,
   const Path_Validation_Restrictions& restrictions,
   const Certificate_Store& store,
   const std::string& hostname = "",
   Usage_Type usage = Usage_Type::UNSPECIFIED,
   std::chrono::system_clock::time_point validation_time = std::chrono::system_clock::now(),
   std::chrono::milliseconds ocsp_timeout = std::chrono::milliseconds(0),
   const std::vector<std::shared_ptr<const OCSP::Response>>& ocsp_resp = {});


/**
* namespace PKIX holds the building blocks that are called by x509_path_validate.
* This allows custom validation logic to be written by applications and makes
* for easier testing, but unless you're positive you know what you're doing you
* probably want to just call x509_path_validate instead.
*/
namespace PKIX {

/**
* Build certificate path
* @param cert_path_out output parameter, cert_path will be appended to this vector
* @param trusted_certstores list of certificate stores that contain trusted certificates
* @param end_entity the cert to be validated
* @param end_entity_extra optional list of additional untrusted certs for path building
* @return result of the path building operation (OK or error)
*/
Certificate_Status_Code
BOTAN_DLL build_certificate_path(std::vector<std::shared_ptr<const X509_Certificate>>& cert_path_out,
                                 const std::vector<Certificate_Store*>& trusted_certstores,
                                 const std::shared_ptr<const X509_Certificate>& end_entity,
                                 const std::vector<std::shared_ptr<const X509_Certificate>>& end_entity_extra);

/**
* Check the certificate chain, but not any revocation data
*
* @param cert_path path built by build_certificate_path with OK result
* @param ref_time whatever time you want to perform the validation
* against (normally current system clock)
* @param hostname the hostname
* @param usage end entity usage checks
* @param min_signature_algo_strength 80 or 110 typically
* Note 80 allows 1024 bit RSA and SHA-1. 110 allows 2048 bit RSA and SHA-2.
* Using 128 requires ECC (P-256) or ~3000 bit RSA keys.
* @param trusted_hashes set of trusted hash functions, empty means accept any
* hash we have an OID for
* @return vector of results on per certificate in the path, each containing a set of
* results. If all codes in the set are < Certificate_Status_Code::FIRST_ERROR_STATUS,
* then the result for that certificate is successful. If all results are
*/
CertificatePathStatusCodes
BOTAN_DLL check_chain(const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path,
                      std::chrono::system_clock::time_point ref_time,
                      const std::string& hostname,
                      Usage_Type usage,
                      size_t min_signature_algo_strength,
                      const std::set<std::string>& trusted_hashes);

/**
* Check OCSP responses for revocation information
* @param cert_path path already validated by check_chain
* @param ocsp_responses the OCSP responses to consider
* @param certstores trusted roots
* @param ref_time whatever time you want to perform the validation against
* (normally current system clock)
* @return revocation status
*/
CertificatePathStatusCodes
BOTAN_DLL check_ocsp(const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path,
                     const std::vector<std::shared_ptr<const OCSP::Response>>& ocsp_responses,
                     const std::vector<Certificate_Store*>& certstores,
                     std::chrono::system_clock::time_point ref_time);

/**
* Check CRLs for revocation infomration
* @param cert_path path already validated by check_chain
* @param crls the list of CRLs to check, it is assumed that crls[i] (if not null)
* is the associated CRL for the subject in cert_path[i].
* @param ref_time whatever time you want to perform the validation against
* (normally current system clock)
* @return revocation status
*/
CertificatePathStatusCodes
BOTAN_DLL check_crl(const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path,
                    const std::vector<std::shared_ptr<const X509_CRL>>& crls,
                    std::chrono::system_clock::time_point ref_time);

/**
* Check CRLs for revocation infomration
* @param cert_path path already validated by check_chain
* @param certstores a list of certificate stores to query for the CRL
* @param ref_time whatever time you want to perform the validation against
* (normally current system clock)
* @return revocation status
*/
CertificatePathStatusCodes
BOTAN_DLL check_crl(const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path,
                    const std::vector<Certificate_Store*>& certstores,
                    std::chrono::system_clock::time_point ref_time);

#if defined(BOTAN_HAS_ONLINE_REVOCATION_CHECKS)

/**
* Check OCSP using online (HTTP) access. Current version creates a thread and
* network connection per OCSP request made.
*
* @param cert_path path already validated by check_chain
* @param trusted_certstores a list of certstores with trusted certs
* @param ref_time whatever time you want to perform the validation against
* (normally current system clock)
* @param timeout for timing out the responses, though actually this function
* may block for up to timeout*cert_path.size()*C for some small C.
* @param ocsp_check_intermediate_CAs if true also performs OCSP on any intermediate
* CA certificates. If false, only does OCSP on the end entity cert.
* @return revocation status
*/
CertificatePathStatusCodes
BOTAN_DLL check_ocsp_online(const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path,
                            const std::vector<Certificate_Store*>& trusted_certstores,
                            std::chrono::system_clock::time_point ref_time,
                            std::chrono::milliseconds timeout,
                            bool ocsp_check_intermediate_CAs);

/**
* Check CRL using online (HTTP) access. Current version creates a thread and
* network connection per CRL access.

* @param cert_path path already validated by check_chain
* @param trusted_certstores a list of certstores with trusted certs
* @param certstore_to_recv_crls optional (nullptr to disable), all CRLs
* retreived will be saved to this cert store.
* @param ref_time whatever time you want to perform the validation against
* (normally current system clock)
* @param timeout for timing out the responses, though actually this function
* may block for up to timeout*cert_path.size()*C for some small C.
* @return revocation status
*/
CertificatePathStatusCodes
BOTAN_DLL check_crl_online(const std::vector<std::shared_ptr<const X509_Certificate>>& cert_path,
                           const std::vector<Certificate_Store*>& trusted_certstores,
                           Certificate_Store_In_Memory* certstore_to_recv_crls,
                           std::chrono::system_clock::time_point ref_time,
                           std::chrono::milliseconds timeout);

#endif

/**
* Find overall status (OK, error) of a validation
* @param cert_status result of merge_revocation_status or check_chain
*/
Certificate_Status_Code BOTAN_DLL overall_status(const CertificatePathStatusCodes& cert_status);

/**
* Merge the results from CRL and/or OCSP checks into chain_status
* @param chain_status the certificate status
* @param crl_status results from check_crl
* @param ocsp_status results from check_ocsp
* @param require_rev_on_end_entity require valid CRL or OCSP on end-entity cert
* @param require_rev_on_intermediates require valid CRL or OCSP on all intermediate certificates
*/
void BOTAN_DLL merge_revocation_status(CertificatePathStatusCodes& chain_status,
                                       const CertificatePathStatusCodes& crl_status,
                                       const CertificatePathStatusCodes& ocsp_status,
                                       bool require_rev_on_end_entity,
                                       bool require_rev_on_intermediates);

}

}


namespace Botan {

/**
* Options for X.509 certificates.
*/
class BOTAN_DLL X509_Cert_Options
   {
   public:
      /**
      * the subject common name
      */
      std::string common_name;

      /**
      * the subject counry
      */
      std::string country;

      /**
      * the subject organization
      */
      std::string organization;

      /**
      * the subject organizational unit
      */
      std::string org_unit;

      /**
      * the subject locality
      */
      std::string locality;

      /**
      * the subject state
      */
      std::string state;

      /**
      * the subject serial number
      */
      std::string serial_number;

      /**
      * the subject email adress
      */
      std::string email;

      /**
      * the subject URI
      */
      std::string uri;

      /**
      * the subject IPv4 address
      */
      std::string ip;

      /**
      * the subject DNS
      */
      std::string dns;

      /**
      * the subject XMPP
      */
      std::string xmpp;

      /**
      * the subject challenge password
      */
      std::string challenge;

      /**
      * the subject notBefore
      */
      X509_Time start;
      /**
      * the subject notAfter
      */
      X509_Time end;

      /**
      * Indicates whether the certificate request
      */
      bool is_CA;

      /**
      * Indicates the BasicConstraints path limit
      */
      size_t path_limit;

      /**
      * The key constraints for the subject public key
      */
      Key_Constraints constraints;

      /**
      * The key extended constraints for the subject public key
      */
      std::vector<OID> ex_constraints;

      /**
      * Additional X.509 extensions
      */
      Extensions extensions;

      /**
      * Mark the certificate as a CA certificate and set the path limit.
      * @param limit the path limit to be set in the BasicConstraints extension.
      */
      void CA_key(size_t limit = 1);

      /**
      * Set the notBefore of the certificate.
      * @param time the notBefore value of the certificate
      */
      void not_before(const std::string& time);

      /**
      * Set the notAfter of the certificate.
      * @param time the notAfter value of the certificate
      */
      void not_after(const std::string& time);

      /**
      * Add the key constraints of the KeyUsage extension.
      * @param constr the constraints to set
      */
      void add_constraints(Key_Constraints constr);

      /**
      * Add constraints to the ExtendedKeyUsage extension.
      * @param oid the oid to add
      */
      void add_ex_constraint(const OID& oid);

      /**
      * Add constraints to the ExtendedKeyUsage extension.
      * @param name the name to look up the oid to add
      */
      void add_ex_constraint(const std::string& name);

      /**
      * Construct a new options object
      * @param opts define the common name of this object. An example for this
      * parameter would be "common_name/country/organization/organizational_unit".
      * @param expire_time the expiration time (from the current clock in seconds)
      */
      X509_Cert_Options(const std::string& opts = "",
                        uint32_t expire_time = 365 * 24 * 60 * 60);
   };

namespace X509 {

/**
* Create a self-signed X.509 certificate.
* @param opts the options defining the certificate to create
* @param key the private key used for signing, i.e. the key
* associated with this self-signed certificate
* @param hash_fn the hash function to use
* @param rng the rng to use
* @return newly created self-signed certificate
*/
BOTAN_DLL X509_Certificate
create_self_signed_cert(const X509_Cert_Options& opts,
                        const Private_Key& key,
                        const std::string& hash_fn,
                        RandomNumberGenerator& rng);

/**
* Create a PKCS#10 certificate request.
* @param opts the options defining the request to create
* @param key the key used to sign this request
* @param rng the rng to use
* @param hash_fn the hash function to use
* @return newly created PKCS#10 request
*/
BOTAN_DLL PKCS10_Request create_cert_req(const X509_Cert_Options& opts,
                                         const Private_Key& key,
                                         const std::string& hash_fn,
                                         RandomNumberGenerator& rng);

}

}


namespace Botan {

/**
* DES/3DES-based MAC from ANSI X9.19
*/
class BOTAN_DLL ANSI_X919_MAC final : public MessageAuthenticationCode
   {
   public:
      void clear() override;
      std::string name() const override;
      size_t output_length() const override { return 8; }

      MessageAuthenticationCode* clone() const override;

      Key_Length_Specification key_spec() const override
         {
         return Key_Length_Specification(8, 16, 8);
         }

      ANSI_X919_MAC();

      ANSI_X919_MAC(const ANSI_X919_MAC&) = delete;
      ANSI_X919_MAC& operator=(const ANSI_X919_MAC&) = delete;
   private:
      void add_data(const uint8_t[], size_t) override;
      void final_result(uint8_t[]) override;
      void key_schedule(const uint8_t[], size_t) override;

      std::unique_ptr<BlockCipher> m_des1, m_des2;
      secure_vector<uint8_t> m_state;
      size_t m_position;
   };

}

#include <math.h>
#include <stdint.h>

namespace Botan {

/**
 * Helper tools for low level byte operations required
 * for the XMSS implementation.
 **/
 class XMSS_Tools
   {
   public:
      XMSS_Tools(const XMSS_Tools&) = delete;
      void operator=(const XMSS_Tools&) = delete;

      /**
       * Concatenates the byte representation in big-endian order of any
       * integral value to a secure_vector.
       *
       * @param target Vector to concatenate the byte representation of the
       *               integral value to.
       * @param src integral value to concatenate.
       **/
      template<typename T,
               typename U = typename std::enable_if<std::is_integral<T>::value,
                                                    void>::type>
      static void concat(secure_vector<uint8_t>& target, const T& src);

      /**
       * Concatenates the last n bytes of the byte representation in big-endian
       * order of any integral value to a to a secure_vector.
       *
       * @param target Vector to concatenate the byte representation of the
       *               integral value to.
       * @param src Integral value to concatenate.
       * @param len number of bytes to concatenate. This value must be smaller
       *            or equal to the size of type T.
       **/
      template <typename T,
                typename U = typename std::enable_if<std::is_integral<T>::value,
                void>::type>
      static void concat(secure_vector<uint8_t>& target, const T& src, size_t len);

   private:
      XMSS_Tools();
   };

template <typename T, typename U>
void XMSS_Tools::concat(secure_vector<uint8_t>& target, const T& src)
   {
   const uint8_t* src_bytes = reinterpret_cast<const uint8_t*>(&src);
   if(CPUID::is_little_endian())
      {
      std::reverse_copy(src_bytes,
                        src_bytes + sizeof(src),
                        std::back_inserter(target));
      }
   else
      {
      std::copy(src_bytes,
                src_bytes + sizeof(src),
                std::back_inserter(target));
      }
   }


template <typename T, typename U>
void XMSS_Tools::concat(secure_vector<uint8_t>& target,
                        const T& src,
                        size_t len)
   {
   size_t c = static_cast<size_t>(std::min(len, sizeof(src)));
   if(len > sizeof(src))
      {
      target.resize(target.size() + len - sizeof(src), 0);
      }

   const uint8_t* src_bytes = reinterpret_cast<const uint8_t*>(&src);
   if(CPUID::is_little_endian())
      {
      std::reverse_copy(src_bytes,
                        src_bytes + c,
                        std::back_inserter(target));
      }
   else
      {
      std::copy(src_bytes + sizeof(src) - c,
                src_bytes + sizeof(src),
                std::back_inserter(target));
      }
   }
}


namespace Botan {

/**
 * Descibes a signature method for XMSS Winternitz One Time Signatures,
 * as defined in:
 * [1] XMSS: Extended Hash-Based Signatures,
 *     draft-itrf-cfrg-xmss-hash-based-signatures-06
 *     Release: July 2016.
 *     https://datatracker.ietf.org/doc/
 *     draft-irtf-cfrg-xmss-hash-based-signatures/?include_text=1
 **/
class XMSS_WOTS_Parameters
   {
   public:
      enum ots_algorithm_t
         {
         WOTSP_SHA2_256_W16 = 0x01000001,
         WOTSP_SHA2_512_W16 = 0x02000002,
         WOTSP_SHAKE128_W16 = 0x03000003,
         WOTSP_SHAKE256_W16 = 0x04000004
         };

      XMSS_WOTS_Parameters(const std::string& algo_name);
      XMSS_WOTS_Parameters(ots_algorithm_t ots_spec);

      static ots_algorithm_t xmss_wots_id_from_string(const std::string& param_set);

      /**
       * Algorithm 1: convert input string to base.
       *
       * @param msg Input string (referred to as X in [1]).
       * @param out_size size of message in base w.
       *
       * @return Input string converted to the given base.
       **/
      secure_vector<uint8_t> base_w(const secure_vector<uint8_t>& msg, size_t out_size) const;

      secure_vector<uint8_t> base_w(size_t value) const;

      void append_checksum(secure_vector<uint8_t>& data);

      /**
       * @return XMSS WOTS registry name for the chosen parameter set.
       **/
      const std::string& name() const
         {
         return m_name;
         }

      /**
       * @return Botan name for the hash function used.
       **/
      const std::string& hash_function_name() const
         {
         return m_hash_name;
         }

      /**
       * Retrieves the uniform length of a message, and the size of
       * each node. This correlates to XMSS parameter "n" defined
       * in [1].
       *
       * @return element length in bytes.
       **/
      size_t element_size() const { return m_element_size; }

      /**
       * The Winternitz parameter.
       *
       * @return numeric base used for internal representation of
       *         data.
       **/
      size_t wots_parameter() const { return m_w; }

      size_t len() const { return m_len; }

      size_t len_1() const { return m_len_1; }

      size_t len_2() const { return m_len_2; }

      size_t lg_w() const { return m_lg_w; }

      ots_algorithm_t oid() const { return m_oid; }

      size_t estimated_strength() const { return m_strength; }

      bool operator==(const XMSS_WOTS_Parameters& p) const
         {
         return m_oid == p.m_oid;
         }

   private:
      static const std::map<std::string, ots_algorithm_t> m_oid_name_lut;
      ots_algorithm_t m_oid;
      std::string m_name;
      std::string m_hash_name;
      size_t m_element_size;
      size_t m_w;
      size_t m_len_1;
      size_t m_len_2;
      size_t m_len;
      size_t m_strength;
      uint8_t m_lg_w;
   };

}


namespace Botan {

/**
 * Descibes a signature method for XMSS, as defined in:
 * [1] XMSS: Extended Hash-Based Signatures,
 *     draft-itrf-cfrg-xmss-hash-based-signatures-06
 *     Release: July 2016.
 *     https://datatracker.ietf.org/doc/
 *     draft-irtf-cfrg-xmss-hash-based-signatures/?include_text=1
 **/
class BOTAN_DLL XMSS_Parameters
   {
   public:
      enum xmss_algorithm_t
         {
         XMSS_SHA2_256_W16_H10 = 0x01000001,
         XMSS_SHA2_256_W16_H16 = 0x02000002,
         XMSS_SHA2_256_W16_H20 = 0x03000003,
         XMSS_SHA2_512_W16_H10 = 0x04000004,
         XMSS_SHA2_512_W16_H16 = 0x05000005,
         XMSS_SHA2_512_W16_H20 = 0x06000006,
         XMSS_SHAKE128_W16_H10 = 0x07000007,
         XMSS_SHAKE128_W16_H16 = 0x08000008,
         XMSS_SHAKE128_W16_H20 = 0x09000009,
         XMSS_SHAKE256_W16_H10 = 0x0a00000a,
         XMSS_SHAKE256_W16_H16 = 0x0b00000b,
         XMSS_SHAKE256_W16_H20 = 0x0c00000c
         };

      static xmss_algorithm_t xmss_id_from_string(const std::string& algo_name);

      XMSS_Parameters(const std::string& algo_name);
      XMSS_Parameters(xmss_algorithm_t oid);

      /**
       * @return XMSS registry name for the chosen parameter set.
       **/
      const std::string& name() const
         {
         return m_name;
         }

      const std::string& hash_function_name() const
         {
         return m_hash_name;
         }

      /**
       * Retrieves the uniform length of a message, and the size of
       * each node. This correlates to XMSS parameter "n" defined
       * in [1].
       *
       * @return element length in bytes.
       **/
      size_t element_size() const { return m_element_size; }

      /**
       * @returns The height (number of levels - 1) of the tree
       **/
      size_t tree_height() const { return m_tree_height; }

      /**
       * The Winternitz parameter.
       *
       * @return numeric base used for internal representation of
       *         data.
       **/
      size_t wots_parameter() const { return m_w; }

      size_t len() const { return m_len; }

      xmss_algorithm_t oid() const { return m_oid; }

      XMSS_WOTS_Parameters::ots_algorithm_t ots_oid() const
         {
         return m_wots_oid;
         }

      /**
       * Returns the estimated pre-quantum security level of
       * the chosen algorithm.
       **/
      size_t estimated_strength() const
         {
         return m_strength;
         }

      bool operator==(const XMSS_Parameters& p) const
         {
         return m_oid == p.m_oid;
         }

   private:
      xmss_algorithm_t m_oid;
      XMSS_WOTS_Parameters::ots_algorithm_t m_wots_oid;
      std::string m_name;
      std::string m_hash_name;
      size_t m_element_size;
      size_t m_tree_height;
      size_t m_w;
      size_t m_len;
      size_t m_strength;
   };

}


namespace Botan {

class XMSS_Verification_Operation;

/**
 * An XMSS: Extended Hash-Based Signature public key.
 * The XMSS public key does not support the X509 standard. Instead the
 * raw format described in [1] is used.
 *
 *   [1] XMSS: Extended Hash-Based Signatures,
 *       draft-itrf-cfrg-xmss-hash-based-signatures-06
 *       Release: July 2016.
 *       https://datatracker.ietf.org/doc/
 *       draft-irtf-cfrg-xmss-hash-based-signatures/?include_text=1
 **/
class BOTAN_DLL XMSS_PublicKey : public virtual Public_Key
   {
   public:
      /**
       * Creates a new XMSS public key for the chosen XMSS signature method.
       * New public and prf seeds are generated using rng. The appropriate WOTS
       * signature method will be automatically set based on the chosen XMSS
       * signature method.
       *
       * @param xmss_oid Identifier for the selected XMSS signature method.
       * @param rng A random number generator to use for key generation.
       **/
      XMSS_PublicKey(XMSS_Parameters::xmss_algorithm_t xmss_oid,
                     RandomNumberGenerator& rng)
         : m_xmss_params(xmss_oid), m_wots_params(m_xmss_params.ots_oid()),
           m_root(m_xmss_params.element_size()),
           m_public_seed(rng.random_vec(m_xmss_params.element_size())) {}

      /**
       * Creates an XMSS public key from a byte sequence produced by
       * raw_private_key().
       **/
      XMSS_PublicKey(const std::vector<uint8_t>& raw_key);

      /**
       * Creates a new XMSS public key for a chosen XMSS signature method as
       * well as pre-computed root node and public_seed values.
       *
       * @param xmss_oid Identifier for the selected XMSS signature method.
       * @param root Root node value.
       * @param public_seed Public seed value.
       **/
      XMSS_PublicKey(XMSS_Parameters::xmss_algorithm_t xmss_oid,
                     const secure_vector<uint8_t>& root,
                     const secure_vector<uint8_t>& public_seed)
         : m_xmss_params(xmss_oid), m_wots_params(m_xmss_params.ots_oid()),
           m_root(root), m_public_seed(public_seed) {}

      /**
       * Creates a new XMSS public key for a chosen XMSS signature method as
       * well as pre-computed root node and public_seed values.
       *
       * @param xmss_oid Identifier for the selected XMSS signature method.
       * @param root Root node value.
       * @param public_seed Public seed value.
       **/
      XMSS_PublicKey(XMSS_Parameters::xmss_algorithm_t xmss_oid,
                     secure_vector<uint8_t>&& root,
                     secure_vector<uint8_t>&& public_seed)
         : m_xmss_params(xmss_oid), m_wots_params(m_xmss_params.ots_oid()),
           m_root(std::move(root)), m_public_seed(std::move(public_seed)) {}

      /**
       * Retrieves the chosen XMSS signature method.
       *
       * @return XMSS signature method identifier.
       **/
      XMSS_Parameters::xmss_algorithm_t xmss_oid() const
         {
         return m_xmss_params.oid();
         }

      /**
       * Sets the chosen XMSS signature method
       *
       * @return XMSS signature method identifier.
       **/
      void set_xmss_oid(XMSS_Parameters::xmss_algorithm_t xmss_oid)
         {
         m_xmss_params = XMSS_Parameters(xmss_oid);
         m_wots_params = XMSS_WOTS_Parameters(m_xmss_params.ots_oid());
         }

      /**
       * Retrieves the XMSS parameters determined by the chosen XMSS Signature
       * method.
       *
       * @return XMSS parameters.
       **/
      const XMSS_Parameters& xmss_parameters() const
         {
         return m_xmss_params;
         }

      /**
       * Retrieves the Winternitz One Time Signature (WOTS) method,
       * corrseponding to the chosen XMSS signature method.
       *
       * @return XMSS WOTS signature method identifier.
       **/
      XMSS_WOTS_Parameters::ots_algorithm_t wots_oid() const
         {
         return m_wots_params.oid();
         }

      /**
       * Retrieves the Winternitz One Time Signature (WOTS) parameters
       * corresponding to the chosen XMSS signature method.
       *
       * @return XMSS WOTS signature method parameters.
       **/
      const XMSS_WOTS_Parameters& wots_parameters() const
         {
         return m_wots_params;
         }

      secure_vector<uint8_t>& root()
         {
         return m_root;
         }

      void set_root(const secure_vector<uint8_t>& root)
         {
         m_root = root;
         }

      void set_root(secure_vector<uint8_t>&& root)
         {
         m_root = std::move(root);
         }

      const secure_vector<uint8_t>& root() const
         {
         return m_root;
         }

      virtual secure_vector<uint8_t>& public_seed()
         {
         return m_public_seed;
         }

      virtual void set_public_seed(const secure_vector<uint8_t>& public_seed)
         {
         m_public_seed = public_seed;
         }

      virtual void set_public_seed(secure_vector<uint8_t>&& public_seed)
         {
         m_public_seed = std::move(public_seed);
         }

      virtual const secure_vector<uint8_t>& public_seed() const
         {
         return m_public_seed;
         }

      std::string algo_name() const override
         {
         return "XMSS";
         }

      virtual AlgorithmIdentifier algorithm_identifier() const override
         {
         return AlgorithmIdentifier(get_oid(), AlgorithmIdentifier::USE_NULL_PARAM);
         }

      virtual bool check_key(RandomNumberGenerator&, bool) const override
         {
         return true;
         }

      virtual std::unique_ptr<PK_Ops::Verification>
         create_verification_op(const std::string&,
                                const std::string& provider) const override;

      virtual size_t estimated_strength() const override
         {
         return m_xmss_params.estimated_strength();
         }

      virtual size_t key_length() const override
         {
         return m_xmss_params.estimated_strength();
         }

      /**
       * Returns a raw byte sequence as defined in [1].
       * This method acts as an alias for raw_public_key().
       *
       * @return raw public key bits.
       **/
      virtual std::vector<uint8_t> public_key_bits() const override
         {
         return raw_public_key();
         }

      /**
       * Size in bytes of the serialized XMSS public key produced by
       * raw_public_key().
       *
       * @return size in bytes of serialized Public Key.
       **/
      virtual size_t size() const
         {
         return sizeof(uint32_t) + 2 * m_xmss_params.element_size();
         }

      /**
       * Generates a non standardized byte sequence representing the XMSS
       * public key, as defined in [1] (p. 23, "XMSS Public Key")
       *
       * @return 4-byte OID, followed by n-byte root node, followed by
       *         public seed.
       **/
      virtual std::vector<uint8_t> raw_public_key() const;

   protected:
      XMSS_Parameters m_xmss_params;
      XMSS_WOTS_Parameters m_wots_params;
      secure_vector<uint8_t> m_root;
      secure_vector<uint8_t> m_public_seed;

   private:
      XMSS_Parameters::xmss_algorithm_t deserialize_xmss_oid(
         const std::vector<uint8_t>& raw_key);
   };

}


namespace Botan {

/**
 * Generic XMSS Address type holding 256 Bits of data. Properties
 * of all three address formats L-Tree-Address, Hash-Tree-Address,
 * OTS-Hash-Address can be called depending on the type currently
 * assigned to the XMSS address using set_type().
 **/
class XMSS_Address
   {
   public:
      /**
       * Distinct types an XMSS_Address can represent. The available types
       * are specified in [1] - 2.5 Hash Function Address Scheme.
       **/
      enum class Type : uint8_t
         {
         None = 255,
         OTS_Hash_Address = 0,
         LTree_Address = 1,
         Hash_Tree_Address = 2
         };

      /**
       * The available modes for an XMSS Address:
       *   - Key_Mode: Used to generate the key.
       *   - Mask_Mode: Sets the n-byte bitmask (OTS-Hash-Address)
       *   - Mask_MSB_Mode: Used to generate the b most significant bytes of
       *     the 2n-byte bitmask (LTree Address and Hash Tree Address).
       *   - Mask_LSB_Mode: Used to generated the b least significant bytes
       *     of the 2n-byte bitmask. (LTree Address and Hash Tree Address).
       **/
      enum class Key_Mask : uint8_t
         {
         Key_Mode = 0,
         Mask_Mode = 1,
         Mask_MSB_Mode = 1,
         Mask_LSB_Mode = 2
         };

      /**
       * Layer Address for XMSS is constantly zero and can not be changed this
       * property is only of relevance to XMSS_MT.
       *
       * @return Layer address, which is constant 0 for XMSS.
       **/
      uint8_t get_layer_addr() const { return 0; }

      /**
       * Layer Address for XMSS is constantly zero and can not be changed this
       * property is only of relevance to XMSS_MT. Calling this method for
       * XMSS will result in an error.
       **/
      void set_layer_addr()
         {
         BOTAN_ASSERT(false, "Only available in XMSS_MT.");
         }

      /**
       * Tree Address for XMSS is constantly zero and can not be changed this
       * property is only of relevance to XMSS_MT.
       *
       * @return Tree address, which is constant 0 for XMSS.
       **/
      uint64_t get_tree_addr() const { return 0; }

      /**
       * Tree Address for XMSS is constantly zero and can not be changed this
       * property is only of relevance to XMSS_MT. Calling this method for
       * XMSS will result in an error.
       **/
      void set_tree_addr()
         {
         BOTAN_ASSERT(false, "Only available in XMSS_MT.");
         }

      /**
       * retrieves the logical type currently assigned to the XMSS Address
       * instance.
       *
       * @return Type of the address (OTS_Hash_Address, LTree_Address or
       *         Hash_Tree_Address)
       **/
      Type get_type() const
         {
         return static_cast<Type>(m_data[15]);
         }

      /**
       * Changes the logical type currently assigned to the XMSS Address
       * instance. Please note that changing the type will automatically
       * reset the 128 LSBs of the Address to zero. This affects the
       * key_mask_mode property as well as all properties identified by
       * XMSS_Address::Property.
       *
       * @param type Type that shall be assigned to the address
       *        (OTS_Hash_Address, LTree_Address or Hash_Tree_Address)
       **/
      void set_type(Type type)
         {
         m_data[15] = static_cast<uint8_t>(type);
         std::fill(m_data.begin() + 16, m_data.end(), 0);
         }

      /**
       * Retrieves the mode the address os currently set to. (See
       * XMSS_Address::Key_Mask for details.)
       *
       * @return currently active mode
       **/
      Key_Mask get_key_mask_mode() const
         {
         return Key_Mask(m_data[31]);
         }

      /**
       * Changes the mode the address currently used address mode.
       * (XMSS_Address::Key_Mask for details.)
       *
       * @param value Target mode.
       **/
      void set_key_mask_mode(Key_Mask value)
         {
         BOTAN_ASSERT(value != Key_Mask::Mask_LSB_Mode ||
                      get_type() != Type::OTS_Hash_Address,
                      "Invalid Key_Mask for current XMSS_Address::Type.");
         m_data[31] = static_cast<uint8_t>(value);
         }

      /**
       * Retrieve the index of the OTS key pair within the tree. A call to
       * this method is only valid, if the address type is set to
       * Type::OTS_Hash_Address.
       *
       * @return index of OTS key pair.
       **/
      uint32_t get_ots_address() const
         {
         BOTAN_ASSERT(get_type() == Type::OTS_Hash_Address,
                      "get_ots_address() requires XMSS_Address::Type::"
                      "OTS_Hash_Address.");
         return get_hi32(2);
         }

      /**
       * Sets the index of the OTS key pair within the tree. A call to this
       * method is only valid, if the address type is set to
       * Type::OTS_Hash_Address.
       *
       * @param value index of OTS key pair.
       **/
      void set_ots_address(uint32_t value)
         {
         BOTAN_ASSERT(get_type() == Type::OTS_Hash_Address,
                      "set_ots_address() requires XMSS_Address::Type::"
                      "OTS_Hash_Address.");
         set_hi32(2, value);
         }

      /**
       * Retrieves the index of the leaf computed with this LTree. A call to
       * this method is only valid, if the address type is set to
       * Type::LTree_Address.
       *
       * @return index of the leaf.
       **/
      uint32_t get_ltree_address() const
         {
         BOTAN_ASSERT(get_type() == Type::LTree_Address,
                      "set_ltree_address() requires XMSS_Address::Type::"
                      "LTree_Address.");
         return get_hi32(2);
         }

      /**
       * Sets the index of the leaf computed with this LTree. A call to this
       * method is only valid, if the address type is set to
       * Type::LTree_Address.
       *
       * @param value index of the leaf.
       **/
      void set_ltree_address(uint32_t value)
         {
         BOTAN_ASSERT(get_type() == Type::LTree_Address,
                      "set_ltree_address() requires XMSS_Address::Type::"
                      "LTree_Address.");
         set_hi32(2, value);
         }

      /**
       * Retrieve the chain address. A call to this method is only valid, if
       * the address type is set to Type::OTS_Hash_Address.
       *
       * @return chain address.
       **/
      uint32_t get_chain_address() const
         {
         BOTAN_ASSERT(get_type() == Type::OTS_Hash_Address,
                      "get_chain_address() requires XMSS_Address::Type::"
                      "OTS_Hash_Address.");
         return get_lo32(2);
         }

      /**
       * Set the chain address. A call to this method is only valid, if
       * the address type is set to Type::OTS_Hash_Address.
       *
       * @return chain address.
       **/
      void set_chain_address(uint32_t value)
         {
         BOTAN_ASSERT(get_type() == Type::OTS_Hash_Address,
                      "set_chain_address() requires XMSS_Address::Type::"
                      "OTS_Hash_Address.");
         set_lo32(2, value);
         }

      /**
       * Retrieves the height of the tree node to be computed within the
       * tree. A call to this method is only valid, if the address type is
       * set to Type::LTree_Address or Type::Hash_Tree_Address.
       *
       * @return height of the tree node.
       **/
      uint32_t get_tree_height() const
         {
         BOTAN_ASSERT(get_type() == Type::LTree_Address ||
                      get_type() == Type::Hash_Tree_Address,
                      "get_tree_height() requires XMSS_Address::Type::"
                      "LTree_Address or XMSS_Address::Type::Hash_Tree_Address.");
         return get_lo32(2);
         }

      /**
       * Sets the height of the tree node to be computed within the
       * tree. A call to this method is only valid, if the address type is
       * set to Type::LTree_Address or Type::Hash_Tree_Address.
       *
       * @param value height of the tree node.
       **/
      void set_tree_height(uint32_t value)
         {
         BOTAN_ASSERT(get_type() == Type::LTree_Address ||
                      get_type() == Type::Hash_Tree_Address,
                      "set_tree_height() requires XMSS_Address::Type::"
                      "LTree_Address or XMSS_Address::Type::Hash_Tree_Address.");
         set_lo32(2, value);
         }

      /**
       * Retrieves the address of the hash function call within the chain.
       * A call to this method is only valid, if the address type is
       * set to Type::OTS_Hash_Address.
       *
       * @return address of the hash function call within chain.
       **/
      uint32_t get_hash_address() const
         {
         BOTAN_ASSERT(get_type() == Type::OTS_Hash_Address,
                      "get_hash_address() requires XMSS_Address::Type::"
                      "OTS_Hash_Address.");
         return get_hi32(3);
         }

      /**
       * Sets the address of the hash function call within the chain.
       * A call to this method is only valid, if the address type is
       * set to Type::OTS_Hash_Address.
       *
       * @param value address of the hash function call within chain.
       **/
      void set_hash_address(uint32_t value)
         {
         BOTAN_ASSERT(get_type() == Type::OTS_Hash_Address,
                      "set_hash_address() requires XMSS_Address::Type::"
                      "OTS_Hash_Address.");
         set_hi32(3, value);
         }

      /**
       * Retrieves the index of the tree node at current tree height in the
       * tree. A call to this method is only valid, if the address type is
       * set to Type::LTree_Address or Type::Hash_Tree_Address.
       *
       * @return index of the tree node at current height.
       **/
      uint32_t get_tree_index() const
         {
         BOTAN_ASSERT(get_type() == Type::LTree_Address ||
                      get_type() == Type::Hash_Tree_Address,
                      "get_tree_index() requires XMSS_Address::Type::"
                      "LTree_Address or XMSS_Address::Type::Hash_Tree_Address.");
         return get_hi32(3);
         }

      /**
       * Sets the index of the tree node at current tree height in the
       * tree. A call to this method is only valid, if the address type is
       * set to Type::LTree_Address or Type::Hash_Tree_Address.
       *
       * @param value index of the tree node at current height.
       **/
      void set_tree_index(uint32_t value)
         {
         BOTAN_ASSERT(get_type() == Type::LTree_Address ||
                      get_type() == Type::Hash_Tree_Address,
                      "set_tree_index() requires XMSS_Address::Type::"
                      "LTree_Address or XMSS_Address::Type::Hash_Tree_Address.");
         set_hi32(3, value);
         }

      const secure_vector<uint8_t>& bytes() const
         {
         return m_data;
         }

      secure_vector<uint8_t>& bytes()
         {
         return m_data;
         }

      /**
       * @return the size of an XMSS_Address
       **/
      size_t size() const
         {
         return m_data.size();
         }

      XMSS_Address()
         : m_data(m_address_size)
         {
         set_type(Type::None);
         }

      XMSS_Address(Type type)
         : m_data(m_address_size)
         {
         set_type(type);
         }

      XMSS_Address(const secure_vector<uint8_t>& data) : m_data(data)
         {
         BOTAN_ASSERT(m_data.size() == m_address_size,
                      "XMSS_Address must be of 256 bits size.");
         }

      XMSS_Address(secure_vector<uint8_t>&& data) : m_data(std::move(data))
         {
         BOTAN_ASSERT(m_data.size() == m_address_size,
                      "XMSS_Address must be of 256 bits size.");
         }

   protected:
      secure_vector<uint8_t> m_data;

   private:
      static const size_t m_address_size = 32;

      inline uint32_t get_hi32(size_t offset) const
         {
         return ((0x000000FF & m_data[8 * offset + 3])       |
                 (0x000000FF & m_data[8 * offset + 2]) <<  8 |
                 (0x000000FF & m_data[8 * offset + 1]) << 16 |
                 (0x000000FF & m_data[8 * offset    ]) << 24);
         }

      inline void set_hi32(size_t offset, uint32_t value)
         {
         m_data[offset * 8    ] = ((value >> 24) & 0xFF);
         m_data[offset * 8 + 1] = ((value >> 16) & 0xFF);
         m_data[offset * 8 + 2] = ((value >>  8) & 0xFF);
         m_data[offset * 8 + 3] = ((value      ) & 0xFF);
         }

      inline uint32_t get_lo32(size_t offset) const
         {
         return ((0x000000FF & m_data[8 * offset + 7])       |
                 (0x000000FF & m_data[8 * offset + 6]) <<  8 |
                 (0x000000FF & m_data[8 * offset + 5]) << 16 |
                 (0x000000FF & m_data[8 * offset + 4]) << 24);
         }

      inline void set_lo32(size_t offset, uint32_t value)
         {
         m_data[offset * 8 + 4] = ((value >> 24) & 0xFF);
         m_data[offset * 8 + 5] = ((value >> 16) & 0xFF);
         m_data[offset * 8 + 6] = ((value >>  8) & 0xFF);
         m_data[offset * 8 + 7] = ((value      ) & 0xFF);
         }
   };

}


namespace Botan {

/**
 * A collection of pseudorandom hash functions required for XMSS and WOTS
 * computations.
 **/
class XMSS_Hash
   {
   public:
      XMSS_Hash(const std::string& h_func_name);
      XMSS_Hash(const XMSS_Hash& hash);

      /**
       * Pseudoranom function creating a hash out of a key and data using
       * a cryptographic hash function.
       *
       * @param[out] result The hash calculated using key and data.
       * @param[in] key An n-byte key value.
       * @param[in] data A 32-byte XMSS_Address data value
       **/
      inline void prf(secure_vector<uint8_t>& result,
                      const secure_vector<uint8_t>& key,
                      const secure_vector<uint8_t>& data)
         {
         m_hash->update(m_zero_padding);
         m_hash->update(m_id_prf);
         m_hash->update(key);
         m_hash->update(data);
         m_hash->final(result);
         }

      /**
       * Pseudoranom function creating a hash out of a key and data using
       * a cryptographic hash function.
       *
       * @param[in] key An n-byte key value.
       * @param[in] data A 32-byte XMSS_Address data value
       * @return result The hash calculated using key and data.
       **/
      inline secure_vector<uint8_t> prf(const secure_vector<uint8_t>& key,
                                     const secure_vector<uint8_t>& data)
         {
         m_hash->update(m_zero_padding);
         m_hash->update(m_id_prf);
         m_hash->update(key);
         m_hash->update(data);
         return m_hash->final();
         }

      /**
       * F is a keyed cryptographic hash function used by the WOTS+ algorithm.
       *
       * @param[out] result The hash calculated using key and data.
       * @param[in] key key of length n bytes.
       * @param[in] data string of arbitrary length.
       **/
      void f(secure_vector<uint8_t>& result,
             const secure_vector<uint8_t>& key,
             const secure_vector<uint8_t>& data)
         {
         m_hash->update(m_zero_padding);
         m_hash->update(m_id_f);
         m_hash->update(key);
         m_hash->update(data);
         m_hash->final(result);
         }

      /**
       * Cryptographic hash function h accepting n byte keys and 2n byte
       * strings of data.
       *
       * @param[out] result The hash calculated using key and data.
       * @param[in] key key of length n bytes.
       * @param[in] data string of 2n bytes length.
       **/
      void h(secure_vector<uint8_t>& result,
             const secure_vector<uint8_t>& key,
             const secure_vector<uint8_t>& data);

      /**
       * Cryptographic hash function h accepting 3n byte keys and data
       * strings of arbitrary length.
       *
       * @param randomness n-byte value.
       * @param root n-byte root node.
       * @param index_bytes Index value padded with leading zeros.
       * @param data string of arbitrary length.
       *
       * @return hash value of n-bytes length.
       **/
      secure_vector<uint8_t> h_msg(const secure_vector<uint8_t>& randomness,
                                const secure_vector<uint8_t>& root,
                                const secure_vector<uint8_t>& index_bytes,
                                const secure_vector<uint8_t>& data);

      /**
       * Initializes buffered h_msg computation with prefix data.
       *
       * @param randomness random n-byte value.
       * @param root n-byte root node.
       * @param index_bytes Index value padded with leading zeros.
       **/
      void h_msg_init(const secure_vector<uint8_t>& randomness,
                      const secure_vector<uint8_t>& root,
                      const secure_vector<uint8_t>& index_bytes);

      /**
       * Adds a message block to buffered h_msg computation.
       *
       * @param data A message block
       **/
      void h_msg_update(const secure_vector<uint8_t>& data);

      /**
       * Adds a message block to buffered h_msg computation.
       *
       * @param data A message block
       * @param size Length of the message block in bytes.
       **/
      void h_msg_update(const uint8_t data[], size_t size);

      /**
       * Finalizes buffered h_msg computation and retrieves the result.
       *
       * @return Hash calculated using the prefix set by h_msg_init() and
       *         message blocks provided through calls to h_msg_update().
       **/
      secure_vector<uint8_t> h_msg_final();

      size_t output_length() const { return m_output_length; };

   private:
      static const uint8_t m_id_f = 0x00;
      static const uint8_t m_id_h = 0x01;
      static const uint8_t m_id_hmsg = 0x02;
      static const uint8_t m_id_prf = 0x03;

      const std::string m_hash_func_name;
      std::unique_ptr<HashFunction> m_hash;
      std::unique_ptr<HashFunction> m_msg_hash;
      size_t m_output_length;

      //32 byte id prefixes prepended to the hash input.
      std::vector<uint8_t> m_zero_padding;
   };

}


namespace Botan {

typedef std::vector<secure_vector<uint8_t>> wots_keysig_t;

/**
 * Operations shared by XMSS signature generation and verification operations.
 **/
class XMSS_Common_Ops
   {
   public:
      XMSS_Common_Ops(XMSS_Parameters::xmss_algorithm_t oid)
         : m_xmss_params(oid), m_hash(m_xmss_params.hash_function_name()) {};

   protected:
      /**
        * Algorithm 7: "RAND_HASH"
        *
        * Generates a randomized hash.
        *
        * @param[out] result The resulting randomized hash.
        * @param[in] left Left half of the hash function input.
        * @param[in] right Right half of the hash function input.
        * @param[in] adrs Adress of the hash function call.
        * @param[in] seed The seed for G.
        **/
      void randomize_tree_hash(
         secure_vector<uint8_t>& result,
         const secure_vector<uint8_t>& left,
         const secure_vector<uint8_t>& right,
         XMSS_Address& adrs,
         const secure_vector<uint8_t>& seed);

      /**
       * Algorithm 8: "ltree"
       * Create an L-tree used to compute the leaves of the binary hash tree.
       * Takes a WOTS+ public key and compresses it to a single n-byte value.
       *
       * @param[out] result Public key compressed to a single n-byte value
       *             pk[0].
       * @param[in] pk Winternitz One Time Signatures+ public key.
       * @param[in] adrs Address encoding the address of the L-Tree
       * @param[in] seed The seed generated during the public key generation.
       **/
      void create_l_tree(
         secure_vector<uint8_t>& result,
         wots_keysig_t pk,
         XMSS_Address& adrs,
         const secure_vector<uint8_t>& seed);

   protected:
      XMSS_Parameters m_xmss_params;
      XMSS_Hash m_hash;

   };

}


namespace Botan {

typedef std::vector<secure_vector<uint8_t>> wots_keysig_t;

/**
 * A Winternitz One Time Signature public key for use with Extended Hash-Based
 * Signatures.
 **/
class BOTAN_DLL XMSS_WOTS_PublicKey : virtual public Public_Key
   {
   public:
      class TreeSignature
         {
         public:
            TreeSignature()
               : m_ots_sig(), m_auth_path() {}

            TreeSignature(const wots_keysig_t& ots_sig,
                          const wots_keysig_t& auth_path)
               : m_ots_sig(ots_sig), m_auth_path(auth_path)
               {}

            TreeSignature(wots_keysig_t&& ots_sig,
                          wots_keysig_t&& auth_path)
               : m_ots_sig(std::move(ots_sig)),
                 m_auth_path(std::move(auth_path))
               {}

            const wots_keysig_t& ots_signature() const
               {
               return m_ots_sig;
               }

            wots_keysig_t& ots_signature()
               {
               return m_ots_sig;
               }

            const wots_keysig_t& authentication_path() const
               {
               return m_auth_path;
               }

            wots_keysig_t& authentication_path()
               {
               return m_auth_path;
               }

         private:
            wots_keysig_t m_ots_sig;
            wots_keysig_t m_auth_path;
         };

      /**
       * Creates a XMSS_WOTS_PublicKey for the signature method identified by
       * oid. The public seed for this key will be initialized with a
       * uniformly random n-byte value, where "n" is the element size of the
       * selected signature method.
       *
       * @param oid Identifier for the selected signature method.
       **/
      XMSS_WOTS_PublicKey(XMSS_WOTS_Parameters::ots_algorithm_t oid)
         : m_wots_params(oid),
           m_hash(m_wots_params.hash_function_name()) {}

      /**
       * Creates a XMSS_WOTS_PublicKey for the signature method identified by
       * oid. The public seed for this key will be initialized with a
       * uniformly random n-byte value, where "n" is the element size of the
       * selected signature method.
       *
       * @param oid Identifier for the selected signature method.
       * @param rng A random number generate used to generate the public seed.
       **/
      XMSS_WOTS_PublicKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                          RandomNumberGenerator& rng)
         : m_wots_params(oid),
           m_hash(m_wots_params.hash_function_name()),
           m_public_seed(rng.random_vec(m_wots_params.element_size())) {}

      /**
       * Creates a XMSS_WOTS_PrivateKey for the signature method identified by
       * oid, with a precomputed public seed.
       *
       * @param oid Identifier for the selected signature method.
       * @param public_seed A precomputed public seed of n-bytes length.
       **/
      XMSS_WOTS_PublicKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                          secure_vector<uint8_t> public_seed)
         : m_wots_params(oid),
           m_hash(m_wots_params.hash_function_name()),
           m_public_seed(public_seed) {}

      /**
       * Creates a XMSS_WOTS_PublicKey for the signature method identified by
       * oid. The public seed will be initialized with a precomputed seed and
       * and precomputed key data which should be derived from a
       * XMSS_WOTS_PrivateKey.
       *
       * @param oid Ident:s/ifier for the selected signature methods.
       * @param public_seed A precomputed public seed of n-bytes length.
       * @param key Precomputed raw key data of the XMSS_WOTS_PublicKey.
       **/
      XMSS_WOTS_PublicKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                          secure_vector<uint8_t>&& public_seed,
                          wots_keysig_t&& key)
         : m_wots_params(oid),
           m_hash(m_wots_params.hash_function_name()),
           m_key(std::move(key)),
           m_public_seed(std::move(public_seed))
         {}

      /**
       * Creates a XMSS_WOTS_PublicKey for the signature method identified by
       * oid. The public seed will be initialized with a precomputed seed and
       * and precomputed key data which should be derived from a
       * XMSS_WOTS_PrivateKey.
       *
       * @param oid Identifier for the selected signature methods.
       * @param public_seed A precomputed public seed of n-bytes length.
       * @param key Precomputed raw key data of the XMSS_WOTS_PublicKey.
       **/
      XMSS_WOTS_PublicKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                          const secure_vector<uint8_t>& public_seed,
                          const wots_keysig_t& key)
         : m_wots_params(oid),
           m_hash(m_wots_params.hash_function_name()),
           m_key(key),
           m_public_seed(public_seed)
         {}

      /**
       * Creates a XMSS_WOTS_PublicKey form a message and signature using
       * Algorithm 6 WOTS_pkFromSig defined in the XMSS standard. This
       * overload is used to verify a message using a public key.
       *
       * @param oid WOTSP algorithm identifier.
       * @param msg A message.
       * @param sig A WOTS signature for msg.
       * @param adrs An XMSS_Address.
       * @param public_seed The public public_seed.
       **/
      XMSS_WOTS_PublicKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                          const secure_vector<uint8_t>& msg,
                          const wots_keysig_t& sig,
                          XMSS_Address& adrs,
                          const secure_vector<uint8_t>& public_seed)
         : m_wots_params(oid),
           m_hash(m_wots_params.hash_function_name()),
           m_key(pub_key_from_signature(msg,
                                        sig,
                                        adrs,
                                        public_seed)),
           m_public_seed(public_seed)
         {}

      /**
       * Retrieves the i-th element out of the length len chain of
       * n-byte elements contained in the public key.
       *
       * @param i index of the element.
       * @returns n-byte element addressed by i.
       **/
      const secure_vector<uint8_t>& operator[](size_t i) const { return m_key[i]; }
      secure_vector<uint8_t>& operator[](size_t i) { return m_key[i]; }

      /**
       * Convert the key into the raw key data. The key becomes a length
       * len vector of n-byte elements.
       **/
      operator const wots_keysig_t& () const { return m_key; };

      /**
       * Convert the key into the raw key data. The key becomes a length
       * len vector of n-byte elements.
       **/
      operator wots_keysig_t& () { return m_key; }

      const secure_vector<uint8_t>& public_seed() const { return m_public_seed; }

      secure_vector<uint8_t>& public_seed() { return m_public_seed; }

      void set_public_seed(const secure_vector<uint8_t>& public_seed)
         {
         m_public_seed = public_seed;
         }

      void set_public_seed(secure_vector<uint8_t>&& public_seed)
         {
         m_public_seed = std::move(public_seed);
         }

      const wots_keysig_t& key_data() const { return m_key; }

      wots_keysig_t& key_data() { return m_key; }

      void set_key_data(const wots_keysig_t& key_data)
         {
         m_key = key_data;
         }

      void set_key_data(wots_keysig_t&& key_data)
         {
         m_key = std::move(key_data);
         }

      const XMSS_WOTS_Parameters& wots_parameters() const
         {
         return m_wots_params;
         }

      virtual std::string algo_name() const override
         {
         return m_wots_params.name();
         }

      virtual AlgorithmIdentifier algorithm_identifier() const override
         {
         throw Not_Implemented("No AlgorithmIdentifier available for XMSS-WOTS.");
         }

      virtual bool check_key(RandomNumberGenerator&, bool) const override
         {
         return true;
         }

      virtual std::unique_ptr<PK_Ops::Verification>
         create_verification_op(const std::string&,
                                const std::string& provider) const override;

      virtual size_t estimated_strength() const override
         {
         return m_wots_params.estimated_strength();
         }

      virtual size_t key_length() const override
         {
         return m_wots_params.estimated_strength();
         }

      virtual std::vector<uint8_t> public_key_bits() const override
         {
         throw Not_Implemented("No key format defined for XMSS-WOTS");
         }

      bool operator==(const XMSS_WOTS_PublicKey& key)
         {
         return m_key == key.m_key;
         }

      bool operator!=(const XMSS_WOTS_PublicKey& key)
         {
         return !(*this == key);
         }

   protected:
      /**
       * Algorithm 2: Chaining Function.
       *
       * Takes an n-byte input string and transforms it into a the function
       * result iterating the cryptographic hash function "F" steps times on
       * the input x using the outputs of the PRNG "G".
       *
       *
       * @param[out] x An n-byte input string, that will be transformed into
       *               the chaining function result.
       * @param start_idx The start index.
       * @param steps A number of steps.
       * @param adrs An OTS Hash Address.
       * @param public_seed A public seed.
       *
       **/
      void chain(secure_vector<uint8_t>& x,
                 size_t start_idx,
                 size_t steps,
                 XMSS_Address& adrs,
                 const secure_vector<uint8_t>& public_seed);

      XMSS_WOTS_Parameters m_wots_params;
      XMSS_Hash m_hash;

      wots_keysig_t m_key;
      secure_vector<uint8_t> m_public_seed;

   private:
      /**
       * Algorithm 6: "WOTS_pkFromSig"
       * Computes a Winternitz One Time Signature+ public key from a message and
       * its signature.
       *
       * @param msg A message.
       * @param sig The signature for msg.
       * @param adrs An address.
       * @param public_seed A public_seed.
       *
       * @return Temporary WOTS+ public key.
       **/
      wots_keysig_t pub_key_from_signature(
         const secure_vector<uint8_t>& msg,
         const wots_keysig_t& sig,
         XMSS_Address& adrs,
         const secure_vector<uint8_t>& public_seed);
   };

}


namespace Botan {

/** A Winternitz One Time Signature private key for use with Extended Hash-Based
 * Signatures.
 **/
class BOTAN_DLL XMSS_WOTS_PrivateKey : public virtual XMSS_WOTS_PublicKey,
                                       public virtual Private_Key
   {
   public:
      /**
       * Creates a WOTS private key for the chosen XMSS WOTS signature method.
       * Members need to be initialized manually.
       *
       * @param oid Identifier for the selected signature method.
       **/
      XMSS_WOTS_PrivateKey(XMSS_WOTS_Parameters::ots_algorithm_t oid)
         : XMSS_WOTS_PublicKey(oid)
         {}

      /**
       * Creates a WOTS private key for the chosen XMSS WOTS signature method.
       *
       * @param oid Identifier for the selected signature method.
       * @param rng A random number generator to use for key generation.
       **/
      XMSS_WOTS_PrivateKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                           RandomNumberGenerator& rng)
         : XMSS_WOTS_PublicKey(oid, rng),
           m_private_seed(rng.random_vec(m_wots_params.element_size()))
         {
         set_key_data(generate(m_private_seed));
         }

      /**
       * Constructs a WOTS private key. Chains will be generated on demand
       * applying a hash function to a unique value generated from a secret
       * seed and a counter. The secret seed of length n, will be
       * automatically generated using AutoSeeded_RNG(). "n" equals
       * the element size of the chosen WOTS security parameter set.
       *
       * @param oid Identifier for the selected signature method.
       * @param public_seed A public seed used for the pseudo random generation
       *        of public keys derived from this private key.
       * @param rng A random number generator to use for key generation.
       **/
      XMSS_WOTS_PrivateKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                           const secure_vector<uint8_t>& public_seed,
                           RandomNumberGenerator &rng)
         : XMSS_WOTS_PublicKey(oid, public_seed),
           m_private_seed(rng.random_vec(m_wots_params.element_size()))
         {
         set_key_data(generate(m_private_seed));
         }

      /**
       * Constructs a WOTS private key. Chains will be generated on demand
       * applying a hash function to a unique value generated from a secret
       * seed and a counter. The secret seed of length n, will be
       * automatically generated using AutoSeeded_RNG(). "n" equals
       * the element size of the chosen WOTS security parameter set.
       *
       * @param oid Identifier for the selected signature method.
       * @param public_seed A public seed used for the pseudo random generation
       *        of public keys derived from this private key.
       **/
      XMSS_WOTS_PrivateKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                           const secure_vector<uint8_t>& public_seed)
         : XMSS_WOTS_PublicKey(oid, public_seed)
         {}

      /**
       * Constructs a WOTS private key. Chains will be generated on demand
       * applying a hash function to a unique value generated from the
       * secret seed and a counter.
       *
       * @param oid Identifier for the selected signature method.
       * @param public_seed A public seed used for the pseudo random generation
       *        of public keys derived from this private key.
       * @param private_seed A secret uniformly random n-byte value.
       **/
      XMSS_WOTS_PrivateKey(XMSS_WOTS_Parameters::ots_algorithm_t oid,
                           const secure_vector<uint8_t>& public_seed,
                           const secure_vector<uint8_t>& private_seed)
         : XMSS_WOTS_PublicKey(oid, public_seed),
           m_private_seed(private_seed)
         {
         set_key_data(generate(private_seed));
         }

      /**
       * Retrieves the i-th WOTS private key using pseudo random key
       * (re-)generation.
       *
       * @param i Index of the key to retrieve.
       *
       * @return WOTS secret key.
       **/
      wots_keysig_t operator[](size_t i)
         {
         secure_vector<uint8_t> idx_bytes;
         XMSS_Tools::concat(idx_bytes, i, m_wots_params.element_size());
         m_hash.h(idx_bytes, m_private_seed, idx_bytes);
         return generate(idx_bytes);
         }

      /**
       * Retrieves the i-th WOTS private key using pseudo random key
       * (re-)generation.
       *
       * @param adrs The address of the key to retrieve.
       *
       * @return WOTS secret key.
       **/
      wots_keysig_t operator[](const XMSS_Address& adrs)
         {
         secure_vector<uint8_t> result;
         m_hash.prf(result, m_private_seed, adrs.bytes());
         return generate(result);
         }

      wots_keysig_t generate_private_key(const secure_vector<uint8_t>& priv_seed);

      /**
       * Algorithm 4: "WOTS_genPK"
       * Generates a Winternitz One Time Signature+ (WOTS+) Public Key from a
       * given private key.
       *
       * @param adrs Hash function address encoding the address of the WOTS+
       *             key pair within a greater structure.
       *
       * @return A XMSS_WOTS_PublicKey.
       **/
      XMSS_WOTS_PublicKey generate_public_key(XMSS_Address& adrs);

      /**
       * Algorithm 4: "WOTS_genPK"
       * Initializes a Winternitz One Time Signature+ (WOTS+) Public Key's
       * key_data() member, with data derived from in_key_data using the
       * WOTS chaining function.
       *
       * @param[out] pub_key Public key to initialize key_data() member on.
       * @param in_key_data Input key material from private key used for
       *        public key generation.
       * @param adrs Hash function address encoding the address of
       *        the WOTS+ key pair within a greater structure.
       **/
      void generate_public_key(XMSS_WOTS_PublicKey& pub_key,
                               wots_keysig_t&& in_key_data,
                               XMSS_Address& adrs);

      /**
       * Algorithm 5: "WOTS_sign"
       * Generates a signature from a private key and a message.
       *
       * @param msg A message to sign.
       * @param adrs An OTS hash address identifying the WOTS+ key pair
       *        used for signing.
       *
       * @return signature for msg.
       **/
      wots_keysig_t sign(const secure_vector<uint8_t>& msg,
                         XMSS_Address& adrs);

      /**
       * Retrieves the secret seed used to generate WOTS+ chains. The seed
       * should be a uniformly random n-byte value.
       *
       * @return secret seed.
       **/
      const secure_vector<uint8_t>& private_seed() const
         {
         return m_private_seed;
         }

      /**
       * Sets the secret seed used to generate WOTS+ chains. The seed
       * should be a uniformly random n-byte value.
       *
       * @param private_seed Uniformly random n-byte value.
       **/
      void set_private_seed(const secure_vector<uint8_t>& private_seed)
         {
         m_private_seed = private_seed;
         }

      /**
       * Sets the secret seed used to generate WOTS+ chains. The seed
       * should be a uniformly random n-byte value.
       *
       * @param private_seed Uniformly random n-byte value.
       **/
      void set_private_seed(secure_vector<uint8_t>&& private_seed)
         {
         m_private_seed = std::move(private_seed);
         }

      virtual AlgorithmIdentifier
      pkcs8_algorithm_identifier() const override
         {
         throw Not_Implemented("No AlgorithmIdentifier available for XMSS-WOTS.");
         }

      virtual std::unique_ptr<PK_Ops::Signature>
         create_signature_op(RandomNumberGenerator&,
                             const std::string&,
                             const std::string& provider) const override;

      virtual secure_vector<uint8_t> private_key_bits() const override
         {
         throw Not_Implemented("No PKCS8 key format defined for XMSS-WOTS.");
         }

   private:
      /**
       * Algorithm 3: "Generating a WOTS+ Private Key".
       * Generates a private key.
       *
       * @param private_seed Uniformly random n-byte value.
       *
       * @returns a vector of length key_size() of vectors of n bytes length
       *          containing uniformly random data.
       **/
      wots_keysig_t generate(const secure_vector<uint8_t>& private_seed);

      secure_vector<uint8_t> m_private_seed;
   };

}

#include <stdint.h>

namespace Botan {

/**
 * A registry for XMSS private keys, keeps track of the leaf index for
 * independend copies of the same key.
 **/
class XMSS_Index_Registry
   {
   public:
      XMSS_Index_Registry(const XMSS_Index_Registry&) = delete;
      XMSS_Index_Registry& operator=(const XMSS_Index_Registry&) = delete;

      /**
       * Retrieves a handle to the process-wide unique XMSS index registry.
       *
       * @return Reference to unique XMSS index registry.
       **/
      static XMSS_Index_Registry& get_instance()
         {
         static XMSS_Index_Registry self;
         return self;
         }

      /**
       * Retrieves the last unused leaf index for the private key identified
       * by private_seed and prf. The leaf index will be updated properly
       * across independent copies of private_key.
       *
       * @param private_seed Part of the unique identifier for an
       *                     XMSS_PrivateKey.
       * @param prf Part of the unique identifier for an XMSS_PrivateKey.
       *
       * @return last unused leaf index for private_key.
       **/
      std::shared_ptr<Atomic<size_t>>
                                   get(const secure_vector<uint8_t>& private_seed,
                                       const secure_vector<uint8_t>& prf);

   private:
      XMSS_Index_Registry()
         : m_key_ids(), m_leaf_indices(), m_mutex() {}

      static const std::string m_index_hash_function;

      /**
       * Creates a unique 64-bit id for an XMSS_Private key, by interpreting
       * the first 64-bit of HASH(PRIVATE_SEED || PRF) as 64 bit integer
       * value.
       *
       * @return unique integral identifier for an XMSS private key.
       **/
      uint64_t make_key_id(const secure_vector<uint8_t>& private_seed,
                           const secure_vector<uint8_t>& prf) const;

      /**
       * Retrieves the index position of a key within the registry or
       * max(size_t) if key has not been found.
       *
       * @param unique id of the XMSS private key (see make_key_id()).
       *
       * @return index position of key or max(size_t) if key not found.
       **/
      size_t get(uint64_t id) const;

      /**
       * If XMSS_PrivateKey identified by id is already registered, the
       * position of the according registry entry is returned. If last_unused
       * is bigger than the last unused index stored for the key identified by
       * id the unused leaf index for this key is set to last_unused. If no key
       * matching id is registed yet, an entry of id is added, with the last
       * unused leaf index initialized to the value of last_unused.
       *
       * @last_unused Initial value for the last unused leaf index of the
       *              registered key.
       *
       * @return positon of leaf index registry entry for key identified
       *         by id.
       **/
      size_t add(uint64_t id, size_t last_unused = 0);

      std::vector<uint64_t> m_key_ids;
      std::vector<std::shared_ptr<Atomic<size_t>>> m_leaf_indices;
      mutex_type m_mutex;
   };

}


namespace Botan {

/**
 * An XMSS: Extended Hash-Based Signature private key.
 * The XMSS private key does not support the X509 and PKCS7 standard. Instead
 * the raw format described in [1] is used.
 *
 *   [1] XMSS: Extended Hash-Based Signatures,
 *       draft-itrf-cfrg-xmss-hash-based-signatures-06
 *       Release: July 2016.
 *       https://datatracker.ietf.org/doc/
 *       draft-irtf-cfrg-xmss-hash-based-signatures/?include_text=1
 **/
class BOTAN_DLL XMSS_PrivateKey : public virtual XMSS_PublicKey,
                                  public XMSS_Common_Ops,
                                  public virtual Private_Key
   {
   public:
      /**
      * Creates a new XMSS private key for the chosen XMSS signature method.
      * New seeds for public/private key and pseudo random function input are
      * generated using the provided RNG. The appropriate WOTS signature method
      * will be automatically set based on the chosen XMSS signature method.
      *
      * @param xmss_algo_id Identifier for the selected XMSS signature method.
      * @param rng A random number generator to use for key generation.
      **/
      XMSS_PrivateKey(XMSS_Parameters::xmss_algorithm_t xmss_algo_id,
                      RandomNumberGenerator& rng);

      /**
       * Creates an XMSS_PrivateKey from a byte sequence produced by
       * raw_private_key().
       *
       * @param raw_key An XMSS private key serialized using raw_private_key().
       **/
      XMSS_PrivateKey(const secure_vector<uint8_t>& raw_key);

      /**
       * Creates a new XMSS private key for the chosen XMSS signature method
       * using precomputed seeds for public/private keys and pseudo random
       * function input. The appropriate WOTS signature method will be
       * automatically set, based on the chosen XMSS signature method.
       *
       * @param xmss_algo_id Identifier for the selected XMSS signature method.
       * @param idx_leaf Index of the next unused leaf.
       * @param wots_priv_seed A seed to generate a Winternitz-One-Time-
       *                      Signature private key from.
       * @param prf a secret n-byte key sourced from a secure source
       *        of uniformly random data.
       * @param root Root node of the binary hash tree.
       * @param public_seed The public seed.
       **/
      XMSS_PrivateKey(XMSS_Parameters::xmss_algorithm_t xmss_algo_id,
                      size_t idx_leaf,
                      const secure_vector<uint8_t>& wots_priv_seed,
                      const secure_vector<uint8_t>& prf,
                      const secure_vector<uint8_t>& root,
                      const secure_vector<uint8_t>& public_seed)
         : XMSS_PublicKey(xmss_algo_id, root, public_seed),
           XMSS_Common_Ops(xmss_algo_id),
           m_wots_priv_key(XMSS_PublicKey::m_xmss_params.ots_oid(),
                           public_seed,
                           wots_priv_seed),
           m_prf(prf),
           m_index_reg(XMSS_Index_Registry::get_instance())
         {
         set_unused_leaf_index(idx_leaf);
         }

      /**
       * Retrieves the last unused leaf index of the private key. Reusing a leaf
       * by utilizing leaf indices lower than the last unused leaf index will
       * compromise security.
       *
       * @return Index of the last unused leaf.
       **/
      size_t unused_leaf_index() const
         {
         return *recover_global_leaf_index();
         }

      /**
       * Sets the last unused leaf index of the private key. The leaf index
       * will be updated automatically during every signing operation, and
       * should not be set manually.
       *
       * @param idx Index of the last unused leaf.
       **/
      void set_unused_leaf_index(size_t idx)
         {
         if(idx >= (1ull << (XMSS_PublicKey::m_xmss_params.tree_height() - 1)))
            {
            throw Integrity_Failure("XMSS private key leaf index out of "
                                    "bounds.");
            }
         else
            {
            std::atomic<size_t>& index =
               static_cast<std::atomic<size_t>&>(*recover_global_leaf_index());
            size_t current = 0;

            do
               {
               current = index.load();
               if(current > idx)
                  return;
               }
            while(!index.compare_exchange_strong(current, idx));
            }
         }

      size_t reserve_unused_leaf_index()
         {
         size_t idx = (static_cast<std::atomic<size_t>&>(
             *recover_global_leaf_index())).fetch_add(1);
         if(idx >= (1ull << (XMSS_PublicKey::m_xmss_params.tree_height() - 1)))
           {
           throw Integrity_Failure("XMSS private key, one time signatures "
                                   "exhausted.");
           }
         return idx;
         }

      /**
       * Winternitz One Time Signature Scheme key utilized for signing
       * operations.
       *
       * @return WOTS+ private key.
       **/
      const XMSS_WOTS_PrivateKey& wots_private_key() const
         {
         return m_wots_priv_key;
         }

      /**
       * Winternitz One Time Signature Scheme key utilized for signing
       * operations.
       *
       * @return WOTS+ private key.
       **/
      XMSS_WOTS_PrivateKey& wots_private_key()
         {
         return m_wots_priv_key;
         }

      const secure_vector<uint8_t>& prf() const
         {
         return m_prf;
         }

      secure_vector<uint8_t>& prf()
         {
         return m_prf;
         }

      virtual void set_public_seed(
         const secure_vector<uint8_t>& public_seed) override
         {
         m_public_seed = public_seed;
         m_wots_priv_key.set_public_seed(public_seed);
         }

      virtual void set_public_seed(secure_vector<uint8_t>&& public_seed) override
         {
         m_public_seed = std::move(public_seed);
         m_wots_priv_key.set_public_seed(m_public_seed);
         }

      virtual const secure_vector<uint8_t>& public_seed() const override
         {
         return m_public_seed;
         }

      virtual std::unique_ptr<PK_Ops::Signature>
         create_signature_op(RandomNumberGenerator&,
                             const std::string&,
                             const std::string& provider) const override;

      virtual secure_vector<uint8_t> private_key_bits() const override
         {
         return raw_private_key();
         }

      virtual size_t size() const override
         {
         return XMSS_PublicKey::size() +
                sizeof(uint64_t) +
                2 * XMSS_PublicKey::m_xmss_params.element_size();
         }

      /**
       * Generates a non standartized byte sequence representing the XMSS
       * private key.
       *
       * @return byte sequence consisting of the following elements in order:
       *         4-byte OID, n-byte root node, n-byte public seed,
       *         8-byte unused leaf index, n-byte prf seed, n-byte private seed.
       **/
      virtual secure_vector<uint8_t> raw_private_key() const;
      /**
       * Algorithm 9: "treeHash"
       * Computes the internal n-byte nodes of a Merkle tree.
       *
       * @param start_idx The start index.
       * @param target_node_height Height of the target node.
       * @param adrs Address of the tree containing the target node.
       *
       * @return The root node of a tree of height target_node height with the
       *         leftmost leaf being the hash of the WOTS+ pk with index
       *         start_idx.
       **/
      secure_vector<uint8_t> tree_hash(
         size_t start_idx,
         size_t target_node_height,
         XMSS_Address& adrs);

   private:
      /**
       * Fetches shared unused leaf index fromt the index registry
       **/
      std::shared_ptr<Atomic<size_t>> recover_global_leaf_index() const;

      XMSS_WOTS_PrivateKey m_wots_priv_key;
      secure_vector<uint8_t> m_prf;
      XMSS_Index_Registry& m_index_reg;
   };

}



namespace Botan {

/**
 * A pair of XMSS public and private key.
 **/
class BOTAN_DLL XMSS_Key_Pair
   {
   public:
      XMSS_Key_Pair(XMSS_Parameters::xmss_algorithm_t xmss_oid,
                    RandomNumberGenerator& rng)
         : m_priv_key(xmss_oid, rng), m_pub_key(m_priv_key) {}

      XMSS_Key_Pair(const XMSS_PublicKey& pub_key,
                    const XMSS_PrivateKey& priv_key)
         : m_priv_key(priv_key), m_pub_key(pub_key)
         {}

      XMSS_Key_Pair(XMSS_PublicKey&& pub_key,
                    XMSS_PrivateKey&& priv_key)
         : m_priv_key(std::move(priv_key)), m_pub_key(std::move(pub_key)) {}

      const XMSS_PublicKey& public_key() const { return m_pub_key; }
      XMSS_PublicKey& public_key() { return m_pub_key; }

      const XMSS_PrivateKey& private_key() const { return m_priv_key; }
      XMSS_PrivateKey& private_key() { return m_priv_key; }

   private:
      XMSS_PrivateKey m_priv_key;
      XMSS_PublicKey m_pub_key;
   };

}


namespace Botan {

/**
* XTEA
*/
class BOTAN_DLL XTEA : public Block_Cipher_Fixed_Params<8, 16>
   {
   public:
      void encrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;
      void decrypt_n(const uint8_t in[], uint8_t out[], size_t blocks) const override;

      void clear() override;
      std::string name() const override { return "XTEA"; }
      BlockCipher* clone() const override { return new XTEA; }
   protected:
      /**
      * @return const reference to the key schedule
      */
      const secure_vector<uint32_t>& get_EK() const { return m_EK; }

   private:
      void key_schedule(const uint8_t[], size_t) override;
      secure_vector<uint32_t> m_EK;
   };

}


namespace Botan {

/**
* IEEE P1619 XTS Mode
*/
class BOTAN_DLL XTS_Mode : public Cipher_Mode
   {
   public:
      std::string name() const override;

      size_t update_granularity() const override;

      size_t minimum_final_size() const override;

      Key_Length_Specification key_spec() const override;

      size_t default_nonce_length() const override;

      bool valid_nonce_length(size_t n) const override;

      void clear() override;

      void reset() override;

   protected:
      explicit XTS_Mode(BlockCipher* cipher);

      const uint8_t* tweak() const { return m_tweak.data(); }

      const BlockCipher& cipher() const { return *m_cipher; }

      void update_tweak(size_t last_used);

   private:
      void start_msg(const uint8_t nonce[], size_t nonce_len) override;
      void key_schedule(const uint8_t key[], size_t length) override;

      std::unique_ptr<BlockCipher> m_cipher, m_tweak_cipher;
      secure_vector<uint8_t> m_tweak;
   };

/**
* IEEE P1619 XTS Encryption
*/
class BOTAN_DLL XTS_Encryption final : public XTS_Mode
   {
   public:
      /**
      * @param cipher underlying block cipher
      */
      explicit XTS_Encryption(BlockCipher* cipher) : XTS_Mode(cipher) {}

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;
   };

/**
* IEEE P1619 XTS Decryption
*/
class BOTAN_DLL XTS_Decryption final : public XTS_Mode
   {
   public:
      /**
      * @param cipher underlying block cipher
      */
      explicit XTS_Decryption(BlockCipher* cipher) : XTS_Mode(cipher) {}

      size_t process(uint8_t buf[], size_t size) override;

      void finish(secure_vector<uint8_t>& final_block, size_t offset = 0) override;

      size_t output_length(size_t input_length) const override;
   };

}


#endif
