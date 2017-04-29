/*
* Runtime CPU detection
* (C) 2009-2010,2013 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CPUID_H__
#define BOTAN_CPUID_H__

#include <botan/types.h>
#include <iosfwd>

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

#endif
